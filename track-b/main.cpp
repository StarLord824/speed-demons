#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <thread>
#include <cstring>
#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#if defined(__AVX2__)
#include <immintrin.h>
inline void bitset_and(uint64_t* dst, const uint64_t* src, size_t n) {
    size_t i = 0;
    for (; i + 4 <= n; i += 4)
        _mm256_storeu_si256((__m256i*)(dst + i), _mm256_and_si256(_mm256_loadu_si256((const __m256i*)(dst + i)), _mm256_loadu_si256((const __m256i*)(src + i))));
    for (; i < n; ++i) dst[i] &= src[i];
}
inline void bitset_or(uint64_t* dst, const uint64_t* src, size_t n) {
    size_t i = 0;
    for (; i + 4 <= n; i += 4)
        _mm256_storeu_si256((__m256i*)(dst + i), _mm256_or_si256(_mm256_loadu_si256((const __m256i*)(dst + i)), _mm256_loadu_si256((const __m256i*)(src + i))));
    for (; i < n; ++i) dst[i] |= src[i];
}
inline void bitset_andnot(uint64_t* dst, const uint64_t* src, size_t n) {
    size_t i = 0;
    for (; i + 4 <= n; i += 4)
        _mm256_storeu_si256((__m256i*)(dst + i), _mm256_andnot_si256(_mm256_loadu_si256((const __m256i*)(src + i)), _mm256_loadu_si256((const __m256i*)(dst + i))));
    for (; i < n; ++i) dst[i] &= ~src[i];
}
#else
inline void bitset_and(uint64_t* dst, const uint64_t* src, size_t n) { for (size_t i = 0; i < n; ++i) dst[i] &= src[i]; }
inline void bitset_or(uint64_t* dst, const uint64_t* src, size_t n) { for (size_t i = 0; i < n; ++i) dst[i] |= src[i]; }
inline void bitset_andnot(uint64_t* dst, const uint64_t* src, size_t n) { for (size_t i = 0; i < n; ++i) dst[i] &= ~src[i]; }
#endif

// Fast lookup tables
static uint32_t WORD_BIT_SUM[65536];
static uint8_t WORD_POPCNT[65536];
static uint8_t CHAR_MAP[256];

