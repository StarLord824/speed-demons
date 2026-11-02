#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>

#if defined(__AVX2__)
#include <immintrin.h>
inline bool bitset_and(uint64_t* dst, const uint64_t* src, size_t n) {
    size_t i = 0;
    __m256i acc = _mm256_setzero_si256();
    for (; i + 4 <= n; i += 4) {
        __m256i d = _mm256_loadu_si256((const __m256i*)(dst + i));
        __m256i s = _mm256_loadu_si256((const __m256i*)(src + i));
        __m256i res = _mm256_and_si256(d, s);
        _mm256_storeu_si256((__m256i*)(dst + i), res);
        acc = _mm256_or_si256(acc, res);
    }
    uint64_t rem = 0;
    for (; i < n; ++i) {
        dst[i] &= src[i];
        rem |= dst[i];
    }
    return _mm256_testz_si256(acc, acc) && (rem == 0);
}
inline void bitset_or(uint64_t* dst, const uint64_t* src, size_t n) {
    size_t i = 0;
    for (; i + 4 <= n; i += 4)
        _mm256_storeu_si256((__m256i*)(dst + i), _mm256_or_si256(_mm256_loadu_si256((const __m256i*)(dst + i)), _mm256_loadu_si256((const __m256i*)(src + i))));
    for (; i < n; ++i) dst[i] |= src[i];
}
inline bool bitset_andnot(uint64_t* dst, const uint64_t* src, size_t n) {
    size_t i = 0;
    __m256i acc = _mm256_setzero_si256();
    for (; i + 4 <= n; i += 4) {
        __m256i d = _mm256_loadu_si256((const __m256i*)(dst + i));
        __m256i s = _mm256_loadu_si256((const __m256i*)(src + i));
        __m256i res = _mm256_andnot_si256(s, d);
        _mm256_storeu_si256((__m256i*)(dst + i), res);
        acc = _mm256_or_si256(acc, res);
    }
    uint64_t rem = 0;
    for (; i < n; ++i) {
        dst[i] &= ~src[i];
        rem |= dst[i];
    }
    return _mm256_testz_si256(acc, acc) && (rem == 0);
}
inline const char* avx2_find_char_pair(const char* ptr, const char* end, uint8_t c1, uint8_t c2) {
    __m256i v1 = _mm256_set1_epi8(c1);
    __m256i v2 = _mm256_set1_epi8(c2);
    while (ptr + 32 <= end) {
        __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
        __m256i eq = _mm256_or_si256(_mm256_cmpeq_epi8(chunk, v1), _mm256_cmpeq_epi8(chunk, v2));
        uint32_t mask = (uint32_t)_mm256_movemask_epi8(eq);
        if (mask != 0) return ptr + __builtin_ctz(mask);
        ptr += 32;
    }
    while (ptr < end && (uint8_t)*ptr != c1 && (uint8_t)*ptr != c2) ptr++;
    return ptr;
}
#else
inline bool bitset_and(uint64_t* dst, const uint64_t* src, size_t n) {
    uint64_t acc = 0;
    for (size_t i = 0; i < n; ++i) { dst[i] &= src[i]; acc |= dst[i]; }
    return acc == 0;
}
inline void bitset_or(uint64_t* dst, const uint64_t* src, size_t n) { for (size_t i = 0; i < n; ++i) dst[i] |= src[i]; }
inline bool bitset_andnot(uint64_t* dst, const uint64_t* src, size_t n) {
    uint64_t acc = 0;
    for (size_t i = 0; i < n; ++i) { dst[i] &= ~src[i]; acc |= dst[i]; }
    return acc == 0;
}
#endif

// Fast lookup tables
static uint32_t WORD_BIT_SUM[65536];
static uint8_t WORD_POPCNT[65536];
static uint8_t CHAR_MAP[256];
static uint64_t QUERY_TERM_FILTER[16384]; // 1M bits (128 KB, fits in L2 cache, 7% FPR)

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

