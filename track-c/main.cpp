#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <atomic>
#include <thread>
#include <cstring>
#include <chrono>
#include <bit>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>

#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#define HAS_AVX2 1
#else
#define HAS_AVX2 0
#endif

// regions.csv IDs are in [1..999,999]. Anything >= 1,000,000 is UNMATCHED (240).
static const uint8_t UNMATCHED_CID = 240;
static const uint32_t NUM_BUCKETS = 241;
static uint8_t REGION_TO_COUNTRY[1000000];

struct CountryInfo {
    char code[4];
};
static CountryInfo COUNTRIES[NUM_BUCKETS];
static uint32_t NUM_COUNTRIES = 0;
static uint16_t CODE_TO_CID[26 * 26 * 26];

inline uint64_t fast_hash(const char* p, size_t len) {
    if (len <= 8) {
        if (len >= 4) {
            uint32_t a, b;
            std::memcpy(&a, p, 4);
            std::memcpy(&b, p + len - 4, 4);
            uint64_t h = (((uint64_t)a << 32) | b) ^ (len * 0x9E3779B97F4A7C15ULL);
            h = (h ^ (h >> 30)) * 0xBF58476D1CE4E5B9ULL;
            return h ^ (h >> 27);
        } else if (len > 0) {
            uint64_t a = (uint8_t)p[0];
            uint64_t b = (uint8_t)p[len >> 1];
            uint64_t c = (uint8_t)p[len - 1];
            uint64_t h = ((a << 16) | (b << 8) | c) ^ (len * 0x9E3779B97F4A7C15ULL);
            h = (h ^ (h >> 30)) * 0xBF58476D1CE4E5B9ULL;
            return h ^ (h >> 27);
        }
        return 0x9E3779B97F4A7C15ULL;
    }
    if (len <= 16) {
        uint64_t a, b;
        std::memcpy(&a, p, 8);
        std::memcpy(&b, p + len - 8, 8);
        uint64_t h = (a ^ 0xBF58476D1CE4E5B9ULL) * 0x94D049BB133111EBULL + (b ^ (len * 0x9E3779B97F4A7C15ULL));
        h = (h ^ (h >> 30)) * 0xBF58476D1CE4E5B9ULL;
        return h ^ (h >> 27);
    }
    uint64_t h = len * 0x9E3779B97F4A7C15ULL;
    size_t i = 0;
    while (i + 8 <= len) {
        uint64_t v;
        std::memcpy(&v, p + i, 8);
        h = (h ^ v) * 0xBF58476D1CE4E5B9ULL;
        i += 8;
    }
    if (i < len) {
        uint64_t v;
        std::memcpy(&v, p + len - 8, 8);
        h = (h ^ v) * 0xBF58476D1CE4E5B9ULL;
    }
    h = (h ^ (h >> 30)) * 0xBF58476D1CE4E5B9ULL;
    return h ^ (h >> 27);
}

__attribute__((always_inline)) inline const char* find_quote_name(const char* ptr) {
#if HAS_AVX2
    __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
    uint32_t mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, _mm256_set1_epi8('"')));
    if (__builtin_expect(mask != 0, 1)) return ptr + std::countr_zero(mask);
    ptr += 32;
    while (true) {
        chunk = _mm256_loadu_si256((const __m256i*)ptr);
        mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, _mm256_set1_epi8('"')));
        if (mask != 0) return ptr + std::countr_zero(mask);
        ptr += 32;
    }
#else
    while (true) {
        uint64_t v;
        std::memcpy(&v, ptr, 8);
        uint64_t x = v ^ 0x2222222222222222ULL;
        uint64_t mask = (x - 0x0101010101010101ULL) & ~x & 0x8080808080808080ULL;
        if (mask != 0) return ptr + (std::countr_zero(mask) >> 3);
        ptr += 8;
    }
#endif
}

