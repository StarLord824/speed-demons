#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <thread>
#include <cstring>
#include <cstdint>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

// Fast itoa
inline char* u64_to_str(char* buf, uint64_t val) {
    if (val == 0) { *buf++ = '0'; return buf; }
    char tmp[24]; int idx = 0;
    while (val > 0) { tmp[idx++] = (char)('0' + (val % 10)); val /= 10; }
    while (idx > 0) { *buf++ = tmp[--idx]; }
    return buf;
}

// 64-bit FNV-1a hash
inline uint64_t fnv1a_64(const char* s, size_t len) {
    uint64_t h = 14695981039346656037ULL;
    for (size_t i = 0; i < len; ++i) {
        h ^= (uint8_t)s[i];
        h *= 1099511628211ULL;
    }
    return h;
}

// Fast lookup table for sum of set bit positions in a uint64_t
static uint32_t WORD_BIT_SUM[65536];
static uint8_t WORD_POPCNT[65536];

struct InitTables {
    InitTables() {
        for (uint32_t i = 0; i < 65536; ++i) {
            uint32_t s = 0;
            uint32_t cnt = 0;
            for (uint32_t b = 0; b < 16; ++b) {
                if ((i >> b) & 1) {
                    s += b;
                    cnt++;
                }
            }
            WORD_BIT_SUM[i] = s;
            WORD_POPCNT[i] = cnt;
        }
    }
} init_tables;

inline uint64_t u64_bits_sum(uint64_t w) {
    uint32_t p0 = w & 0xFFFF;
    uint32_t p1 = (w >> 16) & 0xFFFF;
    uint32_t p2 = (w >> 32) & 0xFFFF;
    uint32_t p3 = (w >> 48);
    return WORD_BIT_SUM[p0] +
           WORD_BIT_SUM[p1] + (uint64_t)WORD_POPCNT[p1] * 16 +
           WORD_BIT_SUM[p2] + (uint64_t)WORD_POPCNT[p2] * 32 +
           WORD_BIT_SUM[p3] + (uint64_t)WORD_POPCNT[p3] * 48;
}

// Term table mapping string -> term_id
struct TermTable {
    struct Slot {
        uint64_t hash;
        uint32_t str_offset;
        uint16_t str_len;
        uint32_t term_id;
    };
    std::vector<Slot> slots;
    std::vector<char> str_pool;
    uint32_t mask = 0;
    uint32_t num_terms = 0;

    void init(uint32_t cap_pow2) {
        slots.assign(cap_pow2, {0, 0, 0, 0});
        mask = cap_pow2 - 1;
        str_pool.reserve(1024 * 1024);
        num_terms = 0;
    }

    uint32_t get_or_insert(const char* s, size_t len) {
        uint64_t h = fnv1a_64(s, len);
        if (h == 0) h = 1;
        uint32_t idx = (uint32_t)h & mask;
        while (slots[idx].hash != 0) {
            if (slots[idx].hash == h && slots[idx].str_len == len) {
                if (std::memcmp(str_pool.data() + slots[idx].str_offset, s, len) == 0) {
                    return slots[idx].term_id;
                }
            }
            idx = (idx + 1) & mask;
        }

        uint32_t tid = num_terms++;
        uint32_t off = (uint32_t)str_pool.size();
        str_pool.insert(str_pool.end(), s, s + len);
        slots[idx] = {h, off, (uint16_t)len, tid};

        if (num_terms * 2 > mask) {
            uint32_t new_cap = (mask + 1) * 2;
            std::vector<Slot> old = std::move(slots);
            slots.assign(new_cap, {0, 0, 0, 0});
            mask = new_cap - 1;
            for (const auto& sl : old) {
                if (sl.hash != 0) {
                    uint32_t nidx = (uint32_t)sl.hash & mask;
                    while (slots[nidx].hash != 0) {
                        nidx = (nidx + 1) & mask;
                    }
                    slots[nidx] = sl;
                }
            }
        }
        return tid;
    }