// Compact L2-resident Term Table: 1.04 MB hashes + 0.52 MB tids
struct TermTable {
    static constexpr size_t CAP = 131072;
    static constexpr uint32_t MASK = CAP - 1;
    uint64_t hashes[CAP];
    uint32_t tids[CAP];
    uint32_t term_str_off[100000];
    uint16_t term_str_len[100000];
    std::string string_pool;
    uint32_t count = 0;

    void init() {
        std::memset(hashes, 0, sizeof(hashes));
        std::memset(tids, 0xFF, sizeof(tids));
        string_pool.reserve(1000000);
    }

    uint32_t get_or_insert(const char* s, size_t len) {
        uint64_t h = fnv1a_64(s, len);
        uint32_t idx = (uint32_t)h & MASK;
        while (true) {
            if (tids[idx] == UINT32_MAX) {
                uint32_t tid = count++;
                hashes[idx] = h;
                tids[idx] = tid;
                term_str_off[tid] = (uint32_t)string_pool.size();
                term_str_len[tid] = (uint16_t)len;
                string_pool.append(s, len);
                return tid;
            }
            if (hashes[idx] == h) return tids[idx];
            idx = (idx + 1) & MASK;
        }
    }

    inline uint32_t lookup(uint64_t h) const {
        uint32_t idx = (uint32_t)h & MASK;
        while (true) {
            if (tids[idx] == UINT32_MAX) return UINT32_MAX;
            if (hashes[idx] == h) return tids[idx];
            idx = (idx + 1) & MASK;
        }
    }
};

enum class BoolOp : uint8_t { AND, OR, NOT };

struct BoolQuery {
    std::vector<uint32_t> term_ids;
    std::vector<BoolOp> ops;
    bool has_or = false;
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

// Lightweight pthread runner with 64 KB stack per thread to eliminate excess stack RSS
struct ThreadRunner {
    template<typename F>
    static void run(unsigned int n, F&& fn) {
        std::vector<pthread_t> threads(n);
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 64 * 1024);