struct InitTables {
    InitTables() {
        for (uint32_t i = 0; i < 256; ++i) {
            if ((i >= 'a' && i <= 'z') || (i >= '0' && i <= '9')) CHAR_MAP[i] = (uint8_t)i;
            else if (i >= 'A' && i <= 'Z') CHAR_MAP[i] = (uint8_t)(i + 32);
            else CHAR_MAP[i] = 0;
        }
        for (uint32_t i = 0; i < 65536; ++i) {
            uint32_t s = 0, cnt = 0;
            for (uint32_t b = 0; b < 16; ++b) {
                if ((i >> b) & 1) { s += b; cnt++; }
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

inline char* u64_to_str(char* buf, uint64_t val) {
    if (val == 0) { *buf++ = '0'; return buf; }
    char tmp[24]; int idx = 0;
    while (val > 0) { tmp[idx++] = (char)('0' + (val % 10)); val /= 10; }
    while (idx > 0) { *buf++ = tmp[--idx]; }
    return buf;
}

inline uint64_t fnv1a_64(const char* s, size_t len) {
    uint64_t h = 14695981039346656037ULL;
    for (size_t i = 0; i < len; ++i) {
        h ^= (uint8_t)s[i];
        h *= 1099511628211ULL;
    }
    return h;
}

struct TermTable {
    struct Slot {
        uint64_t hash;
        uint32_t str_offset;
        uint16_t str_len;
        uint32_t term_id;
    };
    std::vector<Slot> slots;
    uint32_t mask;
    std::string string_pool;
    uint32_t count = 0;

    void init(size_t cap) {
        size_t actual_cap = 1;
        while (actual_cap < cap * 2) actual_cap <<= 1;
        slots.resize(actual_cap, {0, 0, 0, UINT32_MAX});
        mask = actual_cap - 1;
        string_pool.reserve(cap * 12);
    }

    uint32_t get_or_insert(const char* s, size_t len) {
        uint64_t h = fnv1a_64(s, len);
        uint32_t idx = h & mask;
        while (true) {
            if (slots[idx].term_id == UINT32_MAX) {
                uint32_t tid = count++;
                slots[idx].hash = h;
                slots[idx].str_offset = (uint32_t)string_pool.size();
                slots[idx].str_len = (uint16_t)len;
                slots[idx].term_id = tid;
                string_pool.append(s, len);
                return tid;
            }
            if (slots[idx].hash == h && slots[idx].str_len == len &&
                std::memcmp(string_pool.data() + slots[idx].str_offset, s, len) == 0) {
                return slots[idx].term_id;
            }
            idx = (idx + 1) & mask;
        }
    }

    uint32_t lookup(uint64_t h) const {
        uint32_t idx = h & mask;
        while (true) {
            if (slots[idx].term_id == UINT32_MAX) return UINT32_MAX;
            if (slots[idx].hash == h) return slots[idx].term_id;
            idx = (idx + 1) & mask;
        }
    }
};

enum class BoolOp : uint8_t { AND, OR, NOT };

struct BoolQuery {
    std::vector<uint32_t> term_ids;
    std::vector<BoolOp> ops;
};

struct PhraseQuery {
    std::vector<uint32_t> term_ids;
};

enum class QueryType : uint8_t { BOOL, PHRASE };

struct Query {
    QueryType type;
    BoolQuery bq;
    PhraseQuery pq;
};

static uint32_t doc_text_off[250000];
static uint16_t doc_text_len[250000];

const char* find_doc_start(const char* data, size_t size, uint32_t target_doc) {
    if (target_doc == 0) return data;
    size_t low = 0, high = size;
    while (low < high) {
        size_t mid = low + (high - low) / 2;
        const char* p = data + mid;
        while (p < data + size && *p != '\n') p++;
        if (p < data + size) p++;
        if (p >= data + size) { high = mid; continue; }
        uint32_t did = 0;
        const char* q = p;
        while (q < data + size && *q >= '0' && *q <= '9') did = did * 10 + (*q++ - '0');
        if (did < target_doc) low = mid + 1;
        else high = mid;
    }
    const char* p = data + low;
    if (p < data + size) {
        while (p < data + size && *p != '\n') p++;
        if (p < data + size) p++;
    }
    while (p < data + size) {
        const char* q = p;
        uint32_t did = 0;
        while (q < data + size && *q >= '0' && *q <= '9') did = did * 10 + (*q++ - '0');
        if (did == target_doc) return p;
        while (p < data + size && *p != '\n') p++;
        if (p < data + size) p++;
    }
    return data + size;
}

int main(int argc, char** argv) {
    std::string data_dir = "/data";
    std::string out_dir = "/out";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--data" && i + 1 < argc) data_dir = argv[++i];
        else if (arg == "--out" && i + 1 < argc) out_dir = argv[++i];
    }

    TermTable terms;
    terms.init(100000);

    std::vector<Query> queries;
    queries.reserve(32000);

    // Read queries
    std::string qpath = data_dir + "/queries.txt";
    int qfd = open(qpath.c_str(), O_RDONLY);
    struct stat qst;
    fstat(qfd, &qst);
    size_t qsize = qst.st_size;
    const char* qdata = (const char*)mmap(nullptr, qsize, PROT_READ, MAP_SHARED, qfd, 0);
    close(qfd);

    const char* qptr = qdata;
    const char* qend = qdata + qsize;

    while (qptr < qend) {
        const char* line_start = qptr;
        while (qptr < qend && *qptr != '\n') qptr++;
        const char* line_end = qptr;
        if (qptr < qend && *qptr == '\n') qptr++;

        if (line_end <= line_start) continue;

        if (line_start[0] == 'B' && line_start[1] == 'O' && line_start[2] == 'O' && line_start[3] == 'L' && line_start[4] == ' ') {
            Query q;
            q.type = QueryType::BOOL;
            const char* cur = line_start + 5;
            
            while (cur < line_end) {
                while (cur < line_end && *cur == ' ') cur++;
                if (cur >= line_end) break;

                if (cur + 3 <= line_end && std::memcmp(cur, "AND", 3) == 0 && (cur + 3 == line_end || cur[3] == ' ')) {
                    q.bq.ops.push_back(BoolOp::AND);
                    cur += 3;
                    continue;
                }
                if (cur + 2 <= line_end && std::memcmp(cur, "OR", 2) == 0 && (cur + 2 == line_end || cur[2] == ' ')) {
                    q.bq.ops.push_back(BoolOp::OR);
                    cur += 2;
                    continue;
                }
                if (cur + 3 <= line_end && std::memcmp(cur, "NOT", 3) == 0 && (cur + 3 == line_end || cur[3] == ' ')) {
                    q.bq.ops.push_back(BoolOp::NOT);
                    cur += 3;
                    continue;
                }

                const char* w_start = cur;
                while (cur < line_end && *cur != ' ') cur++;
                char lower_buf[128];
                size_t w_len = cur - w_start;
                for (size_t j = 0; j < w_len; ++j) {
                    char c = w_start[j];
                    if (c >= 'A' && c <= 'Z') c += 32;
                    lower_buf[j] = c;
                }
                uint32_t tid = terms.get_or_insert(lower_buf, w_len);
                q.bq.term_ids.push_back(tid);
            }
            queries.push_back(std::move(q));
        } else if (line_start[0] == 'P' && line_start[1] == 'H' && line_start[2] == 'R' && line_start[3] == 'A' && line_start[4] == 'S' && line_start[5] == 'E' && line_start[6] == ' ') {
            Query q;
            q.type = QueryType::PHRASE;
            const char* cur = line_start + 7;
            while (cur < line_end && (*cur == ' ' || *cur == '\"')) cur++;
            const char* phrase_end = line_end;
            while (phrase_end > cur && (phrase_end[-1] == ' ' || phrase_end[-1] == '\"' || phrase_end[-1] == '\r')) phrase_end--;

            while (cur < phrase_end) {
                while (cur < phrase_end && *cur == ' ') cur++;
                if (cur >= phrase_end) break;
                const char* w_start = cur;
                while (cur < phrase_end && *cur != ' ') cur++;
                char lower_buf[128];
                size_t w_len = cur - w_start;
                for (size_t j = 0; j < w_len; ++j) {
                    char c = w_start[j];
                    if (c >= 'A' && c <= 'Z') c += 32;
                    lower_buf[j] = c;
                }
                uint32_t tid = terms.get_or_insert(lower_buf, w_len);
                q.pq.term_ids.push_back(tid);
            }
            queries.push_back(std::move(q));
        }
    }
    munmap((void*)qdata, qsize);

    uint32_t num_terms = terms.count;

    // Build term string view table for fast phrase verification
    std::vector<uint32_t> term_str_off(num_terms);
    std::vector<uint16_t> term_str_len(num_terms);
    for (const auto& sl : terms.slots) {
        if (sl.term_id != UINT32_MAX) {
            term_str_off[sl.term_id] = sl.str_offset;
            term_str_len[sl.term_id] = sl.str_len;
        }
    }
    const char* term_pool_data = terms.string_pool.data();
    auto get_term_view = [&](uint32_t tid) -> std::string_view {
        return std::string_view(term_pool_data + term_str_off[tid], term_str_len[tid]);
    };

    std::vector<uint32_t> query_term_freq(num_terms, 0);
    for (const auto& q : queries) {
        if (q.type == QueryType::BOOL) {
            for (uint32_t tid : q.bq.term_ids) query_term_freq[tid]++;
        } else {
            for (uint32_t tid : q.pq.term_ids) query_term_freq[tid]++;
        }
    }

    uint32_t num_docs = 250000;
    uint32_t num_words = (num_docs + 63) / 64;

    // Identify hot terms (frequency in queries >= 15)
    std::vector<uint32_t> hot_id(num_terms, UINT32_MAX);
    uint32_t num_hot_terms = 0;
    for (uint32_t tid = 0; tid < num_terms; ++tid) {
        if (query_term_freq[tid] >= 15) {
            hot_id[tid] = num_hot_terms++;
        }
    }

    // Allocate contiguous bitset storage for hot terms (718 terms * 31.25 KB = 22.4 MB)
    std::vector<uint64_t> hot_bitsets((size_t)num_hot_terms * num_words, 0);

    // Process docs
    std::string dpath = data_dir + "/docs.txt";
    int dfd = open(dpath.c_str(), O_RDONLY);
    struct stat dst;
    fstat(dfd, &dst);
    size_t dsize = dst.st_size;
    const char* ddata = (const char*)mmap(nullptr, dsize, PROT_READ, MAP_SHARED, dfd, 0);
    close(dfd);

    unsigned int num_threads = 12;
    if (num_threads > std::thread::hardware_concurrency() && std::thread::hardware_concurrency() > 0) {
        num_threads = std::thread::hardware_concurrency();
    }

    // Find thread document and byte boundaries (aligned to multiples of 64 docs)
    uint32_t words_per_thread = (num_words + num_threads - 1) / num_threads;
    std::vector<uint32_t> thread_d_start(num_threads + 1);
    std::vector<const char*> thread_chunk_starts(num_threads + 1);
    for (uint32_t t = 0; t <= num_threads; ++t) {
        uint32_t w = std::min(num_words, t * words_per_thread);
        uint32_t did = std::min(num_docs, w * 64);
        thread_d_start[t] = did;
        thread_chunk_starts[t] = find_doc_start(ddata, dsize, did);
    }

    std::vector<std::thread> workers;

    // Single-pass Scan & Tokenize directly into thread buffers + direct hot bitsets
    std::vector<std::vector<uint32_t>> thread_tids(num_threads);
    std::vector<std::vector<uint32_t>> thread_doc_start(num_threads);
    std::vector<std::vector<uint32_t>> thread_counts(num_threads, std::vector<uint32_t>(num_terms, 0));

    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            uint32_t d_start = thread_d_start[t];
            uint32_t d_end = thread_d_start[t + 1];
            uint32_t num_d = d_end - d_start;
            auto& tids = thread_tids[t];
            auto& doc_start = thread_doc_start[t];
            auto& counts = thread_counts[t];
            tids.reserve(num_d * 55);
            doc_start.resize(num_d + 1);

            std::vector<uint32_t> last_seen_doc(num_terms, UINT32_MAX);

            const char* ptr = thread_chunk_starts[t];
            const char* end = thread_chunk_starts[t + 1];

            for (uint32_t doc_id = d_start; doc_id < d_end; ++doc_id) {
                doc_start[doc_id - d_start] = (uint32_t)tids.size();
                while (ptr < end && *ptr != '\t') ptr++;
                if (ptr < end) ptr++; // skip '\t'
                const char* text_start = ptr;
                while (ptr < end && *ptr != '\n') ptr++;
                uint32_t text_len = (uint32_t)(ptr - text_start);
                if (ptr < end) ptr++; // skip '\n'

                doc_text_off[doc_id] = (uint32_t)(text_start - ddata);
                doc_text_len[doc_id] = (uint16_t)text_len;

                const char* cur = text_start;
                const char* d_end_text = text_start + text_len;

                while (cur < d_end_text) {
                    while (cur < d_end_text && CHAR_MAP[(uint8_t)*cur] == 0) cur++;
                    if (cur >= d_end_text) break;

                    uint64_t h = 14695981039346656037ULL;
                    while (cur < d_end_text) {
                        uint8_t lc = CHAR_MAP[(uint8_t)*cur];
                        if (lc == 0) break;
                        h = (h ^ lc) * 1099511628211ULL;
                        cur++;
                    }

                    uint32_t tid = terms.lookup(h);
                    if (tid != UINT32_MAX) {
                        if (last_seen_doc[tid] != doc_id) {
                            last_seen_doc[tid] = doc_id;
                            uint32_t hid = hot_id[tid];
                            if (hid != UINT32_MAX) {
                                hot_bitsets[(size_t)hid * num_words + (doc_id / 64)] |= (1ULL << (doc_id % 64));
                            } else {
                                tids.push_back(tid);
                                counts[tid]++;
                            }
                        }
                    }
                }
            }
            doc_start[num_d] = (uint32_t)tids.size();
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // Compute prefix sums for CSR flat non-hot postings array
    std::vector<uint32_t> postings_head(num_terms + 1, 0);
    uint32_t total_postings = 0;
    for (uint32_t tid = 0; tid < num_terms; ++tid) {
        postings_head[tid] = total_postings;
        if (hot_id[tid] == UINT32_MAX) {
            for (unsigned int t = 0; t < num_threads; ++t) {
                total_postings += thread_counts[t][tid];
            }
        }
    }
    postings_head[num_terms] = total_postings;

    // Calculate thread write offsets
    std::vector<std::vector<uint32_t>> thread_write_offsets(num_threads, std::vector<uint32_t>(num_terms, 0));
    for (uint32_t tid = 0; tid < num_terms; ++tid) {
        if (hot_id[tid] == UINT32_MAX) {
            uint32_t cur = postings_head[tid];
            for (unsigned int t = 0; t < num_threads; ++t) {
                thread_write_offsets[t][tid] = cur;
                cur += thread_counts[t][tid];
            }
        }
    }
    std::vector<std::vector<uint32_t>>().swap(thread_counts);

    // Flat CSR postings buffer for non-hot terms
    std::vector<uint32_t> postings_data(total_postings);

    // Fast in-memory scatter into postings_data
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            uint32_t d_start = thread_d_start[t];
            uint32_t d_end = thread_d_start[t + 1];
            auto& offsets = thread_write_offsets[t];
            const auto& tids = thread_tids[t];
            const auto& doc_start = thread_doc_start[t];

            for (uint32_t doc_id = d_start; doc_id < d_end; ++doc_id) {
                uint32_t s_idx = doc_start[doc_id - d_start];
                uint32_t e_idx = doc_start[doc_id - d_start + 1];
                for (uint32_t i = s_idx; i < e_idx; ++i) {
                    uint32_t tid = tids[i];
                    uint32_t pos = offsets[tid]++;
                    postings_data[pos] = doc_id;
                }
            }
            std::vector<uint32_t>().swap(thread_tids[t]);
            std::vector<uint32_t>().swap(thread_doc_start[t]);
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();
    std::vector<std::vector<uint32_t>>().swap(thread_write_offsets);

    // Parallel query evaluation
    size_t num_queries = queries.size();
    std::vector<std::string> thread_outputs(num_threads);

    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            size_t q_start = (num_queries * t) / num_threads;
            size_t q_end = (num_queries * (t + 1)) / num_threads;
            auto& out_str = thread_outputs[t];
            out_str.reserve((q_end - q_start) * 64);

            std::vector<uint64_t> bitset_cur(num_words);
            std::vector<uint64_t> bitset_tmp(num_words, 0);
            char line_buf[256];

            struct TokHistory {
                uint32_t hash;
                uint16_t len;
                const char* ptr;
            };

            auto fast_doc_phrase_match = [&](uint32_t doc_id, const uint32_t* p_hashes, const std::string_view* p_views, size_t k) -> bool {
                const char* ptr = ddata + doc_text_off[doc_id];
                const char* end = ptr + doc_text_len[doc_id];

                TokHistory hist[8];
                size_t num_toks = 0;

                while (ptr < end) {
                    while (ptr < end && CHAR_MAP[(uint8_t)*ptr] == 0) ptr++;
                    if (ptr >= end) break;

                    const char* tok_start = ptr;
                    uint32_t h = 5381;
                    size_t tok_len = 0;
                    while (ptr < end) {
                        uint8_t lc = CHAR_MAP[(uint8_t)*ptr];
                        if (lc == 0) break;
                        h = ((h << 5) + h) + lc;
                        tok_len++;
                        ptr++;
                    }

                    hist[num_toks & 7] = {h, (uint16_t)tok_len, tok_start};
                    num_toks++;

                    if (num_toks >= k) {
                        bool match = true;
                        for (size_t j = 0; j < k; ++j) {
                            size_t h_idx = (num_toks - k + j) & 7;
                            if (hist[h_idx].hash != p_hashes[j] || hist[h_idx].len != p_views[j].size()) {
                                match = false;
                                break;
                            }
                        }
                        if (match) {
                            bool full_match = true;
                            for (size_t j = 0; j < k; ++j) {
                                size_t h_idx = (num_toks - k + j) & 7;
                                const char* t_p = hist[h_idx].ptr;
                                const char* v_p = p_views[j].data();
                                size_t v_len = p_views[j].size();
                                for (size_t b = 0; b < v_len; ++b) {
                                    uint8_t tc = (uint8_t)t_p[b];
                                    if (tc >= 'A' && tc <= 'Z') tc += 32;
                                    if (tc != (uint8_t)v_p[b]) {
                                        full_match = false;
                                        break;
                                    }
                                }
                                if (!full_match) break;
                            }
                            if (full_match) return true;
                        }
                    }
                }
                return false;
            };

            for (size_t qi = q_start; qi < q_end; ++qi) {
                const auto& q = queries[qi];
                if (q.type == QueryType::BOOL) {
                    const auto& bq = q.bq;
                    uint32_t t0 = bq.term_ids[0];
                    uint32_t hid0 = hot_id[t0];
                    if (hid0 != UINT32_MAX) {
                        std::memcpy(bitset_cur.data(), hot_bitsets.data() + (size_t)hid0 * num_words, num_words * sizeof(uint64_t));
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
                        uint32_t hid = hot_id[term_id];
                        const uint64_t* term_bits = nullptr;
                        uint32_t pstart = 0, pend = 0;

                        if (hid != UINT32_MAX) {
                            term_bits = hot_bitsets.data() + (size_t)hid * num_words;
                        } else {
                            pstart = postings_head[term_id];
                            pend = postings_head[term_id + 1];
                            for (uint32_t i = pstart; i < pend; ++i) {
                                uint32_t d = postings_data[i];
                                bitset_tmp[d / 64] |= (1ULL << (d % 64));
                            }
                            term_bits = bitset_tmp.data();
                        }

                        if (op == BoolOp::AND) {
                            bitset_and(bitset_cur.data(), term_bits, num_words);
                        } else if (op == BoolOp::OR) {
                            bitset_or(bitset_cur.data(), term_bits, num_words);
                        } else if (op == BoolOp::NOT) {
                            bitset_andnot(bitset_cur.data(), term_bits, num_words);
                        }

                        if (hid == UINT32_MAX) {
                            // Sparse reset of bitset_tmp
                            for (uint32_t i = pstart; i < pend; ++i) {
                                bitset_tmp[postings_data[i] / 64] = 0;
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
                        out_str.append("0 0\n");
                    } else {
                        char* p = line_buf;
                        p = u64_to_str(p, count);
                        *p++ = ' ';
                        p = u64_to_str(p, sum);
                        for (uint32_t i = 0; i < top_cnt; ++i) {
                            *p++ = ' ';
                            p = u64_to_str(p, top10[i]);
                        }
                        *p++ = '\n';
                        out_str.append(line_buf, p - line_buf);
                    }
                } else {
                    // PHRASE query
                    const auto& pq = q.pq;
                    bool all_hot = true;
                    for (uint32_t tid : pq.term_ids) {
                        if (hot_id[tid] == UINT32_MAX) {
                            all_hot = false;
                            break;
                        }
                    }

                    std::vector<uint32_t> candidates;

                    if (all_hot) {
                        uint32_t hid0 = hot_id[pq.term_ids[0]];
                        std::memcpy(bitset_cur.data(), hot_bitsets.data() + (size_t)hid0 * num_words, num_words * sizeof(uint64_t));
                        for (size_t i = 1; i < pq.term_ids.size(); ++i) {
                            uint32_t hidi = hot_id[pq.term_ids[i]];
                            bitset_and(bitset_cur.data(), hot_bitsets.data() + (size_t)hidi * num_words, num_words);
                        }
                        for (uint32_t w = 0; w < num_words; ++w) {
                            uint64_t word = bitset_cur[w];
                            while (word != 0) {
                                uint32_t bit = __builtin_ctzll(word);
                                candidates.push_back(w * 64 + bit);
                                word &= (word - 1);
                            }
                        }
                    } else {
                        size_t min_idx = SIZE_MAX;
                        uint32_t min_size = UINT32_MAX;
                        for (size_t i = 0; i < pq.term_ids.size(); ++i) {
                            uint32_t tid = pq.term_ids[i];
                            if (hot_id[tid] == UINT32_MAX) {
                                uint32_t sz = postings_head[tid + 1] - postings_head[tid];
                                if (sz < min_size) {
                                    min_size = sz;
                                    min_idx = i;
                                }
                            }
                        }

                        uint32_t seed_start = postings_head[pq.term_ids[min_idx]];
                        uint32_t seed_end = postings_head[pq.term_ids[min_idx] + 1];
                        candidates.reserve(seed_end - seed_start);

                        for (uint32_t idx = seed_start; idx < seed_end; ++idx) {
                            uint32_t d = postings_data[idx];
                            bool has_all = true;
                            for (size_t i = 0; i < pq.term_ids.size(); ++i) {
                                if (i == min_idx) continue;
                                uint32_t tid = pq.term_ids[i];
                                uint32_t hid = hot_id[tid];
                                if (hid != UINT32_MAX) {
                                    const uint64_t* bptr = hot_bitsets.data() + (size_t)hid * num_words;
                                    if (!((bptr[d / 64] >> (d % 64)) & 1)) {
                                        has_all = false;
                                        break;
                                    }
                                } else {
                                    uint32_t pstart = postings_head[tid];
                                    uint32_t pend = postings_head[tid + 1];
                                    if (!std::binary_search(postings_data.data() + pstart, postings_data.data() + pend, d)) {
                                        has_all = false;
                                        break;
                                    }
                                }
                            }
                            if (has_all) candidates.push_back(d);
                        }
                    }

                    size_t k = pq.term_ids.size();
                    uint32_t p_hashes[8];
                    std::string_view p_views[8];
                    for (size_t j = 0; j < k; ++j) {
                        p_views[j] = get_term_view(pq.term_ids[j]);
                        uint32_t h = 5381;
                        for (char c : p_views[j]) h = ((h << 5) + h) + (uint8_t)c;
                        p_hashes[j] = h;
                    }

                    std::vector<uint32_t> matches;
                    for (uint32_t d : candidates) {
                        if (fast_doc_phrase_match(d, p_hashes, p_views, k)) {
                            matches.push_back(d);
                        }
                    }

                    if (matches.empty()) {
                        out_str.append("0 0\n");
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
                        *p++ = '\n';
                        out_str.append(line_buf, p - line_buf);
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
        for (unsigned int t = 0; t < num_threads; ++t) {
            write(out_fd, thread_outputs[t].data(), thread_outputs[t].size());
        }
        close(out_fd);
    }

    return 0;
}
