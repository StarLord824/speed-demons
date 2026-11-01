#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <atomic>
#include <cstring>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

// regions.csv IDs are in [1..999,999]. Anything >= 1,000,000 is UNMATCHED (255).
static uint8_t REGION_TO_COUNTRY[1000000];

struct CountryInfo {
    char code[4];
};
static CountryInfo COUNTRIES[256];
static uint32_t NUM_COUNTRIES = 0;
static uint16_t CODE_TO_CID[26 * 26 * 26];

inline uint64_t fnv1a_64_update(uint64_t h, uint8_t b) {
    return (h ^ b) * 1099511628211ULL;
}

// Global lock-free 262,144 slot hash table mapping 64-bit hash -> slot index
static const uint32_t TABLE_SIZE = 262144;
static const uint32_t TABLE_MASK = TABLE_SIZE - 1;
static std::atomic<uint64_t> NAME_TABLE[TABLE_SIZE];

// Bitset: 256 countries x 4096 uint64 = 8 MB
static std::atomic<uint64_t> COUNTRY_BITS[256][4096];

struct ThreadCountryAgg {
    uint64_t count = 0;
    int64_t id_sum = 0;
    int64_t amount_sum = 0;
    int64_t amount_min = 0;
    int64_t amount_max = 0;
    uint64_t name_bytes = 0;
    uint64_t note_bytes = 0;
    bool has_record = false;
};

struct ThreadContext {
    ThreadCountryAgg aggs[256];
};

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

