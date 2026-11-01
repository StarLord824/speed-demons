#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <thread>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

// At most 10,000,000 region IDs
static uint8_t REGION_TO_COUNTRY[10000000];

struct CountryInfo {
    char code[4];
};

static CountryInfo COUNTRIES[256];
static uint32_t NUM_COUNTRIES = 0;

inline uint64_t fnv1a_64(const char* s, size_t len) {
    uint64_t h = 14695981039346656037ULL;
    for (size_t i = 0; i < len; ++i) {
        h ^= (uint8_t)s[i];
        h *= 1099511628211ULL;
    }
    return h;
}

// Flat open-addressing hash set for distinct names
struct NameHashSet {
    struct Slot {
        uint64_t hash;
        uint32_t offset;
        uint16_t len;
    };
    std::vector<Slot> slots;
    uint32_t mask = 0;
    uint32_t count = 0;

    void init(uint32_t cap_pow2) {
        slots.assign(cap_pow2, {0, 0, 0});
        mask = cap_pow2 - 1;
        count = 0;
    }

    void insert(uint64_t h, uint32_t off, uint16_t len, const char* str_pool) {
        if (h == 0) h = 1;
        uint32_t idx = (uint32_t)h & mask;
        while (slots[idx].hash != 0) {
            if (slots[idx].hash == h && slots[idx].len == len) {
                if (std::memcmp(str_pool + slots[idx].offset, str_pool + off, len) == 0) {
                    return;
                }
            }
            idx = (idx + 1) & mask;
        }
        slots[idx] = {h, off, len};
        count++;
        if (count * 2 > mask) {
            uint32_t new_cap = (mask + 1) * 2;
            std::vector<Slot> old = std::move(slots);
            init(new_cap);
            for (const auto& s : old) {
                if (s.hash != 0) {
                    insert(s.hash, s.offset, s.len, str_pool);
                }
            }
        }
    }
};

struct ThreadAgg {
    uint64_t count = 0;
    int64_t id_sum = 0;
    int64_t amount_sum = 0;
    int64_t amount_min = 0;
    int64_t amount_max = 0;
    uint64_t name_bytes = 0;
    uint64_t note_bytes = 0;
    bool has_record = false;
    NameHashSet name_set;
    std::vector<char> name_pool;
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

// Check if ptr is at the start of a record
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

    std::string reg_path = data_dir + "/regions.csv";
    int rfd = open(reg_path.c_str(), O_RDONLY);
    if (rfd < 0) return 1;
    struct stat rst;
    if (fstat(rfd, &rst) < 0) { close(rfd); return 1; }
    size_t rsize = rst.st_size;
    const char* rdata = (const char*)mmap(nullptr, rsize, PROT_READ, MAP_SHARED, rfd, 0);
    close(rfd);

    const char* rptr = rdata;
    const char* rend = rdata + rsize;
    while (rptr < rend && *rptr != '\n') rptr++;
    if (rptr < rend && *rptr == '\n') rptr++;