__attribute__((always_inline)) inline const char* find_quote_note(const char* ptr) {
#if HAS_AVX2
    const __m256i quotes = _mm256_set1_epi8('"');
    __m256i chunk0 = _mm256_loadu_si256((const __m256i*)ptr);
    uint32_t mask0 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk0, quotes));
    if (__builtin_expect(mask0 != 0, 1)) return ptr + std::countr_zero(mask0);

    __m256i chunk1 = _mm256_loadu_si256((const __m256i*)(ptr + 32));
    uint32_t mask1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk1, quotes));
    if (__builtin_expect(mask1 != 0, 1)) return ptr + 32 + std::countr_zero(mask1);

    ptr += 64;
    while (true) {
        __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
        uint32_t mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, quotes));
        if (mask != 0) return ptr + std::countr_zero(mask);
        ptr += 32;
    }
#else
    while (true) {
        uint64_t v;
        std::memcpy(&v, ptr, 8);
        uint64_t x = v ^ 0x2222222222222222ULL;
        uint64_t mask = (x - 0x0101010101010101ULL) & ~x & 0x8080808080808080ULL;
        if (mask != 0) return ptr + (std::countr_zero(mask) >> 3);
        ptr += 8;
    }
#endif
}

__attribute__((always_inline)) inline const char* find_comma(const char* ptr) {
#if HAS_AVX2
    const __m256i commas = _mm256_set1_epi8(',');
    __m256i chunk0 = _mm256_loadu_si256((const __m256i*)ptr);
    uint32_t mask0 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk0, commas));
    if (__builtin_expect(mask0 != 0, 1)) return ptr + std::countr_zero(mask0);

    __m256i chunk1 = _mm256_loadu_si256((const __m256i*)(ptr + 32));
    uint32_t mask1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk1, commas));
    if (__builtin_expect(mask1 != 0, 1)) return ptr + 32 + std::countr_zero(mask1);

    ptr += 64;
    while (true) {
        __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
        uint32_t mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, commas));
        if (mask != 0) return ptr + std::countr_zero(mask);
        ptr += 32;
    }
#else
    while (true) {
        uint64_t v;
        std::memcpy(&v, ptr, 8);
        uint64_t x = v ^ 0x2C2C2C2C2C2C2C2CULL;
        uint64_t mask = (x - 0x0101010101010101ULL) & ~x & 0x8080808080808080ULL;
        if (mask != 0) return ptr + (std::countr_zero(mask) >> 3);
        ptr += 8;
    }
#endif
}

static const uint32_t TABLE_SIZE = 131072;
static const uint32_t TABLE_MASK = TABLE_SIZE - 1;
static std::atomic<uint64_t> NAME_TABLE[TABLE_SIZE];
static std::atomic<uint64_t> COUNTRY_BITS[NUM_BUCKETS][2048];

struct ThreadCountryAgg {
    uint64_t count = 0;
    int64_t id_sum = 0;
    int64_t amount_sum = 0;
    int64_t amount_min = INT64_MAX;
    int64_t amount_max = INT64_MIN;
    uint64_t name_bytes = 0;
    uint64_t note_bytes = 0;
};

struct ThreadContext {
    ThreadCountryAgg aggs[NUM_BUCKETS];
};

static ThreadContext TCONTEXTS[16];

struct ChunkTask {
    const char* start;
    const char* end;
};
static ChunkTask TASKS[64];
static uint32_t NUM_TASKS = 0;
static std::atomic<uint32_t> NEXT_TASK_IDX{0};

inline char* u64_to_str(char* buf, uint64_t val) {
    if (val == 0) { *buf++ = '0'; return buf; }
    char tmp[24]; int idx = 0;
    while (val > 0) { tmp[idx++] = (char)('0' + (val % 10)); val /= 10; }
    while (idx > 0) { *buf++ = tmp[--idx]; }
    return buf;
}

inline char* format_cents(char* buf, int64_t cents) {
    if (cents < 0) {
        *buf++ = '-';
        cents = -cents;
    }
    uint64_t whole = (uint64_t)(cents / 100);
    uint32_t frac = (uint32_t)(cents % 100);
    buf = u64_to_str(buf, whole);
    *buf++ = '.';
    *buf++ = (char)('0' + (frac / 10));
    *buf++ = (char)('0' + (frac % 10));
    return buf;
}