        struct Worker {
            F* f;
            unsigned int id;
        };
        std::vector<Worker> workers(n);
        for (unsigned int t = 0; t < n; ++t) {
            workers[t] = { &fn, t };
            pthread_create(&threads[t], &attr, [](void* arg) -> void* {
                auto* w = static_cast<Worker*>(arg);
                (*(w->f))(w->id);
                return nullptr;
            }, &workers[t]);
        }
        pthread_attr_destroy(&attr);
        for (unsigned int t = 0; t < n; ++t) {
            pthread_join(threads[t], nullptr);
        }
    }
};

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
    terms.init();

    std::vector<Query> queries;
    queries.reserve(50000);

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
                    q.bq.has_or = true;
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
                uint64_t qh = fnv1a_64(lower_buf, w_len);
                uint32_t q_bit = (uint32_t)qh & 0xFFFFF;
                QUERY_TERM_FILTER[q_bit >> 6] |= (1ULL << (q_bit & 63));
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
                uint64_t qh = fnv1a_64(lower_buf, w_len);
                uint32_t q_bit = (uint32_t)qh & 0xFFFFF;
                QUERY_TERM_FILTER[q_bit >> 6] |= (1ULL << (q_bit & 63));
                q.pq.term_ids.push_back(tid);
            }
            queries.push_back(std::move(q));
        }
    }
    munmap((void*)qdata, qsize);

    uint32_t num_terms = terms.count;
    const char* term_pool_data = terms.string_pool.data();
    auto get_term_view = [&](uint32_t tid) -> std::string_view {
        return std::string_view(term_pool_data + terms.term_str_off[tid], terms.term_str_len[tid]);
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

    // Hot terms threshold: query freq >= 35 gives 212 hot terms = 6.47 MB bitsets!
    std::vector<uint32_t> hot_id(num_terms, UINT32_MAX);
    uint32_t num_hot_terms = 0;
    for (uint32_t tid = 0; tid < num_terms; ++tid) {
        if (query_term_freq[tid] >= 35) {
            hot_id[tid] = num_hot_terms++;
        }
    }
    std::vector<uint64_t> hot_bitsets((size_t)num_hot_terms * num_words, 0);

    // 4 Blocks definition:
    static constexpr uint32_t NUM_BLOCKS = 4;
    static constexpr uint32_t WORDS_PER_BLOCK = 977;
    uint32_t block_d_start[NUM_BLOCKS];
    uint32_t block_d_end[NUM_BLOCKS];
    for (uint32_t b = 0; b < NUM_BLOCKS; ++b) {
        uint32_t w_start = b * WORDS_PER_BLOCK;
        uint32_t w_end = std::min(num_words, (b + 1) * WORDS_PER_BLOCK);
        block_d_start[b] = w_start * 64;
        block_d_end[b] = std::min(num_docs, w_end * 64);
    }

    std::string dpath = data_dir + "/docs.txt";
    int dfd = open(dpath.c_str(), O_RDONLY);
    struct stat dst;
    fstat(dfd, &dst);
    size_t dsize = dst.st_size;
    const char* ddata = (const char*)mmap(nullptr, dsize, PROT_READ, MAP_SHARED, dfd, 0);
    close(dfd);

    unsigned int num_threads = 12;

    // Per-block postings: block_postings_head[4][num_terms + 1] and block_postings_data[4] (uint16_t)
    std::vector<std::vector<uint32_t>> block_postings_head(NUM_BLOCKS, std::vector<uint32_t>(num_terms + 1, 0));
    std::vector<std::vector<uint16_t>> block_postings_data(NUM_BLOCKS);

    // Reusable thread_counts buffer: 12 * 75747 * 2 = 1.81 MB!
    std::vector<std::vector<uint16_t>> thread_counts(num_threads, std::vector<uint16_t>(num_terms, 0));
    std::vector<std::vector<uint32_t>> thread_write_offsets(num_threads, std::vector<uint32_t>(num_terms, 0));

    // Process Block by Block with 2-pass per block: ZERO thread_tids vectors allocated!
    for (uint32_t b = 0; b < NUM_BLOCKS; ++b) {
        uint32_t b_d_start = block_d_start[b];
        uint32_t b_d_end = block_d_end[b];
        uint32_t b_num_docs = b_d_end - b_d_start;
        uint32_t b_num_words = (b_num_docs + 63) / 64;
        uint32_t b_words_per_thread = (b_num_words + num_threads - 1) / num_threads;

        std::vector<uint32_t> b_thread_d_start(num_threads + 1);
        std::vector<const char*> b_thread_chunk_starts(num_threads + 1);
        for (uint32_t t = 0; t <= num_threads; ++t) {
            uint32_t w = std::min(b_num_words, t * b_words_per_thread);
            uint32_t did = std::min(b_num_docs, w * 64);
            b_thread_d_start[t] = b_d_start + did;
            b_thread_chunk_starts[t] = find_doc_start(ddata, dsize, b_thread_d_start[t]);
        }

        for (unsigned int t = 0; t < num_threads; ++t) {
            std::memset(thread_counts[t].data(), 0, num_terms * sizeof(uint16_t));
        }

        // PASS 1 for Block b: Count frequencies & populate hot bitsets directly
        ThreadRunner::run(num_threads, [&](unsigned int t) {
            uint32_t d_start = b_thread_d_start[t];
            uint32_t d_end = b_thread_d_start[t + 1];
            auto& counts = thread_counts[t];
            std::vector<uint32_t> last_seen_doc(num_terms, UINT32_MAX);

            const char* ptr = b_thread_chunk_starts[t];
            const char* end = b_thread_chunk_starts[t + 1];

            for (uint32_t doc_id = d_start; doc_id < d_end; ++doc_id) {
                while (ptr < end && *ptr != '\t') ptr++;
                if (ptr < end) ptr++;

                while (ptr < end && *ptr != '\n') {
                    while (ptr < end && *ptr != '\n' && CHAR_MAP[(uint8_t)*ptr] == 0) ptr++;
                    if (ptr >= end || *ptr == '\n') break;

                    uint64_t h = 14695981039346656037ULL;
                    while (ptr < end) {
                        uint8_t lc = CHAR_MAP[(uint8_t)*ptr];
                        if (lc == 0) break;
                        h = (h ^ lc) * 1099511628211ULL;
                        ptr++;
                    }

                    uint32_t q_bit = (uint32_t)h & 0xFFFFF;
                    if ((QUERY_TERM_FILTER[q_bit >> 6] & (1ULL << (q_bit & 63))) == 0) continue;

                    uint32_t tid = terms.lookup(h);
                    if (tid != UINT32_MAX) {
                        if (last_seen_doc[tid] != doc_id) {
                            last_seen_doc[tid] = doc_id;
                            uint32_t hid = hot_id[tid];
                            if (hid != UINT32_MAX) {
                                hot_bitsets[(size_t)hid * num_words + (doc_id / 64)] |= (1ULL << (doc_id % 64));
                            } else {
                                counts[tid]++;
                            }
                        }
                    }
                }
                if (ptr < end && *ptr == '\n') ptr++;
            }
        });

        // Prefix sum for Block b
        auto& head_b = block_postings_head[b];
        uint32_t block_total = 0;
        for (uint32_t tid = 0; tid < num_terms; ++tid) {
            head_b[tid] = block_total;
            if (hot_id[tid] == UINT32_MAX) {
                for (unsigned int t = 0; t < num_threads; ++t) {
                    block_total += thread_counts[t][tid];
                }
            }
        }
        head_b[num_terms] = block_total;

        for (uint32_t tid = 0; tid < num_terms; ++tid) {
            if (hot_id[tid] == UINT32_MAX) {
                uint32_t cur = head_b[tid];
                for (unsigned int t = 0; t < num_threads; ++t) {
                    thread_write_offsets[t][tid] = cur;
                    cur += thread_counts[t][tid];
                }
            }
        }

        // Allocate uint16_t postings buffer for Block b (~6.6 MB)
        block_postings_data[b].resize(block_total);
        auto& data_b = block_postings_data[b];

        // PASS 2 for Block b: Write directly into block_postings_data[b] & record doc_text_off / len
        ThreadRunner::run(num_threads, [&](unsigned int t) {
            uint32_t d_start = b_thread_d_start[t];
            uint32_t d_end = b_thread_d_start[t + 1];
            auto& offsets = thread_write_offsets[t];
            std::vector<uint32_t> last_seen_doc(num_terms, UINT32_MAX);

            const char* ptr = b_thread_chunk_starts[t];
            const char* end = b_thread_chunk_starts[t + 1];
            uint32_t base = block_d_start[b];

            for (uint32_t doc_id = d_start; doc_id < d_end; ++doc_id) {
                while (ptr < end && *ptr != '\t') ptr++;
                if (ptr < end) ptr++;
                const char* text_start = ptr;

                while (ptr < end && *ptr != '\n') {
                    while (ptr < end && *ptr != '\n' && CHAR_MAP[(uint8_t)*ptr] == 0) ptr++;
                    if (ptr >= end || *ptr == '\n') break;

                    uint64_t h = 14695981039346656037ULL;
                    while (ptr < end) {
                        uint8_t lc = CHAR_MAP[(uint8_t)*ptr];
                        if (lc == 0) break;
                        h = (h ^ lc) * 1099511628211ULL;
                        ptr++;
                    }

                    uint32_t q_bit = (uint32_t)h & 0xFFFFF;
                    if ((QUERY_TERM_FILTER[q_bit >> 6] & (1ULL << (q_bit & 63))) == 0) continue;

                    uint32_t tid = terms.lookup(h);
                    if (tid != UINT32_MAX && hot_id[tid] == UINT32_MAX) {
                        if (last_seen_doc[tid] != doc_id) {
                            last_seen_doc[tid] = doc_id;
                            uint16_t local_d = (uint16_t)(doc_id - base);
                            data_b[offsets[tid]++] = local_d;
                        }
                    }
                }

                doc_text_off[doc_id] = (uint32_t)(text_start - ddata);
                doc_text_len[doc_id] = (uint16_t)(ptr - text_start);
                if (ptr < end && *ptr == '\n') ptr++;
            }
        });
    }
    // Free intermediate offsets
    std::vector<std::vector<uint16_t>>().swap(thread_counts);
    std::vector<std::vector<uint32_t>>().swap(thread_write_offsets);

    // Fast seed-anchored phrase verification
    auto seed_phrase_match = [&](uint32_t doc_id, const std::string_view* p_views, size_t k, size_t seed_idx) -> bool {
        const char* doc_start = ddata + doc_text_off[doc_id];
        const char* doc_end = doc_start + doc_text_len[doc_id];
        std::string_view seed_w = p_views[seed_idx];
        size_t seed_len = seed_w.size();
        if (doc_end - doc_start < (ptrdiff_t)seed_len) return false;

        uint8_t c0 = (uint8_t)seed_w[0];
        uint8_t c0_upper = (c0 >= 'a' && c0 <= 'z') ? (c0 - 32) : c0;

        const char* ptr = doc_start;
        const char* scan_end = doc_end - seed_len + 1;

        while (ptr < scan_end) {
#if defined(__AVX2__)
            ptr = avx2_find_char_pair(ptr, scan_end, c0, c0_upper);
            if (ptr >= scan_end) break;
#else
            uint8_t c = (uint8_t)*ptr;
            if (c != c0 && c != c0_upper) {
                ptr++;
                continue;
            }
#endif

            if (ptr > doc_start && CHAR_MAP[(uint8_t)*(ptr - 1)] != 0) {
                ptr++;
                continue;
            }

            bool seed_match = true;
            for (size_t b = 1; b < seed_len; ++b) {
                if (CHAR_MAP[(uint8_t)ptr[b]] != (uint8_t)seed_w[b]) {
                    seed_match = false;
                    break;
                }
            }
            if (!seed_match) {
                ptr++;
                continue;
            }

            if (ptr + seed_len < doc_end && CHAR_MAP[(uint8_t)*(ptr + seed_len)] != 0) {
                ptr++;
                continue;
            }

            bool phrase_ok = true;
            const char* cur = ptr + seed_len;
            for (size_t j = seed_idx + 1; j < k; ++j) {
                while (cur < doc_end && CHAR_MAP[(uint8_t)*cur] == 0) cur++;
                size_t wlen = p_views[j].size();
                if (cur + wlen > doc_end) { phrase_ok = false; break; }
                for (size_t b = 0; b < wlen; ++b) {
                    if (CHAR_MAP[(uint8_t)cur[b]] != (uint8_t)p_views[j][b]) {
                        phrase_ok = false;
                        break;
                    }
                }
                if (!phrase_ok) break;
                cur += wlen;
                if (cur < doc_end && CHAR_MAP[(uint8_t)*cur] != 0) { phrase_ok = false; break; }
            }

            if (!phrase_ok) {
                ptr += seed_len;
                continue;
            }

            cur = ptr;
            for (int j = (int)seed_idx - 1; j >= 0; --j) {
                while (cur > doc_start && CHAR_MAP[(uint8_t)*(cur - 1)] == 0) cur--;
                size_t wlen = p_views[j].size();
                if (cur - wlen < doc_start) { phrase_ok = false; break; }
                const char* w_start = cur - wlen;
                if (w_start > doc_start && CHAR_MAP[(uint8_t)*(w_start - 1)] != 0) { phrase_ok = false; break; }
                for (size_t b = 0; b < wlen; ++b) {
                    if (CHAR_MAP[(uint8_t)w_start[b]] != (uint8_t)p_views[j][b]) {
                        phrase_ok = false;
                        break;
                    }
                }
                if (!phrase_ok) break;
                cur = w_start;
            }

            if (phrase_ok) return true;
            ptr += seed_len;
        }

        return false;
    };

    // Parallel query evaluation
    size_t num_queries = queries.size();
    std::vector<std::string> thread_outputs(num_threads);

    ThreadRunner::run(num_threads, [&](unsigned int t) {
        size_t q_start = (num_queries * t) / num_threads;
        size_t q_end = (num_queries * (t + 1)) / num_threads;
        auto& out_str = thread_outputs[t];
        out_str.reserve((q_end - q_start) * 40);

        std::vector<uint64_t> bitset_cur(num_words);
        std::vector<uint64_t> bitset_tmp(num_words, 0);
        char line_buf[256];

        for (size_t qi = q_start; qi < q_end; ++qi) {
            const auto& q = queries[qi];
            if (q.type == QueryType::BOOL) {
                const auto& bq = q.bq;
                uint32_t t0 = bq.term_ids[0];
                uint32_t hid0 = hot_id[t0];

                uint32_t total_df0 = 0;
                if (hid0 == UINT32_MAX) {
                    for (uint32_t b = 0; b < NUM_BLOCKS; ++b) {
                        total_df0 += (block_postings_head[b][t0 + 1] - block_postings_head[b][t0]);
                    }
                }

                // FAST PATH: Hybrid Sparse-Doc Evaluation across 4 blocks
                if (!bq.has_or && hid0 == UINT32_MAX && total_df0 <= 1024) {
                    uint64_t total_cnt = 0;
                    uint64_t total_sum = 0;
                    uint32_t top10[10];
                    uint32_t top_cnt = 0;

                    for (uint32_t b = 0; b < NUM_BLOCKS; ++b) {
                        uint32_t p0_start = block_postings_head[b][t0];
                        uint32_t p0_end = block_postings_head[b][t0 + 1];
                        uint32_t df0_b = p0_end - p0_start;
                        if (df0_b == 0) continue;

                        uint16_t cur_local[1024];
                        uint32_t cur_cnt = df0_b;
                        std::memcpy(cur_local, block_postings_data[b].data() + p0_start, df0_b * sizeof(uint16_t));
                        uint32_t base = block_d_start[b];

                        for (size_t op_i = 0; op_i < bq.ops.size() && cur_cnt > 0; ++op_i) {
                            BoolOp op = bq.ops[op_i];
                            uint32_t term_id = bq.term_ids[op_i + 1];
                            uint32_t hid = hot_id[term_id];

                            uint32_t new_cnt = 0;
                            if (hid != UINT32_MAX) {
                                const uint64_t* bptr = hot_bitsets.data() + (size_t)hid * num_words;
                                if (op == BoolOp::AND) {
                                    for (uint32_t i = 0; i < cur_cnt; ++i) {
                                        uint32_t d = base + cur_local[i];
                                        if ((bptr[d / 64] >> (d % 64)) & 1) cur_local[new_cnt++] = cur_local[i];
                                    }
                                } else { // NOT
                                    for (uint32_t i = 0; i < cur_cnt; ++i) {
                                        uint32_t d = base + cur_local[i];
                                        if (!((bptr[d / 64] >> (d % 64)) & 1)) cur_local[new_cnt++] = cur_local[i];
                                    }
                                }
                            } else {
                                uint32_t pstart = block_postings_head[b][term_id];
                                uint32_t pend = block_postings_head[b][term_id + 1];
                                const uint16_t* pdata = block_postings_data[b].data();
                                if (op == BoolOp::AND) {
                                    for (uint32_t i = 0; i < cur_cnt; ++i) {
                                        if (std::binary_search(pdata + pstart, pdata + pend, cur_local[i])) {
                                            cur_local[new_cnt++] = cur_local[i];
                                        }
                                    }
                                } else { // NOT
                                    for (uint32_t i = 0; i < cur_cnt; ++i) {
                                        if (!std::binary_search(pdata + pstart, pdata + pend, cur_local[i])) {
                                            cur_local[new_cnt++] = cur_local[i];
                                        }
                                    }
                                }
                            }
                            cur_cnt = new_cnt;
                        }

                        if (cur_cnt > 0) {
                            total_cnt += cur_cnt;
                            for (uint32_t i = 0; i < cur_cnt; ++i) {
                                uint32_t d = base + cur_local[i];
                                total_sum += d;
                                if (top_cnt < 10) top10[top_cnt++] = d;
                            }
                        }
                    }

                    if (total_cnt == 0) {
                        out_str.append("0 0\n");
                    } else {
                        char* p = line_buf;
                        p = u64_to_str(p, total_cnt);
                        *p++ = ' ';
                        p = u64_to_str(p, total_sum);
                        for (uint32_t i = 0; i < top_cnt; ++i) {
                            *p++ = ' ';
                            p = u64_to_str(p, top10[i]);
                        }
                        *p++ = '\n';
                        out_str.append(line_buf, p - line_buf);
                    }
                    continue;
                }

                // Dense bitset evaluation
                bool is_cur_zero = false;
                if (hid0 != UINT32_MAX) {
                    std::memcpy(bitset_cur.data(), hot_bitsets.data() + (size_t)hid0 * num_words, num_words * sizeof(uint64_t));
                } else {
                    if (total_df0 == 0) {
                        is_cur_zero = true;
                    } else {
                        std::memset(bitset_cur.data(), 0, num_words * sizeof(uint64_t));
                        for (uint32_t b = 0; b < NUM_BLOCKS; ++b) {
                            uint32_t pstart = block_postings_head[b][t0];
                            uint32_t pend = block_postings_head[b][t0 + 1];
                            uint32_t base = block_d_start[b];
                            const uint16_t* pdata = block_postings_data[b].data();
                            for (uint32_t i = pstart; i < pend; ++i) {
                                uint32_t d = base + pdata[i];
                                bitset_cur[d / 64] |= (1ULL << (d % 64));
                            }
                        }
                    }
                }

                if (is_cur_zero && !bq.has_or) {
                    out_str.append("0 0\n");
                    continue;
                }

                for (size_t op_i = 0; op_i < bq.ops.size(); ++op_i) {
                    BoolOp op = bq.ops[op_i];
                    uint32_t term_id = bq.term_ids[op_i + 1];

                    if (is_cur_zero) {
                        if (op == BoolOp::AND || op == BoolOp::NOT) {
                            continue;
                        } else {
                            uint32_t hid = hot_id[term_id];
                            if (hid != UINT32_MAX) {
                                std::memcpy(bitset_cur.data(), hot_bitsets.data() + (size_t)hid * num_words, num_words * sizeof(uint64_t));
                                is_cur_zero = false;
                            } else {
                                std::memset(bitset_cur.data(), 0, num_words * sizeof(uint64_t));
                                for (uint32_t b = 0; b < NUM_BLOCKS; ++b) {
                                    uint32_t pstart = block_postings_head[b][term_id];
                                    uint32_t pend = block_postings_head[b][term_id + 1];
                                    uint32_t base = block_d_start[b];
                                    const uint16_t* pdata = block_postings_data[b].data();
                                    for (uint32_t i = pstart; i < pend; ++i) {
                                        uint32_t d = base + pdata[i];
                                        bitset_cur[d / 64] |= (1ULL << (d % 64));
                                    }
                                }
                                is_cur_zero = false;
                            }
                            continue;
                        }
                    }

                    uint32_t hid = hot_id[term_id];
                    const uint64_t* term_bits = nullptr;

                    if (hid != UINT32_MAX) {
                        term_bits = hot_bitsets.data() + (size_t)hid * num_words;
                    } else {
                        for (uint32_t b = 0; b < NUM_BLOCKS; ++b) {
                            uint32_t pstart = block_postings_head[b][term_id];
                            uint32_t pend = block_postings_head[b][term_id + 1];
                            uint32_t base = block_d_start[b];
                            const uint16_t* pdata = block_postings_data[b].data();
                            for (uint32_t i = pstart; i < pend; ++i) {
                                uint32_t d = base + pdata[i];
                                bitset_tmp[d / 64] |= (1ULL << (d % 64));
                            }
                        }
                        term_bits = bitset_tmp.data();
                    }

                    if (op == BoolOp::AND) {
                        is_cur_zero = bitset_and(bitset_cur.data(), term_bits, num_words);
                    } else if (op == BoolOp::OR) {
                        bitset_or(bitset_cur.data(), term_bits, num_words);
                    } else if (op == BoolOp::NOT) {
                        is_cur_zero = bitset_andnot(bitset_cur.data(), term_bits, num_words);
                    }

                    if (hid == UINT32_MAX) {
                        for (uint32_t b = 0; b < NUM_BLOCKS; ++b) {
                            uint32_t pstart = block_postings_head[b][term_id];
                            uint32_t pend = block_postings_head[b][term_id + 1];
                            uint32_t base = block_d_start[b];
                            const uint16_t* pdata = block_postings_data[b].data();
                            for (uint32_t i = pstart; i < pend; ++i) {
                                bitset_tmp[(base + pdata[i]) / 64] = 0;
                            }
                        }
                    }

                    if (is_cur_zero) {
                        bool has_rem_or = false;
                        for (size_t j = op_i + 1; j < bq.ops.size(); ++j) {
                            if (bq.ops[j] == BoolOp::OR) { has_rem_or = true; break; }
                        }
                        if (!has_rem_or) break;
                    }
                }

                if (is_cur_zero) {
                    out_str.append("0 0\n");
                    continue;
                }

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
                size_t k = pq.term_ids.size();
                std::string_view p_views[8];
                for (size_t j = 0; j < k; ++j) {
                    p_views[j] = get_term_view(pq.term_ids[j]);
                }

                bool all_hot = true;
                for (uint32_t tid : pq.term_ids) {
                    if (hot_id[tid] == UINT32_MAX) {
                        all_hot = false;
                        break;
                    }
                }

                std::vector<uint32_t> candidates;
                size_t seed_idx = 0;

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
                    seed_idx = 0;
                } else {
                    size_t min_idx = SIZE_MAX;
                    uint32_t min_size = UINT32_MAX;
                    for (size_t i = 0; i < pq.term_ids.size(); ++i) {
                        uint32_t tid = pq.term_ids[i];
                        if (hot_id[tid] == UINT32_MAX) {
                            uint32_t sz = 0;
                            for (uint32_t b = 0; b < NUM_BLOCKS; ++b) {
                                sz += (block_postings_head[b][tid + 1] - block_postings_head[b][tid]);
                            }
                            if (sz < min_size) {
                                min_size = sz;
                                min_idx = i;
                            }
                        }
                    }

                    seed_idx = min_idx;
                    uint32_t hot_check_hids[8];
                    uint32_t hot_check_cnt = 0;
                    uint32_t non_hot_tids[8];
                    uint32_t non_hot_cnt = 0;

                    for (size_t i = 0; i < pq.term_ids.size(); ++i) {
                        if (i == min_idx) continue;
                        uint32_t tid = pq.term_ids[i];
                        uint32_t hid = hot_id[tid];
                        if (hid != UINT32_MAX) {
                            hot_check_hids[hot_check_cnt++] = hid;
                        } else {
                            non_hot_tids[non_hot_cnt++] = tid;
                        }
                    }

                    for (uint32_t b = 0; b < NUM_BLOCKS; ++b) {
                        uint32_t seed_start = block_postings_head[b][pq.term_ids[min_idx]];
                        uint32_t seed_end = block_postings_head[b][pq.term_ids[min_idx] + 1];
                        uint32_t base = block_d_start[b];
                        const uint16_t* pdata = block_postings_data[b].data();

                        for (uint32_t idx = seed_start; idx < seed_end; ++idx) {
                            uint16_t local_d = pdata[idx];
                            uint32_t d = base + local_d;
                            bool has_all = true;
                            for (uint32_t i = 0; i < hot_check_cnt; ++i) {
                                const uint64_t* bptr = hot_bitsets.data() + (size_t)hot_check_hids[i] * num_words;
                                if (!((bptr[d / 64] >> (d % 64)) & 1)) {
                                    has_all = false;
                                    break;
                                }
                            }
                            if (!has_all) continue;
                            for (uint32_t i = 0; i < non_hot_cnt; ++i) {
                                uint32_t tid = non_hot_tids[i];
                                uint32_t pstart = block_postings_head[b][tid];
                                uint32_t pend = block_postings_head[b][tid + 1];
                                if (!std::binary_search(pdata + pstart, pdata + pend, local_d)) {
                                    has_all = false;
                                    break;
                                }
                            }
                            if (has_all) candidates.push_back(d);
                        }
                    }
                }

                std::vector<uint32_t> matches;
                for (uint32_t d : candidates) {
                    if (seed_phrase_match(d, p_views, k, seed_idx)) {
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