    auto get_country_id = [&](const char* code) -> uint8_t {
        for (uint32_t i = 0; i < NUM_COUNTRIES; ++i) {
            if (COUNTRIES[i].code[0] == code[0] &&
                COUNTRIES[i].code[1] == code[1] &&
                COUNTRIES[i].code[2] == code[2]) {
                return (uint8_t)i;
            }
        }
        uint8_t id = (uint8_t)NUM_COUNTRIES++;
        COUNTRIES[id].code[0] = code[0];
        COUNTRIES[id].code[1] = code[1];
        COUNTRIES[id].code[2] = code[2];
        COUNTRIES[id].code[3] = '\0';
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
        if (reg_id < 10000000) {
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
    if (fstat(rec_fd, &rec_st) < 0) { close(rec_fd); return 1; }
    size_t rec_size = rec_st.st_size;
    const char* rec_data = (const char*)mmap(nullptr, rec_size, PROT_READ, MAP_SHARED, rec_fd, 0);
    close(rec_fd);
    madvise((void*)rec_data, rec_size, MADV_WILLNEED | MADV_SEQUENTIAL);

    const char* file_start = rec_data;
    const char* file_end = rec_data + rec_size;

    // Skip initial BOM
    if (file_start + 3 <= file_end && (uint8_t)file_start[0] == 0xEF && (uint8_t)file_start[1] == 0xBB && (uint8_t)file_start[2] == 0xBF) {
        file_start += 3;
    }
    // Skip header line
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

    std::vector<std::vector<ThreadAgg>> thread_aggs(num_threads);
    for (unsigned int t = 0; t < num_threads; ++t) {
        thread_aggs[t].resize(256);
        for (int c = 0; c < 256; ++c) {
            thread_aggs[t][c].name_set.init(256);
        }
    }

    std::vector<std::thread> workers;
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            const char* ptr = chunk_starts[t];
            const char* end = chunk_starts[t + 1];
            auto& aggs = thread_aggs[t];

            while (ptr < end) {
                // Check mid-file BOM
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

                // 2. name
                const char* name_unescaped_start = ptr;
                uint32_t name_bytes = 0;
                bool name_has_escapes = false;

                if (*ptr == '"') {
                    ptr++; // skip opening quote
                    name_unescaped_start = ptr;
                    while (ptr < end) {
                        if (*ptr == '"') {
                            if (ptr + 1 < end && *(ptr + 1) == '"') {
                                name_has_escapes = true;
                                name_bytes++;
                                ptr += 2;
                            } else {
                                ptr++; // skip closing quote
                                break;
                            }
                        } else {
                            name_bytes++;
                            ptr++;
                        }
                    }
                } else {
                    while (ptr < end && *ptr != ',') {
                        name_bytes++;
                        ptr++;
                    }
                }
                ptr++; // skip ','

                // 3. note
                uint32_t note_bytes = 0;
                if (*ptr == '"') {
                    ptr++; // skip opening quote
                    while (ptr < end) {
                        if (*ptr == '"') {
                            if (ptr + 1 < end && *(ptr + 1) == '"') {
                                note_bytes++;
                                ptr += 2;
                            } else {
                                ptr++; // skip closing quote
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

                uint8_t cid = (reg_id < 10000000) ? REGION_TO_COUNTRY[reg_id] : 255;
                ThreadAgg& agg = aggs[cid];
                if (!agg.has_record) {
                    agg.has_record = true;
                    agg.count = 1;
                    agg.id_sum = rec_id;
                    agg.amount_sum = cents;
                    agg.amount_min = cents;
                    agg.amount_max = cents;
                    agg.name_bytes = name_bytes;
                    agg.note_bytes = note_bytes;
                } else {
                    agg.count++;
                    agg.id_sum += rec_id;
                    agg.amount_sum += cents;
                    if (cents < agg.amount_min) agg.amount_min = cents;
                    if (cents > agg.amount_max) agg.amount_max = cents;
                    agg.name_bytes += name_bytes;
                    agg.note_bytes += note_bytes;
                }

                // Append unescaped name into thread-local pool
                uint32_t pool_offset = (uint32_t)agg.name_pool.size();
                if (!name_has_escapes) {
                    agg.name_pool.insert(agg.name_pool.end(), name_unescaped_start, name_unescaped_start + name_bytes);
                } else {
                    const char* cur = name_unescaped_start;
                    for (uint32_t b = 0; b < name_bytes; ) {
                        if (*cur == '"' && *(cur + 1) == '"') {
                            agg.name_pool.push_back('"');
                            cur += 2;
                            b++;
                        } else {
                            agg.name_pool.push_back(*cur++);
                            b++;
                        }
                    }
                }
                uint64_t nhash = fnv1a_64(agg.name_pool.data() + pool_offset, name_bytes);
                agg.name_set.insert(nhash, pool_offset, (uint16_t)name_bytes, agg.name_pool.data());
            }
        });
    }

    for (auto& w : workers) w.join();
    workers.clear();

    munmap((void*)rec_data, rec_size);

    // Merge aggregations across all threads into AGGS[256]
    struct FinalAgg {
        uint64_t count = 0;
        int64_t id_sum = 0;
        int64_t amount_sum = 0;
        int64_t amount_min = 0;
        int64_t amount_max = 0;
        uint64_t name_bytes = 0;
        uint64_t note_bytes = 0;
        bool has_record = false;
        NameHashSet name_set;
        std::vector<char> name_pool;
    };
    std::vector<FinalAgg> final_aggs(256);
    for (int c = 0; c < 256; ++c) {
        final_aggs[c].name_set.init(1024);
    }

    for (uint32_t c = 0; c < 256; ++c) {
        for (unsigned int t = 0; t < num_threads; ++t) {
            const auto& src = thread_aggs[t][c];
            if (!src.has_record) continue;
            auto& dst = final_aggs[c];
            if (!dst.has_record) {
                dst.has_record = true;
                dst.count = src.count;
                dst.id_sum = src.id_sum;
                dst.amount_sum = src.amount_sum;
                dst.amount_min = src.amount_min;
                dst.amount_max = src.amount_max;
                dst.name_bytes = src.name_bytes;
                dst.note_bytes = src.note_bytes;
            } else {
                dst.count += src.count;
                dst.id_sum += src.id_sum;
                dst.amount_sum += src.amount_sum;
                if (src.amount_min < dst.amount_min) dst.amount_min = src.amount_min;
                if (src.amount_max > dst.amount_max) dst.amount_max = src.amount_max;
                dst.name_bytes += src.name_bytes;
                dst.note_bytes += src.note_bytes;
            }

            // Merge names
            for (const auto& slot : src.name_set.slots) {
                if (slot.hash != 0) {
                    uint32_t new_off = (uint32_t)dst.name_pool.size();
                    dst.name_pool.insert(dst.name_pool.end(),
                                         src.name_pool.data() + slot.offset,
                                         src.name_pool.data() + slot.offset + slot.len);
                    dst.name_set.insert(slot.hash, new_off, slot.len, dst.name_pool.data());
                }
            }
        }
    }

    // 3. Format output
    std::vector<std::string> active_keys;
    for (uint32_t i = 0; i < NUM_COUNTRIES; ++i) {
        if (final_aggs[i].has_record) {
            active_keys.push_back(std::string(COUNTRIES[i].code, 3));
        }
    }
    std::sort(active_keys.begin(), active_keys.end());
    if (final_aggs[255].has_record) {
        active_keys.push_back("_UNMATCHED");
    }

    mkdir(out_dir.c_str(), 0777);
    std::string out_path = out_dir + "/aggregates.txt";
    int out_fd = open(out_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (out_fd < 0) return 1;

    std::string out_buf;
    out_buf.reserve(64 * 1024);
    char line_buf[512];

    for (const auto& key : active_keys) {
        uint8_t cid = 255;
        if (key != "_UNMATCHED") {
            for (uint32_t i = 0; i < NUM_COUNTRIES; ++i) {
                if (COUNTRIES[i].code[0] == key[0] &&
                    COUNTRIES[i].code[1] == key[1] &&
                    COUNTRIES[i].code[2] == key[2]) {
                    cid = (uint8_t)i;
                    break;
                }
            }
        }
        const auto& agg = final_aggs[cid];

        char* p = line_buf;
        std::memcpy(p, key.data(), key.size());
        p += key.size();
        *p++ = ' ';

        p = u64_to_str(p, agg.count);
        *p++ = ' ';

        p = u64_to_str(p, (uint64_t)agg.id_sum);
        *p++ = ' ';

        p = format_cents(p, agg.amount_sum);
        *p++ = ' ';

        p = format_cents(p, agg.amount_min);
        *p++ = ' ';

        p = format_cents(p, agg.amount_max);
        *p++ = ' ';

        p = u64_to_str(p, agg.name_set.count);
        *p++ = ' ';

        p = u64_to_str(p, agg.name_bytes);
        *p++ = ' ';

        p = u64_to_str(p, agg.note_bytes);
        *p++ = '\n';

        out_buf.append(line_buf, p - line_buf);
    }

    size_t written = 0;
    while (written < out_buf.size()) {
        ssize_t ret = write(out_fd, out_buf.data() + written, out_buf.size() - written);
        if (ret <= 0) break;
        written += ret;
    }
    close(out_fd);

    return 0;
}