    inline uint32_t lookup(const char* s, size_t len) const {
        uint64_t h = fnv1a_64(s, len);
        if (h == 0) h = 1;
        uint32_t idx = (uint32_t)h & mask;
        while (slots[idx].hash != 0) {
            if (slots[idx].hash == h && slots[idx].str_len == len) {
                if (std::memcmp(str_pool.data() + slots[idx].str_offset, s, len) == 0) {
                    return slots[idx].term_id;
                }
            }
            idx = (idx + 1) & mask;
        }
        return UINT32_MAX;
    }
};

enum class QueryType : uint8_t {
    BOOL,
    PHRASE
};

enum class BoolOp : uint8_t {
    AND,
    OR,
    NOT
};

struct BoolQuery {
    std::vector<uint32_t> term_ids;
    std::vector<BoolOp> ops;
};

struct PhraseQuery {
    std::vector<uint32_t> term_ids;
};

struct ParsedQuery {
    QueryType type;
    BoolQuery bq;
    PhraseQuery pq;
};

struct DocSlice {
    const char* text;
    uint32_t len;
};

int main(int argc, char** argv) {
    std::string data_dir = "/data";
    std::string out_dir = "/out";
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--data" && i + 1 < argc) data_dir = argv[++i];
        else if (std::string(argv[i]) == "--out" && i + 1 < argc) out_dir = argv[++i];
    }

    // 1. Read and parse queries.txt first
    std::string qpath = data_dir + "/queries.txt";
    int qfd = open(qpath.c_str(), O_RDONLY);
    if (qfd < 0) return 1;
    struct stat qst;
    fstat(qfd, &qst);
    size_t qsize = qst.st_size;
    const char* qdata = (const char*)mmap(nullptr, qsize, PROT_READ, MAP_SHARED, qfd, 0);
    close(qfd);

    TermTable terms;
    terms.init(131072);

    std::vector<ParsedQuery> queries;
    queries.reserve(55000);

    const char* qptr = qdata;
    const char* qend = qdata + qsize;

    while (qptr < qend) {
        while (qptr < qend && (*qptr == '\r' || *qptr == '\n')) qptr++;
        if (qptr >= qend) break;

        const char* line_start = qptr;
        while (qptr < qend && *qptr != '\n') qptr++;
        const char* line_end = qptr;
        if (line_end > line_start && *(line_end - 1) == '\r') line_end--;

        if (line_end - line_start >= 5 && std::memcmp(line_start, "BOOL ", 5) == 0) {
            ParsedQuery pq;
            pq.type = QueryType::BOOL;
            const char* p = line_start + 5;
            bool expect_term = true;
            while (p < line_end) {
                while (p < line_end && *p == ' ') p++;
                if (p >= line_end) break;
                const char* w_start = p;
                while (p < line_end && *p != ' ') p++;
                size_t w_len = p - w_start;

                if (expect_term) {
                    uint32_t tid = terms.get_or_insert(w_start, w_len);
                    pq.bq.term_ids.push_back(tid);
                    expect_term = false;
                } else {
                    if (w_len == 3 && std::memcmp(w_start, "AND", 3) == 0) pq.bq.ops.push_back(BoolOp::AND);
                    else if (w_len == 2 && std::memcmp(w_start, "OR", 2) == 0) pq.bq.ops.push_back(BoolOp::OR);
                    else if (w_len == 3 && std::memcmp(w_start, "NOT", 3) == 0) pq.bq.ops.push_back(BoolOp::NOT);
                    expect_term = true;
                }
            }
            queries.push_back(std::move(pq));
        } else if (line_end - line_start >= 9 && std::memcmp(line_start, "PHRASE \"", 8) == 0) {
            ParsedQuery pq;
            pq.type = QueryType::PHRASE;
            const char* p = line_start + 8;
            const char* p_end = line_end;
            if (p_end > p && *(p_end - 1) == '"') p_end--;

            while (p < p_end) {
                while (p < p_end && *p == ' ') p++;
                if (p >= p_end) break;
                const char* w_start = p;
                while (p < p_end && *p != ' ') p++;
                size_t w_len = p - w_start;

                uint32_t tid = terms.get_or_insert(w_start, w_len);
                pq.pq.term_ids.push_back(tid);
            }
            queries.push_back(std::move(pq));
        }
    }
    munmap((void*)qdata, qsize);

    uint32_t num_terms = terms.num_terms;

    // Track query term frequencies
    std::vector<uint32_t> query_term_freq(num_terms, 0);
    for (const auto& q : queries) {
        if (q.type == QueryType::BOOL) {
            for (uint32_t tid : q.bq.term_ids) query_term_freq[tid]++;
        } else {
            for (uint32_t tid : q.pq.term_ids) query_term_freq[tid]++;
        }
    }

    // 2. Read docs.txt
    std::string dpath = data_dir + "/docs.txt";
    int dfd = open(dpath.c_str(), O_RDONLY);
    if (dfd < 0) return 1;
    struct stat dst;
    fstat(dfd, &dst);
    size_t dsize = dst.st_size;
    const char* ddata = (const char*)mmap(nullptr, dsize, PROT_READ, MAP_SHARED, dfd, 0);
    close(dfd);

    unsigned int num_threads = 12;
    if (num_threads > std::thread::hardware_concurrency() && std::thread::hardware_concurrency() > 0) {
        num_threads = std::thread::hardware_concurrency();
    }

    std::vector<DocSlice> docs(250000);
    std::vector<std::thread> workers;

    // Parallel doc slicing across 12 threads
    std::vector<const char*> chunk_starts(num_threads + 1);
    chunk_starts[0] = ddata;
    chunk_starts[num_threads] = ddata + dsize;
    for (unsigned int t = 1; t < num_threads; ++t) {
        const char* approx = ddata + (dsize / num_threads) * t;
        while (approx < ddata + dsize && *approx != '\n') approx++;
        if (approx < ddata + dsize) approx++;
        chunk_starts[t] = approx;
    }

    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            const char* ptr = chunk_starts[t];
            const char* end = chunk_starts[t + 1];
            while (ptr < end) {
                uint32_t doc_id = 0;
                while (*ptr >= '0' && *ptr <= '9') {
                    doc_id = doc_id * 10 + (*ptr++ - '0');
                }
                if (ptr >= end) break;
                ptr++; // skip \t
                const char* text_start = ptr;
                while (ptr < end && *ptr != '\n') ptr++;
                docs[doc_id] = {text_start, (uint32_t)(ptr - text_start)};
                if (ptr < end && *ptr == '\n') ptr++;
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    uint32_t num_docs = (uint32_t)docs.size();
    uint32_t num_words = (num_docs + 63) / 64;

    // PASS 1: Count postings per term per thread
    std::vector<std::vector<uint32_t>> thread_counts(num_threads, std::vector<uint32_t>(num_terms, 0));
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            uint32_t d_start = (num_docs * t) / num_threads;
            uint32_t d_end = (num_docs * (t + 1)) / num_threads;
            auto& counts = thread_counts[t];
            std::vector<uint32_t> last_seen_doc(num_terms, UINT32_MAX);
            char token_buf[64];

            for (uint32_t doc_id = d_start; doc_id < d_end; ++doc_id) {
                const char* ptr = docs[doc_id].text;
                const char* end = ptr + docs[doc_id].len;

                while (ptr < end) {
                    while (ptr < end) {
                        uint8_t c = (uint8_t)*ptr;
                        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) break;
                        ptr++;
                    }
                    if (ptr >= end) break;

                    size_t tok_len = 0;
                    while (ptr < end && tok_len < 32) {
                        uint8_t c = (uint8_t)*ptr;
                        if (c >= 'a' && c <= 'z') { token_buf[tok_len++] = c; ptr++; }
                        else if (c >= '0' && c <= '9') { token_buf[tok_len++] = c; ptr++; }
                        else if (c >= 'A' && c <= 'Z') { token_buf[tok_len++] = c + ('a' - 'A'); ptr++; }
                        else break;
                    }
                    while (ptr < end) {
                        uint8_t c = (uint8_t)*ptr;
                        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) ptr++;
                        else break;
                    }

                    uint32_t tid = terms.lookup(token_buf, tok_len);
                    if (tid != UINT32_MAX) {
                        if (last_seen_doc[tid] != doc_id) {
                            last_seen_doc[tid] = doc_id;
                            counts[tid]++;
                        }
                    }
                }
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // Compute prefix sums for CSR flat postings array
    std::vector<uint32_t> postings_head(num_terms + 1, 0);
    uint32_t total_postings = 0;
    for (uint32_t tid = 0; tid < num_terms; ++tid) {
        postings_head[tid] = total_postings;
        for (unsigned int t = 0; t < num_threads; ++t) {
            total_postings += thread_counts[t][tid];
        }
    }
    postings_head[num_terms] = total_postings;

    // Calculate thread write offsets
    std::vector<std::vector<uint32_t>> thread_write_offsets(num_threads, std::vector<uint32_t>(num_terms, 0));
    for (uint32_t tid = 0; tid < num_terms; ++tid) {
        uint32_t cur = postings_head[tid];
        for (unsigned int t = 0; t < num_threads; ++t) {
            thread_write_offsets[t][tid] = cur;
            cur += thread_counts[t][tid];
        }
    }
    std::vector<std::vector<uint32_t>>().swap(thread_counts);

    // Flat CSR postings buffer
    std::vector<uint32_t> postings_data(total_postings);

    // PASS 2: Scatter postings directly into flat CSR
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            uint32_t d_start = (num_docs * t) / num_threads;
            uint32_t d_end = (num_docs * (t + 1)) / num_threads;
            auto& offsets = thread_write_offsets[t];
            std::vector<uint32_t> last_seen_doc(num_terms, UINT32_MAX);
            char token_buf[64];

            for (uint32_t doc_id = d_start; doc_id < d_end; ++doc_id) {
                const char* ptr = docs[doc_id].text;
                const char* end = ptr + docs[doc_id].len;

                while (ptr < end) {
                    while (ptr < end) {
                        uint8_t c = (uint8_t)*ptr;
                        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) break;
                        ptr++;
                    }
                    if (ptr >= end) break;

                    size_t tok_len = 0;
                    while (ptr < end && tok_len < 32) {
                        uint8_t c = (uint8_t)*ptr;
                        if (c >= 'a' && c <= 'z') { token_buf[tok_len++] = c; ptr++; }
                        else if (c >= '0' && c <= '9') { token_buf[tok_len++] = c; ptr++; }
                        else if (c >= 'A' && c <= 'Z') { token_buf[tok_len++] = c + ('a' - 'A'); ptr++; }
                        else break;
                    }
                    while (ptr < end) {
                        uint8_t c = (uint8_t)*ptr;
                        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) ptr++;
                        else break;
                    }

                    uint32_t tid = terms.lookup(token_buf, tok_len);
                    if (tid != UINT32_MAX) {
                        if (last_seen_doc[tid] != doc_id) {
                            last_seen_doc[tid] = doc_id;
                            uint32_t pos = offsets[tid]++;
                            postings_data[pos] = doc_id;
                        }
                    }
                }
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();
    std::vector<std::vector<uint32_t>>().swap(thread_write_offsets);

    // Pre-materialize bitsets ONLY for hot query terms (freq >= 50 && df >= 1000)
    std::vector<std::vector<uint64_t>> precomputed_bitsets(num_terms);
    std::vector<const uint64_t*> bitset_ptrs(num_terms, nullptr);

    for (uint32_t tid = 0; tid < num_terms; ++tid) {
        uint32_t pstart = postings_head[tid];
        uint32_t pend = postings_head[tid + 1];
        uint32_t df = pend - pstart;
        if (query_term_freq[tid] >= 50 && df >= 1000) {
            precomputed_bitsets[tid].assign(num_words, 0);
            for (uint32_t i = pstart; i < pend; ++i) {
                uint32_t d = postings_data[i];
                precomputed_bitsets[tid][d / 64] |= (1ULL << (d % 64));
            }
            bitset_ptrs[tid] = precomputed_bitsets[tid].data();
        }
    }

    // 3. Parallel query answering
    size_t num_queries = queries.size();
    std::vector<std::string> query_results(num_queries);

    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            size_t q_start = (num_queries * t) / num_threads;
            size_t q_end = (num_queries * (t + 1)) / num_threads;

            std::vector<uint64_t> bitset_cur(num_words, 0);
            std::vector<uint64_t> bitset_tmp(num_words, 0);
            char line_buf[512];

            auto doc_tokens_match = [&](uint32_t doc_id, const std::vector<uint32_t>& phrase_tids) -> bool {
                const char* ptr = docs[doc_id].text;
                const char* end = ptr + docs[doc_id].len;
                size_t k = phrase_tids.size();
                if (k == 0) return true;

                uint32_t doc_tids[1024];
                size_t num_toks = 0;
                char token_buf[64];

                while (ptr < end && num_toks < 1024) {
                    while (ptr < end) {
                        uint8_t c = (uint8_t)*ptr;
                        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) break;
                        ptr++;
                    }
                    if (ptr >= end) break;

                    size_t tok_len = 0;
                    while (ptr < end && tok_len < 32) {
                        uint8_t c = (uint8_t)*ptr;
                        if (c >= 'a' && c <= 'z') { token_buf[tok_len++] = c; ptr++; }
                        else if (c >= '0' && c <= '9') { token_buf[tok_len++] = c; ptr++; }
                        else if (c >= 'A' && c <= 'Z') { token_buf[tok_len++] = c + ('a' - 'A'); ptr++; }
                        else break;
                    }
                    while (ptr < end) {
                        uint8_t c = (uint8_t)*ptr;
                        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) ptr++;
                        else break;
                    }

                    uint32_t tid = terms.lookup(token_buf, tok_len);
                    doc_tids[num_toks++] = tid;
                }

                if (num_toks < k) return false;
                for (size_t i = 0; i + k <= num_toks; ++i) {
                    bool match = true;
                    for (size_t j = 0; j < k; ++j) {
                        if (doc_tids[i + j] != phrase_tids[j]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) return true;
                }
                return false;
            };

            for (size_t qi = q_start; qi < q_end; ++qi) {
                const auto& q = queries[qi];
                if (q.type == QueryType::BOOL) {
                    const auto& bq = q.bq;
                    uint32_t t0 = bq.term_ids[0];
                    if (bitset_ptrs[t0] != nullptr) {
                        std::memcpy(bitset_cur.data(), bitset_ptrs[t0], num_words * sizeof(uint64_t));
                    } else {
                        std::memset(bitset_cur.data(), 0, num_words * sizeof(uint64_t));
                        uint32_t pstart = postings_head[t0];
                        uint32_t pend = postings_head[t0 + 1];
                        for (uint32_t i = pstart; i < pend; ++i) {
                            uint32_t d = postings_data[i];
                            bitset_cur[d / 64] |= (1ULL << (d % 64));
                        }
                    }

                    for (size_t op_i = 0; op_i < bq.ops.size(); ++op_i) {
                        BoolOp op = bq.ops[op_i];
                        uint32_t term_id = bq.term_ids[op_i + 1];
                        const uint64_t* term_bits = bitset_ptrs[term_id];

                        if (term_bits == nullptr) {
                            std::memset(bitset_tmp.data(), 0, num_words * sizeof(uint64_t));
                            uint32_t pstart = postings_head[term_id];
                            uint32_t pend = postings_head[term_id + 1];
                            for (uint32_t i = pstart; i < pend; ++i) {
                                uint32_t d = postings_data[i];
                                bitset_tmp[d / 64] |= (1ULL << (d % 64));
                            }
                            term_bits = bitset_tmp.data();
                        }

                        if (op == BoolOp::AND) {
                            for (uint32_t w = 0; w < num_words; ++w) {
                                bitset_cur[w] &= term_bits[w];
                            }
                        } else if (op == BoolOp::OR) {
                            for (uint32_t w = 0; w < num_words; ++w) {
                                bitset_cur[w] |= term_bits[w];
                            }
                        } else if (op == BoolOp::NOT) {
                            for (uint32_t w = 0; w < num_words; ++w) {
                                bitset_cur[w] &= ~term_bits[w];
                            }
                        }
                    }

                    // Format result with O(1) word bit sum
                    uint64_t count = 0;
                    uint64_t sum = 0;
                    uint32_t top10[10];
                    uint32_t top_cnt = 0;

                    for (uint32_t w = 0; w < num_words; ++w) {
                        uint64_t word = bitset_cur[w];
                        if (word == 0) continue;
                        uint32_t pc = __builtin_popcountll(word);
                        count += pc;
                        uint32_t base_id = w * 64;
                        sum += (uint64_t)pc * base_id + u64_bits_sum(word);
                        if (top_cnt < 10) {
                            uint64_t tw = word;
                            while (tw != 0 && top_cnt < 10) {
                                uint32_t bit = __builtin_ctzll(tw);
                                top10[top_cnt++] = base_id + bit;
                                tw &= (tw - 1);
                            }
                        }
                    }

                    if (count == 0) {
                        query_results[qi] = "0 0";
                    } else {
                        char* p = line_buf;
                        p = u64_to_str(p, count);
                        *p++ = ' ';
                        p = u64_to_str(p, sum);
                        for (uint32_t i = 0; i < top_cnt; ++i) {
                            *p++ = ' ';
                            p = u64_to_str(p, top10[i]);
                        }
                        query_results[qi].assign(line_buf, p - line_buf);
                    }
                } else {
                    // PHRASE query
                    const auto& pq = q.pq;
                    size_t min_idx = 0;
                    uint32_t min_size = postings_head[pq.term_ids[0] + 1] - postings_head[pq.term_ids[0]];
                    for (size_t i = 1; i < pq.term_ids.size(); ++i) {
                        uint32_t sz = postings_head[pq.term_ids[i] + 1] - postings_head[pq.term_ids[i]];
                        if (sz < min_size) {
                            min_size = sz;
                            min_idx = i;
                        }
                    }

                    uint32_t seed_start = postings_head[pq.term_ids[min_idx]];
                    uint32_t seed_end = postings_head[pq.term_ids[min_idx] + 1];

                    std::vector<uint32_t> candidates;
                    candidates.reserve(seed_end - seed_start);

                    for (uint32_t idx = seed_start; idx < seed_end; ++idx) {
                        uint32_t d = postings_data[idx];
                        bool has_all = true;
                        for (size_t i = 0; i < pq.term_ids.size(); ++i) {
                            if (i == min_idx) continue;
                            uint32_t pstart = postings_head[pq.term_ids[i]];
                            uint32_t pend = postings_head[pq.term_ids[i] + 1];
                            if (!std::binary_search(postings_data.data() + pstart, postings_data.data() + pend, d)) {
                                has_all = false;
                                break;
                            }
                        }
                        if (has_all) candidates.push_back(d);
                    }

                    std::vector<uint32_t> matches;
                    for (uint32_t d : candidates) {
                        if (doc_tokens_match(d, pq.term_ids)) {
                            matches.push_back(d);
                        }
                    }

                    if (matches.empty()) {
                        query_results[qi] = "0 0";
                    } else {
                        uint64_t count = matches.size();
                        uint64_t sum = 0;
                        for (uint32_t d : matches) sum += d;
                        char* p = line_buf;
                        p = u64_to_str(p, count);
                        *p++ = ' ';
                        p = u64_to_str(p, sum);
                        size_t print_cnt = std::min((size_t)10, (size_t)count);
                        for (size_t i = 0; i < print_cnt; ++i) {
                            *p++ = ' ';
                            p = u64_to_str(p, matches[i]);
                        }
                        query_results[qi].assign(line_buf, p - line_buf);
                    }
                }
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    munmap((void*)ddata, dsize);

    mkdir(out_dir.c_str(), 0777);
    std::string out_path = out_dir + "/results.txt";
    int out_fd = open(out_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (out_fd >= 0) {
        std::string buffer;
        buffer.reserve(1024 * 1024);
        for (const auto& r : query_results) {
            buffer.append(r);
            buffer.push_back('\n');
            if (buffer.size() >= 512 * 1024) {
                write(out_fd, buffer.data(), buffer.size());
                buffer.clear();
            }
        }
        if (!buffer.empty()) {
            write(out_fd, buffer.data(), buffer.size());
        }
        close(out_fd);
    }

    return 0;
}