inline bool is_record_start(const char* ptr, const char* end) {
    if (ptr >= end) return false;
    if (ptr + 3 <= end && (uint8_t)ptr[0] == 0xEF && (uint8_t)ptr[1] == 0xBB && (uint8_t)ptr[2] == 0xBF) {
        ptr += 3;
    }
    if (ptr >= end || *ptr < '0' || *ptr > '9') return false;
    while (ptr < end && *ptr >= '0' && *ptr <= '9') ptr++;
    return (ptr < end && *ptr == ',');
}

void* worker_thread(void* arg) {
    ThreadContext* ctx = (ThreadContext*)arg;
    auto& aggs = ctx->aggs;
    char unescape_buf[64];

    while (true) {
        uint32_t task_id = NEXT_TASK_IDX.fetch_add(1, std::memory_order_relaxed);
        if (task_id >= NUM_TASKS) break;

        const char* ptr = TASKS[task_id].start;
        const char* end = TASKS[task_id].end;

        while (ptr < end) {
            if (__builtin_expect((uint8_t)*ptr == 0xEF, 0)) {
                if (ptr + 3 <= end && (uint8_t)ptr[1] == 0xBB && (uint8_t)ptr[2] == 0xBF) {
                    ptr += 3;
                }
            }
            if (ptr >= end) break;

            // 1. id
            uint64_t rec_id = *ptr++ - '0';
            while (*ptr != ',') {
                rec_id = rec_id * 10 + (*ptr++ - '0');
            }
            ptr++; // skip ','

            // 2. name
            uint64_t nhash = 0;
            uint16_t name_len = 0;
            if (__builtin_expect(*ptr == '"', 1)) {
                ptr++;
                const char* start = ptr;
                const char* q = find_quote_name(ptr);
                if (__builtin_expect(*(q + 1) == ',', 1)) {
                    name_len = (uint16_t)(q - start);
                    nhash = fast_hash(start, name_len);
                    ptr = q + 2; // skip '"' and ','
                } else {
                    size_t prefix_len = q - start;
                    std::memcpy(unescape_buf, start, prefix_len);
                    int ulen = (int)prefix_len;
                    const char* cur = q;
                    while (true) {
                        if (*(cur + 1) == '"') {
                            unescape_buf[ulen++] = '"';
                            cur += 2;
                            const char* nq = find_quote_name(cur);
                            size_t seg = nq - cur;
                            std::memcpy(unescape_buf + ulen, cur, seg);
                            ulen += (int)seg;
                            cur = nq;
                        } else {
                            ptr = cur + 2; // skip '"' and ','
                            break;
                        }
                    }
                    name_len = (uint16_t)ulen;
                    nhash = fast_hash(unescape_buf, ulen);
                }
            } else {
                const char* start = ptr;
                const char* c = find_comma(ptr);
                name_len = (uint16_t)(c - start);
                nhash = fast_hash(start, name_len);
                ptr = c + 1;
            }

            if (nhash == 0) nhash = 1;
            uint32_t slot = (uint32_t)nhash & TABLE_MASK;
            __builtin_prefetch(&NAME_TABLE[slot], 0, 1);

            // 3. note
            uint32_t note_bytes = 0;
            if (__builtin_expect(*ptr == '"', 1)) {
                ptr++;
                const char* start = ptr;
                const char* q = find_quote_note(ptr);
                if (__builtin_expect(*(q + 1) == ',', 1)) {
                    note_bytes = (uint32_t)(q - start);
                    ptr = q + 2;
                } else {
                    uint32_t quote_pairs = 0;
                    while (true) {
                        if (*(q + 1) == '"') {
                            quote_pairs++;
                            q = find_quote_note(q + 2);
                        } else {
                            note_bytes = (uint32_t)(q - start) - quote_pairs;
                            ptr = q + 2; // skip '"' and ','
                            break;
                        }
                    }
                }
            } else {
                const char* start = ptr;
                const char* c = find_comma(ptr);
                note_bytes = (uint32_t)(c - start);
                ptr = c + 1;
            }

            // 4. region_id
            uint32_t reg_id = *ptr++ - '0';
            while (*ptr != ',') {
                reg_id = reg_id * 10 + (*ptr++ - '0');
            }
            ptr++; // skip ','

            uint8_t cid = (reg_id < 1000000) ? REGION_TO_COUNTRY[reg_id] : UNMATCHED_CID;
            uint32_t word_idx = slot >> 6;
            __builtin_prefetch(&COUNTRY_BITS[cid][word_idx], 1, 1);

            // 5. amount
            bool neg = (*ptr == '-');
            if (neg) ptr++;
            int64_t cents = *ptr++ - '0';
            while (*ptr != '.') {
                cents = cents * 10 + (*ptr++ - '0');
            }
            ptr++; // skip '.'
            cents = cents * 100 + ((uint8_t)ptr[0] * 10 + (uint8_t)ptr[1] - 528);
            ptr += 2;
            if (neg) cents = -cents;

            if (ptr < end && *ptr == '\r') ptr++;
            if (ptr < end && *ptr == '\n') ptr++;

            ThreadCountryAgg& agg = aggs[cid];
            agg.count++;
            agg.id_sum += rec_id;
            agg.amount_sum += cents;
            agg.amount_min = std::min(agg.amount_min, cents);
            agg.amount_max = std::max(agg.amount_max, cents);
            agg.name_bytes += name_len;
            agg.note_bytes += note_bytes;

            uint64_t val = NAME_TABLE[slot].load(std::memory_order_relaxed);
            if (__builtin_expect(val != nhash, 0)) {
                if (val == 0) {
                    uint64_t expected = 0;
                    if (!NAME_TABLE[slot].compare_exchange_strong(expected, nhash, std::memory_order_relaxed) && expected != nhash) {
                        uint32_t step = ((uint32_t)(nhash >> 17) | 1) & TABLE_MASK;
                        while (true) {
                            slot = (slot + step) & TABLE_MASK;
                            val = NAME_TABLE[slot].load(std::memory_order_relaxed);
                            if (val == nhash) break;
                            if (val == 0) {
                                expected = 0;
                                if (NAME_TABLE[slot].compare_exchange_strong(expected, nhash, std::memory_order_relaxed) || expected == nhash) break;
                            }
                        }
                        word_idx = slot >> 6;
                    }
                } else {
                    uint32_t step = ((uint32_t)(nhash >> 17) | 1) & TABLE_MASK;
                    while (true) {
                        slot = (slot + step) & TABLE_MASK;
                        val = NAME_TABLE[slot].load(std::memory_order_relaxed);
                        if (val == nhash) break;
                        if (val == 0) {
                            uint64_t expected = 0;
                            if (NAME_TABLE[slot].compare_exchange_strong(expected, nhash, std::memory_order_relaxed) || expected == nhash) break;
                        }
                    }
                    word_idx = slot >> 6;
                }
            }

            uint64_t bit_mask = 1ULL << (slot & 63);
            if (__builtin_expect((COUNTRY_BITS[cid][word_idx].load(std::memory_order_relaxed) & bit_mask) == 0, 1)) {
                COUNTRY_BITS[cid][word_idx].fetch_or(bit_mask, std::memory_order_relaxed);
            }
        }
    }
    return nullptr;
}