int main(int argc, char** argv) {
    std::string data_dir = "/data";
    std::string out_dir = "/out";
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--data" && i + 1 < argc) data_dir = argv[++i];
        else if (std::string(argv[i]) == "--out" && i + 1 < argc) out_dir = argv[++i];
    }

    std::memset(REGION_TO_COUNTRY, 0xFF, sizeof(REGION_TO_COUNTRY));
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
        uint32_t reg_id = 0;
        while (*rptr >= '0' && *rptr <= '9') {
            reg_id = reg_id * 10 + (*rptr++ - '0');
        }
        if (*rptr == ',') rptr++;
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

    COUNTRIES[255].code[0] = '_';
    COUNTRIES[255].code[1] = 'U';
    COUNTRIES[255].code[2] = 'N';

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
    if (num_threads > std::thread::hardware_concurrency() && std::thread::hardware_concurrency() > 0) {
        num_threads = std::thread::hardware_concurrency();
    }

    size_t records_bytes = file_end - file_start;
    std::vector<const char*> chunk_starts(num_threads + 1);
    chunk_starts[0] = file_start;
    chunk_starts[num_threads] = file_end;

    for (unsigned int t = 1; t < num_threads; ++t) {
        const char* approx = file_start + (records_bytes / num_threads) * t;
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

    std::vector<ThreadContext> tcontexts(num_threads);
    std::vector<std::thread> workers;

    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            const char* ptr = chunk_starts[t];
            const char* end = chunk_starts[t + 1];
            auto& aggs = tcontexts[t].aggs;

            while (ptr < end) {
                if (ptr + 3 <= end && (uint8_t)ptr[0] == 0xEF && (uint8_t)ptr[1] == 0xBB && (uint8_t)ptr[2] == 0xBF) {
                    ptr += 3;
                }
                if (ptr >= end) break;

                // 1. id
                uint64_t rec_id = 0;
                while (*ptr >= '0' && *ptr <= '9') {
                    rec_id = rec_id * 10 + (*ptr++ - '0');
                }
                ptr++; // skip ','

                // 2. name: stream fnv1a_64 directly
                uint64_t nhash = 14695981039346656037ULL;
                uint16_t name_len = 0;

                if (*ptr == '"') {
                    ptr++;
                    while (ptr < end) {
                        if (*ptr == '"') {
                            if (ptr + 1 < end && *(ptr + 1) == '"') {
                                nhash = fnv1a_64_update(nhash, '"');
                                name_len++;
                                ptr += 2;
                            } else {
                                ptr++;
                                break;
                            }
                        } else {
                            nhash = fnv1a_64_update(nhash, (uint8_t)*ptr++);
                            name_len++;
                        }
                    }
                } else {
                    while (ptr < end && *ptr != ',') {
                        nhash = fnv1a_64_update(nhash, (uint8_t)*ptr++);
                        name_len++;
                    }
                }
                ptr++; // skip ','

                // 3. note: only track byte count
                uint32_t note_bytes = 0;
                if (*ptr == '"') {
                    ptr++;
                    while (ptr < end) {
                        if (*ptr == '"') {
                            if (ptr + 1 < end && *(ptr + 1) == '"') {
                                note_bytes++;
                                ptr += 2;
                            } else {
                                ptr++;
                                break;
                            }
                        } else {
                            note_bytes++;
                            ptr++;
                        }
                    }
                } else {
                    while (ptr < end && *ptr != ',') {
                        note_bytes++;
                        ptr++;
                    }
                }
                ptr++; // skip ','

                // 4. region_id
                uint32_t reg_id = 0;
                while (*ptr >= '0' && *ptr <= '9') {
                    reg_id = reg_id * 10 + (*ptr++ - '0');
                }
                ptr++; // skip ','

                // 5. amount
                bool neg = (*ptr == '-');
                if (neg) ptr++;
                int64_t cents = 0;
                while (*ptr >= '0' && *ptr <= '9') {
                    cents = cents * 10 + (*ptr++ - '0');
                }
                if (*ptr == '.') ptr++;
                cents = cents * 100 + (*ptr - '0') * 10 + (*(ptr + 1) - '0');
                ptr += 2;
                if (neg) cents = -cents;

                if (ptr < end && *ptr == '\r') ptr++;
                if (ptr < end && *ptr == '\n') ptr++;

                uint8_t cid = (reg_id < 1000000) ? REGION_TO_COUNTRY[reg_id] : 255;
                ThreadCountryAgg& agg = aggs[cid];
                if (!agg.has_record) {
                    agg.has_record = true;
                    agg.count = 1;
                    agg.id_sum = rec_id;
                    agg.amount_sum = cents;
                    agg.amount_min = cents;
                    agg.amount_max = cents;
                    agg.name_bytes = name_len;
                    agg.note_bytes = note_bytes;
                } else {
                    agg.count++;
                    agg.id_sum += rec_id;
                    agg.amount_sum += cents;
                    if (cents < agg.amount_min) agg.amount_min = cents;
                    if (cents > agg.amount_max) agg.amount_max = cents;
                    agg.name_bytes += name_len;
                    agg.note_bytes += note_bytes;
                }

                // Global Lock-Free Name Slot Lookup
                if (nhash == 0) nhash = 1;
                uint32_t slot = (uint32_t)nhash & TABLE_MASK;
                while (true) {
                    uint64_t val = NAME_TABLE[slot].load(std::memory_order_relaxed);
                    if (val == nhash) break;
                    if (val == 0) {
                        uint64_t expected = 0;
                        if (NAME_TABLE[slot].compare_exchange_strong(expected, nhash, std::memory_order_relaxed)) {
                            break;
                        }
                        if (expected == nhash) break;
                    }
                    slot = (slot + 1) & TABLE_MASK;
                }

                // Mark bit in country bitset
                COUNTRY_BITS[cid][slot >> 6].fetch_or(1ULL << (slot & 63), std::memory_order_relaxed);
            }
        });
    }

    for (auto& w : workers) w.join();
    workers.clear();
    munmap((void*)rec_data, rec_size);

    struct FinalAgg {
        uint64_t count = 0;
        int64_t id_sum = 0;
        int64_t amount_sum = 0;
        int64_t amount_min = 0;
        int64_t amount_max = 0;
        uint64_t names_distinct = 0;
        uint64_t name_bytes = 0;
        uint64_t note_bytes = 0;
        bool has_record = false;
        std::string country_code;
    };

    std::vector<FinalAgg> final_aggs(256);
    for (int c = 0; c < 256; ++c) {
        final_aggs[c].country_code = (c == 255) ? "_UNMATCHED" : std::string(COUNTRIES[c].code, 3);
        bool first = true;
        for (unsigned int t = 0; t < num_threads; ++t) {
            const auto& tagg = tcontexts[t].aggs[c];
            if (!tagg.has_record) continue;
            if (first) {
                final_aggs[c].has_record = true;
                final_aggs[c].count = tagg.count;
                final_aggs[c].id_sum = tagg.id_sum;
                final_aggs[c].amount_sum = tagg.amount_sum;
                final_aggs[c].amount_min = tagg.amount_min;
                final_aggs[c].amount_max = tagg.amount_max;
                final_aggs[c].name_bytes = tagg.name_bytes;
                final_aggs[c].note_bytes = tagg.note_bytes;
                first = false;
            } else {
                final_aggs[c].count += tagg.count;
                final_aggs[c].id_sum += tagg.id_sum;
                final_aggs[c].amount_sum += tagg.amount_sum;
                if (tagg.amount_min < final_aggs[c].amount_min) final_aggs[c].amount_min = tagg.amount_min;
                if (tagg.amount_max > final_aggs[c].amount_max) final_aggs[c].amount_max = tagg.amount_max;
                final_aggs[c].name_bytes += tagg.name_bytes;
                final_aggs[c].note_bytes += tagg.note_bytes;
            }
        }

        if (final_aggs[c].has_record) {
            uint64_t dist = 0;
            for (int w = 0; w < 4096; ++w) {
                uint64_t bits = COUNTRY_BITS[c][w].load(std::memory_order_relaxed);
                if (bits) dist += std::popcount(bits);
            }
            final_aggs[c].names_distinct = dist;
        }
    }

    std::vector<FinalAgg> active;
    for (int c = 0; c < 256; ++c) {
        if (final_aggs[c].has_record) active.push_back(final_aggs[c]);
    }
    std::sort(active.begin(), active.end(), [](const FinalAgg& a, const FinalAgg& b) {
        return a.country_code < b.country_code;
    });

    std::string out_buf;
    out_buf.reserve(65536);
    char num_tmp[64];

    for (const auto& row : active) {
        out_buf.append(row.country_code);
        out_buf.push_back(' ');

        char* p = u64_to_str(num_tmp, row.count);
        out_buf.append(num_tmp, p - num_tmp);
        out_buf.push_back(' ');

        p = u64_to_str(num_tmp, row.id_sum);
        out_buf.append(num_tmp, p - num_tmp);
        out_buf.push_back(' ');

        p = format_cents(num_tmp, row.amount_sum);
        out_buf.append(num_tmp, p - num_tmp);
        out_buf.push_back(' ');

        p = format_cents(num_tmp, row.amount_min);
        out_buf.append(num_tmp, p - num_tmp);
        out_buf.push_back(' ');

        p = format_cents(num_tmp, row.amount_max);
        out_buf.append(num_tmp, p - num_tmp);
        out_buf.push_back(' ');

        p = u64_to_str(num_tmp, row.names_distinct);
        out_buf.append(num_tmp, p - num_tmp);
        out_buf.push_back(' ');

        p = u64_to_str(num_tmp, row.name_bytes);
        out_buf.append(num_tmp, p - num_tmp);
        out_buf.push_back(' ');

        p = u64_to_str(num_tmp, row.note_bytes);
        out_buf.append(num_tmp, p - num_tmp);
        out_buf.push_back('\n');
    }

    mkdir(out_dir.c_str(), 0777);
    std::string out_path = out_dir + "/aggregates.txt";
    int out_fd = open(out_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (out_fd >= 0) {
        write(out_fd, out_buf.data(), out_buf.size());
        close(out_fd);
    }

    return 0;
}