int main(int argc, char** argv) {
    std::string data_dir = "/data";
    std::string out_dir = "/out";
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--data" && i + 1 < argc) data_dir = argv[++i];
        else if (std::string(argv[i]) == "--out" && i + 1 < argc) out_dir = argv[++i];
    }

    std::memset(REGION_TO_COUNTRY, UNMATCHED_CID, sizeof(REGION_TO_COUNTRY));
    std::memset(CODE_TO_CID, 0xFF, sizeof(CODE_TO_CID));

    // 1. Parse regions.csv
    std::string reg_path = data_dir + "/regions.csv";
    int rfd = open(reg_path.c_str(), O_RDONLY);
    if (rfd < 0) return 1;
    struct stat rst;
    fstat(rfd, &rst);
    size_t rsize = rst.st_size;
    const char* rdata = (const char*)mmap(nullptr, rsize, PROT_READ, MAP_SHARED, rfd, 0);
    close(rfd);

    const char* rptr = rdata;
    const char* rend = rdata + rsize;
    while (rptr < rend && *rptr != '\n') rptr++;
    if (rptr < rend && *rptr == '\n') rptr++;

    auto get_country_id = [&](const char* code) -> uint8_t {
        uint32_t code_idx = (uint32_t)(code[0] - 'A') * 676 +
                            (uint32_t)(code[1] - 'A') * 26 +
                            (uint32_t)(code[2] - 'A');
        uint16_t cid = CODE_TO_CID[code_idx];
        if (cid != 0xFFFF) return (uint8_t)cid;

        uint8_t id = (uint8_t)NUM_COUNTRIES++;
        COUNTRIES[id].code[0] = code[0];
        COUNTRIES[id].code[1] = code[1];
        COUNTRIES[id].code[2] = code[2];
        COUNTRIES[id].code[3] = '\0';
        CODE_TO_CID[code_idx] = id;
        return id;
    };

    while (rptr < rend) {
        if (*rptr == '\r' || *rptr == '\n') { rptr++; continue; }
        uint32_t reg_id = *rptr++ - '0';
        while (*rptr != ',') {
            reg_id = reg_id * 10 + (*rptr++ - '0');
        }
        rptr++; // skip ','
        const char* code = rptr;
        rptr += 3;
        if (*rptr == '\r') rptr++;
        if (*rptr == '\n') rptr++;

        uint8_t cid = get_country_id(code);
        if (reg_id < 1000000) {
            REGION_TO_COUNTRY[reg_id] = cid;
        }
    }
    munmap((void*)rdata, rsize);

    COUNTRIES[UNMATCHED_CID].code[0] = '_';
    COUNTRIES[UNMATCHED_CID].code[1] = 'U';
    COUNTRIES[UNMATCHED_CID].code[2] = 'N';

    // 2. Open records.csv
    std::string rec_path = data_dir + "/records.csv";
    int rec_fd = open(rec_path.c_str(), O_RDONLY);
    if (rec_fd < 0) return 1;
    struct stat rec_st;
    fstat(rec_fd, &rec_st);
    size_t rec_size = rec_st.st_size;
    const char* rec_data = (const char*)mmap(nullptr, rec_size, PROT_READ, MAP_SHARED, rec_fd, 0);
    close(rec_fd);

    const char* file_start = rec_data;
    const char* file_end = rec_data + rec_size;

    if (file_start + 3 <= file_end && (uint8_t)file_start[0] == 0xEF && (uint8_t)file_start[1] == 0xBB && (uint8_t)file_start[2] == 0xBF) {
        file_start += 3;
    }
    while (file_start < file_end && *file_start != '\n') file_start++;
    if (file_start < file_end && *file_start == '\n') file_start++;

    unsigned int num_threads = 12;
    unsigned int hw = std::thread::hardware_concurrency();
    if (hw > 0 && hw < num_threads) {
        num_threads = hw;
    }

    size_t records_bytes = file_end - file_start;

    // Dynamic work-stealing tasks: ~4.0 MB per chunk (~48 tasks across 193 MB)
    size_t target_chunk_size = 4000000;
    size_t num_chunks = records_bytes / target_chunk_size;
    if (num_chunks < num_threads) num_chunks = num_threads;
    if (num_chunks > 48) num_chunks = 48;
    if (num_chunks < 1) num_chunks = 1;

    std::vector<const char*> chunk_starts(num_chunks + 1);
    chunk_starts[0] = file_start;
    chunk_starts[num_chunks] = file_end;

    for (size_t t = 1; t < num_chunks; ++t) {
        const char* approx = file_start + (records_bytes / num_chunks) * t;
        while (approx < file_end) {
            if (*approx == '\n') {
                const char* cand = approx + 1;
                if (is_record_start(cand, file_end)) {
                    approx = cand;
                    break;
                }
            }
            approx++;
        }
        chunk_starts[t] = approx;
    }

    NUM_TASKS = (uint32_t)num_chunks;
    for (size_t t = 0; t < num_chunks; ++t) {
        TASKS[t].start = chunk_starts[t];
        TASKS[t].end = chunk_starts[t + 1];
    }
    NEXT_TASK_IDX.store(0, std::memory_order_relaxed);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 65536);

    pthread_t threads[16];
    for (unsigned int t = 0; t < num_threads; ++t) {
        pthread_create(&threads[t], &attr, worker_thread, &TCONTEXTS[t]);
    }
    pthread_attr_destroy(&attr);

    for (unsigned int t = 0; t < num_threads; ++t) {
        pthread_join(threads[t], nullptr);
    }
    munmap((void*)rec_data, rec_size);

    struct FinalAgg {
        uint64_t count = 0;
        int64_t id_sum = 0;
        int64_t amount_sum = 0;
        int64_t amount_min = INT64_MAX;
        int64_t amount_max = INT64_MIN;
        uint64_t names_distinct = 0;
        uint64_t name_bytes = 0;
        uint64_t note_bytes = 0;
        bool has_record = false;
        std::string country_code;
    };

    std::vector<FinalAgg> final_aggs(NUM_BUCKETS);
    for (uint32_t c = 0; c < NUM_BUCKETS; ++c) {
        final_aggs[c].country_code = (c == UNMATCHED_CID) ? "_UNMATCHED" : std::string(COUNTRIES[c].code, 3);
        for (unsigned int t = 0; t < num_threads; ++t) {
            const auto& tagg = TCONTEXTS[t].aggs[c];
            if (tagg.count == 0) continue;
            final_aggs[c].has_record = true;
            final_aggs[c].count += tagg.count;
            final_aggs[c].id_sum += tagg.id_sum;
            final_aggs[c].amount_sum += tagg.amount_sum;
            final_aggs[c].amount_min = std::min(final_aggs[c].amount_min, tagg.amount_min);
            final_aggs[c].amount_max = std::max(final_aggs[c].amount_max, tagg.amount_max);
            final_aggs[c].name_bytes += tagg.name_bytes;
            final_aggs[c].note_bytes += tagg.note_bytes;
        }

        if (final_aggs[c].has_record) {
            uint64_t dist = 0;
            for (int w = 0; w < 2048; ++w) {
                uint64_t bits = COUNTRY_BITS[c][w].load(std::memory_order_relaxed);
                if (bits) dist += std::popcount(bits);
            }
            final_aggs[c].names_distinct = dist;
        }
    }

    std::vector<FinalAgg> active;
    for (uint32_t c = 0; c < NUM_BUCKETS; ++c) {
        if (final_aggs[c].has_record) active.push_back(final_aggs[c]);
    }
    std::sort(active.begin(), active.end(), [](const FinalAgg& a, const FinalAgg& b) {
        return a.country_code < b.country_code;
    });

    char out_buf[131072];
    char* out_ptr = out_buf;

    for (const auto& row : active) {
        std::memcpy(out_ptr, row.country_code.data(), row.country_code.size());
        out_ptr += row.country_code.size();
        *out_ptr++ = ' ';

        out_ptr = u64_to_str(out_ptr, row.count);
        *out_ptr++ = ' ';

        out_ptr = u64_to_str(out_ptr, row.id_sum);
        *out_ptr++ = ' ';

        out_ptr = format_cents(out_ptr, row.amount_sum);
        *out_ptr++ = ' ';

        out_ptr = format_cents(out_ptr, row.amount_min);
        *out_ptr++ = ' ';

        out_ptr = format_cents(out_ptr, row.amount_max);
        *out_ptr++ = ' ';

        out_ptr = u64_to_str(out_ptr, row.names_distinct);
        *out_ptr++ = ' ';

        out_ptr = u64_to_str(out_ptr, row.name_bytes);
        *out_ptr++ = ' ';

        out_ptr = u64_to_str(out_ptr, row.note_bytes);
        *out_ptr++ = '\n';
    }

    mkdir(out_dir.c_str(), 0777);
    std::string out_path = out_dir + "/aggregates.txt";
    int out_fd = open(out_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (out_fd >= 0) {
        ssize_t written = write(out_fd, out_buf, out_ptr - out_buf);
        (void)written;
        close(out_fd);
    }

    return 0;
}
