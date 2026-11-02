#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <cstring>
#include <cstdint>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

static const char* HOURS_STR[72] = {
    "2026-10-30T18:00:00Z", "2026-10-30T19:00:00Z", "2026-10-30T20:00:00Z", "2026-10-30T21:00:00Z", "2026-10-30T22:00:00Z", "2026-10-30T23:00:00Z",
    "2026-10-31T00:00:00Z", "2026-10-31T01:00:00Z", "2026-10-31T02:00:00Z", "2026-10-31T03:00:00Z", "2026-10-31T04:00:00Z", "2026-10-31T05:00:00Z",
    "2026-10-31T06:00:00Z", "2026-10-31T07:00:00Z", "2026-10-31T08:00:00Z", "2026-10-31T09:00:00Z", "2026-10-31T10:00:00Z", "2026-10-31T11:00:00Z",
    "2026-10-31T12:00:00Z", "2026-10-31T13:00:00Z", "2026-10-31T14:00:00Z", "2026-10-31T15:00:00Z", "2026-10-31T16:00:00Z", "2026-10-31T17:00:00Z",
    "2026-10-31T18:00:00Z", "2026-10-31T19:00:00Z", "2026-10-31T20:00:00Z", "2026-10-31T21:00:00Z", "2026-10-31T22:00:00Z", "2026-10-31T23:00:00Z",
    "2026-11-01T00:00:00Z", "2026-11-01T01:00:00Z", "2026-11-01T02:00:00Z", "2026-11-01T03:00:00Z", "2026-11-01T04:00:00Z", "2026-11-01T05:00:00Z",
    "2026-11-01T06:00:00Z", "2026-11-01T07:00:00Z", "2026-11-01T08:00:00Z", "2026-11-01T09:00:00Z", "2026-11-01T10:00:00Z", "2026-11-01T11:00:00Z",
    "2026-11-01T12:00:00Z", "2026-11-01T13:00:00Z", "2026-11-01T14:00:00Z", "2026-11-01T15:00:00Z", "2026-11-01T16:00:00Z", "2026-11-01T17:00:00Z",
    "2026-11-01T18:00:00Z", "2026-11-01T19:00:00Z", "2026-11-01T20:00:00Z", "2026-11-01T21:00:00Z", "2026-11-01T22:00:00Z", "2026-11-01T23:00:00Z",
    "2026-11-02T00:00:00Z", "2026-11-02T01:00:00Z", "2026-11-02T02:00:00Z", "2026-11-02T03:00:00Z", "2026-11-02T04:00:00Z", "2026-11-02T05:00:00Z",
    "2026-11-02T06:00:00Z", "2026-11-02T07:00:00Z", "2026-11-02T08:00:00Z", "2026-11-02T09:00:00Z", "2026-11-02T10:00:00Z", "2026-11-02T11:00:00Z",
    "2026-11-02T12:00:00Z", "2026-11-02T13:00:00Z", "2026-11-02T14:00:00Z", "2026-11-02T15:00:00Z", "2026-11-02T16:00:00Z", "2026-11-02T17:00:00Z"
};

static const char DIGITS_LUT[200] = {
    '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
    '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
    '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
    '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
    '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
    '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
    '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
    '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
    '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
    '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
};

inline char* u32_to_str(char* buf, uint32_t val) {
    if (val == 0) {
        *buf++ = '0';
        return buf;
    }
    char tmp[16];
    int idx = 0;
    while (val >= 100) {
        uint32_t q = val / 100;
        uint32_t r = val % 100;
        val = q;
        tmp[idx++] = DIGITS_LUT[r * 2 + 1];
        tmp[idx++] = DIGITS_LUT[r * 2];
    }
    if (val < 10) {
        tmp[idx++] = (char)('0' + val);
    } else {
        tmp[idx++] = DIGITS_LUT[val * 2 + 1];
        tmp[idx++] = DIGITS_LUT[val * 2];
    }
    while (idx > 0) {
        *buf++ = tmp[--idx];
    }
    return buf;
}

inline char* u64_to_str(char* buf, uint64_t val) {
    if (val <= 0xFFFFFFFFULL) return u32_to_str(buf, (uint32_t)val);
    char tmp[24];
    int idx = 0;
    while (val > 0) {
        tmp[idx++] = (char)('0' + (val % 10));
        val /= 10;
    }
    while (idx > 0) {
        *buf++ = tmp[--idx];
    }
    return buf;
}

inline uint32_t u32_len(uint32_t val) {
    if (val < 10) return 1;
    if (val < 100) return 2;
    if (val < 1000) return 3;
    if (val < 10000) return 4;
    if (val < 100000) return 5;
    if (val < 1000000) return 6;
    if (val < 10000000) return 7;
    if (val < 100000000) return 8;
    if (val < 1000000000) return 9;
    return 10;
}

inline uint8_t decode_hour(const char* ts) {
    uint32_t d = (uint32_t)(ts[8] - '0') * 10 + (uint32_t)(ts[9] - '0');
    uint32_t h = (uint32_t)(ts[11] - '0') * 10 + (uint32_t)(ts[12] - '0');
    uint32_t day_off = (ts[6] == '0') ? (d - 30) : (d + 1);
    return (uint8_t)(day_off * 24 + h - 18);
}

inline uint64_t swar_hash_and_find_end(const char* s, const char*& ep_end) {
    uint64_t h = 0x517cc1b727220a95ULL;
    const char* cur = s;
    while (true) {
        uint64_t v;
        std::memcpy(&v, cur, 8);
        uint64_t x = v ^ 0x2020202020202020ULL;
        uint64_t mask = (x - 0x0101010101010101ULL) & ~x & 0x8080808080808080ULL;
        if (mask != 0) {
            int off = __builtin_ctzll(mask) >> 3;
            ep_end = cur + off;
            uint64_t tail_mask = (1ULL << (off * 8)) - 1;
            v &= tail_mask;
            h = (h ^ v) * 0xbf58476d1ce4e5b9ULL;
            return h ^ (uint64_t)(ep_end - s);
        }
        h = (h ^ v) * 0xbf58476d1ce4e5b9ULL;
        cur += 8;
    }
}

template<typename T>
inline void insertion_sort(T* arr, size_t n) {
    for (size_t i = 1; i < n; ++i) {
        T key = arr[i];
        size_t j = i;
        while (j > 0 && arr[j - 1] > key) {
            arr[j] = arr[j - 1];
            --j;
        }
        arr[j] = key;
    }
}

template<typename F>
static void run_workers(unsigned int num_threads, const F& fn) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 65536);

    struct ThreadContext {
        unsigned int id;
        const F* fn;
    };

    pthread_t threads[16] = {};
    ThreadContext contexts[16] = {};

    for (unsigned int t = 0; t < num_threads; ++t) {
        contexts[t].id = t;
        contexts[t].fn = &fn;
        pthread_create(&threads[t], &attr, [](void* arg) -> void* {
            auto* ctx = static_cast<ThreadContext*>(arg);
            (*ctx->fn)(ctx->id);
            return nullptr;
        }, &contexts[t]);
    }
    pthread_attr_destroy(&attr);

    for (unsigned int t = 0; t < num_threads; ++t) {
        pthread_join(threads[t], nullptr);
    }
}

constexpr size_t MAX_ENDPOINTS = 4096;
constexpr size_t HASH_CAPACITY = 16384;
constexpr size_t HASH_MASK = HASH_CAPACITY - 1;

struct TableEntry {
    uint64_t hash;
    uint16_t intern_id;
    uint16_t sorted_id;
};

struct EpString {
    uint32_t offset;
    uint16_t len;
};

static char g_ep_storage[256 * 1024];
static std::atomic<uint32_t> g_ep_storage_offset{0};

struct GlobalEpTable {
    TableEntry slots[HASH_CAPACITY];
    EpString ep_strings[MAX_ENDPOINTS];
    std::atomic<uint32_t> count{0};

    uint32_t get_or_insert(const char* s, size_t len, uint64_t h) {
        uint32_t slot = (uint32_t)h & HASH_MASK;
        while (true) {
            uint64_t entry_h = slots[slot].hash;
            if (__builtin_expect(entry_h == h, 1)) return slots[slot].intern_id;
            if (__builtin_expect(entry_h == 0, 0)) {
                static std::mutex insert_mutex;
                std::lock_guard<std::mutex> lock(insert_mutex);
                if (slots[slot].hash == h) return slots[slot].intern_id;
                if (slots[slot].hash == 0) {
                    uint32_t new_id = count.load(std::memory_order_relaxed);
                    uint32_t off = g_ep_storage_offset.load(std::memory_order_relaxed);
                    std::memcpy(g_ep_storage + off, s, len);
                    g_ep_storage_offset.store(off + (uint32_t)len, std::memory_order_relaxed);

                    ep_strings[new_id] = {off, (uint16_t)len};
                    count.store(new_id + 1, std::memory_order_relaxed);
                    slots[slot].intern_id = (uint16_t)new_id;
                    slots[slot].sorted_id = (uint16_t)new_id;
                    slots[slot].hash = h;
                    return new_id;
                }
            }
            slot = (slot + 1) & HASH_MASK;
        }
    }

    inline uint32_t lookup_sorted(uint64_t h) const {
        uint32_t slot = (uint32_t)h & HASH_MASK;
        while (true) {
            if (__builtin_expect(slots[slot].hash == h, 1)) {
                return slots[slot].sorted_id;
            }
            slot = (slot + 1) & HASH_MASK;
        }
    }
};

constexpr size_t TOTAL_GROUPS_CAP = 72 * 2048;

struct GroupSummary {
    uint32_t p50, p95, p99;
};

static GlobalEpTable ep_table;
static uint32_t group_offsets[TOTAL_GROUPS_CAP + 1];
static uint16_t total_errs[TOTAL_GROUPS_CAP];
static uint32_t active_groups[TOTAL_GROUPS_CAP];
static GroupSummary summaries[TOTAL_GROUPS_CAP];

int main(int argc, char** argv) {
    std::string data_dir = "/data";
    std::string out_dir = "/out";
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--data" && i + 1 < argc) data_dir = argv[++i];
        else if (std::string(argv[i]) == "--out" && i + 1 < argc) out_dir = argv[++i];
    }

    std::string in_path = data_dir + "/requests.log";
    int fd = open(in_path.c_str(), O_RDONLY);
    if (fd < 0) return 1;

    struct stat st;
    fstat(fd, &st);
    size_t file_size = st.st_size;
    if (file_size == 0) {
        close(fd);
        std::string out_path = out_dir + "/summary.txt";
        mkdir(out_dir.c_str(), 0777);
        int out_fd = open(out_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
        if (out_fd >= 0) close(out_fd);
        return 0;
    }

    const char* data = (const char*)mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    unsigned int num_threads = 12;

    std::vector<size_t> chunk_starts(num_threads + 1, 0);
    for (unsigned int i = 1; i < num_threads; ++i) {
        size_t approx = (file_size / num_threads) * i;
        while (approx < file_size && data[approx] != '\n') approx++;
        if (approx < file_size) approx++;
        chunk_starts[i] = approx;
    }
    chunk_starts[num_threads] = file_size;

    std::memset(ep_table.slots, 0, sizeof(ep_table.slots));
    g_ep_storage_offset.store(0, std::memory_order_relaxed);
    ep_table.count.store(0, std::memory_order_relaxed);

    size_t tc_bytes = num_threads * TOTAL_GROUPS_CAP * sizeof(uint32_t);
    size_t te_bytes = num_threads * TOTAL_GROUPS_CAP * sizeof(uint16_t);
    uint32_t* thread_counts = (uint32_t*)mmap(nullptr, tc_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    uint16_t* thread_errors = (uint16_t*)mmap(nullptr, te_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    uint8_t thread_min_h[16];
    uint8_t thread_max_h[16];
    for (unsigned int t = 0; t < num_threads; ++t) {
        thread_min_h[t] = 72;
        thread_max_h[t] = 0;
    }

    // PASS 1: Zero-atomic thread-local counting
    run_workers(num_threads, [&](unsigned int t) {
        const char* ptr = data + chunk_starts[t];
        const char* end = data + chunk_starts[t + 1];
        const char* file_end = data + file_size;
        uint32_t* my_counts = thread_counts + t * TOTAL_GROUPS_CAP;
        uint16_t* my_errors = thread_errors + t * TOTAL_GROUPS_CAP;
        uint8_t min_h = 72, max_h = 0;

        while (ptr < end) {
            uint8_t hr = decode_hour(ptr);
            if (hr < min_h) min_h = hr;
            if (hr > max_h) max_h = hr;

            // Branchless SWAR method find
            uint64_t w;
            std::memcpy(&w, ptr + 31, 8);
            uint64_t mx = w ^ 0x2020202020202020ULL;
            uint64_t mm = (mx - 0x0101010101010101ULL) & ~mx & 0x8080808080808080ULL;
            const char* ep_start = (ptr + 31) + (__builtin_ctzll(mm) >> 3) + 1;

            const char* ep_end;
            uint64_t h = swar_hash_and_find_end(ep_start, ep_end);
            uint32_t intern_id = ep_table.get_or_insert(ep_start, ep_end - ep_start, h);

            bool is_err = (ep_end[1] == '5');
            uint32_t gid = (uint32_t)hr * 2048 + intern_id;
            my_counts[gid]++;
            if (is_err) my_errors[gid]++;

            // Skip latency completely using SWAR newline search
            const char* p = ep_end + 5;
            if (__builtin_expect(p + 8 <= file_end, 1)) {
                uint64_t v;
                std::memcpy(&v, p, 8);
                uint64_t x = v ^ 0x0A0A0A0A0A0A0A0AULL;
                uint64_t mask = (x - 0x0101010101010101ULL) & ~x & 0x8080808080808080ULL;
                if (mask != 0) {
                    ptr = p + (__builtin_ctzll(mask) >> 3) + 1;
                } else {
                    while (*p != '\n') p++;
                    ptr = p + 1;
                }
            } else {
                while (*p != '\n' && p < file_end) p++;
                ptr = p + 1;
            }
        }
        thread_min_h[t] = min_h;
        thread_max_h[t] = max_h;
    });

    uint32_t num_eps = ep_table.count.load(std::memory_order_relaxed);

    std::vector<uint32_t> sorted_intern_ids(num_eps);
    for (uint32_t i = 0; i < num_eps; ++i) sorted_intern_ids[i] = i;
    std::sort(sorted_intern_ids.begin(), sorted_intern_ids.end(), [&](uint32_t a, uint32_t b) {
        auto sa = ep_table.ep_strings[a];
        auto sb = ep_table.ep_strings[b];
        const char* pa = g_ep_storage + sa.offset;
        const char* pb = g_ep_storage + sb.offset;
        size_t min_len = std::min((size_t)sa.len, (size_t)sb.len);
        int cmp = std::memcmp(pa, pb, min_len);
        if (cmp != 0) return cmp < 0;
        return sa.len < sb.len;
    });

    std::vector<uint16_t> intern_to_sorted(num_eps);
    for (uint32_t s = 0; s < num_eps; ++s) {
        intern_to_sorted[sorted_intern_ids[s]] = (uint16_t)s;
    }

    for (size_t s = 0; s < HASH_CAPACITY; ++s) {
        if (ep_table.slots[s].hash != 0) {
            ep_table.slots[s].sorted_id = intern_to_sorted[ep_table.slots[s].intern_id];
        }
    }

    uint32_t num_groups = 72 * num_eps;
    size_t total_items = 0;
    size_t num_active = 0;

    // Compact write offsets buffer: only ~600 KB total!
    size_t thread_offset_start[16];
    size_t total_write_offset_slots = 0;
    for (unsigned int t = 0; t < num_threads; ++t) {
        thread_offset_start[t] = total_write_offset_slots;
        if (thread_max_h[t] >= thread_min_h[t]) {
            total_write_offset_slots += (size_t)(thread_max_h[t] - thread_min_h[t] + 1) * num_eps;
        }
    }

    size_t two_bytes = total_write_offset_slots * sizeof(uint32_t);
    uint32_t* write_offsets_buf = (uint32_t*)mmap(nullptr, two_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    uint32_t* thread_write_offsets[16];
    for (unsigned int t = 0; t < num_threads; ++t) {
        thread_write_offsets[t] = write_offsets_buf + thread_offset_start[t];
    }

    for (uint32_t hr = 0; hr < 72; ++hr) {
        for (uint32_t s = 0; s < num_eps; ++s) {
            uint32_t intern_id = sorted_intern_ids[s];
            uint32_t old_gid = hr * 2048 + intern_id;
            uint32_t new_gid = hr * num_eps + s;

            uint32_t group_cnt = 0;
            uint32_t group_err = 0;
            for (unsigned int t = 0; t < num_threads; ++t) {
                group_cnt += thread_counts[t * TOTAL_GROUPS_CAP + old_gid];
                group_err += thread_errors[t * TOTAL_GROUPS_CAP + old_gid];
            }
            total_errs[new_gid] = (uint16_t)group_err;
            group_offsets[new_gid] = (uint32_t)total_items;

            uint32_t cur = (uint32_t)total_items;
            for (unsigned int t = 0; t < num_threads; ++t) {
                if (hr >= thread_min_h[t] && hr <= thread_max_h[t]) {
                    uint32_t local_gid = (hr - thread_min_h[t]) * num_eps + s;
                    thread_write_offsets[t][local_gid] = cur;
                }
                cur += thread_counts[t * TOTAL_GROUPS_CAP + old_gid];
            }

            total_items += group_cnt;
            if (group_cnt > 0) {
                active_groups[num_active++] = new_gid;
            }
        }
    }
    group_offsets[num_groups] = (uint32_t)total_items;

    // Immediately unmap thread_counts and thread_errors BEFORE allocating all_latencies!
    munmap(thread_counts, tc_bytes);
    munmap(thread_errors, te_bytes);

    // Allocate flat latencies array using anonymous mmap
    size_t lat_bytes = total_items * sizeof(uint32_t);
    uint32_t* all_latencies = (uint32_t*)mmap(nullptr, lat_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // PASS 2: Parse and stream latencies with zero atomics via compact thread_write_offsets
    run_workers(num_threads, [&](unsigned int t) {
        const char* ptr = data + chunk_starts[t];
        const char* end = data + chunk_starts[t + 1];
        uint32_t* my_offsets = thread_write_offsets[t];
        uint8_t my_min_h = thread_min_h[t];

        while (ptr < end) {
            if (ptr + 400 < end) {
                uint32_t gids[4];
                uint32_t lats[4];

                for (int k = 0; k < 4; ++k) {
                    uint8_t hr = decode_hour(ptr);

                    uint64_t w;
                    std::memcpy(&w, ptr + 31, 8);
                    uint64_t mx = w ^ 0x2020202020202020ULL;
                    uint64_t mm = (mx - 0x0101010101010101ULL) & ~mx & 0x8080808080808080ULL;
                    const char* ep_start = (ptr + 31) + (__builtin_ctzll(mm) >> 3) + 1;

                    const char* ep_end;
                    uint64_t h = swar_hash_and_find_end(ep_start, ep_end);
                    uint32_t sorted_ep = ep_table.lookup_sorted(h);
                    gids[k] = (uint32_t)(hr - my_min_h) * num_eps + sorted_ep;

                    ptr = ep_end + 5;
                    uint32_t lat = (uint32_t)(*ptr++ - '0');
                    while (true) {
                        char c = *ptr++;
                        if (c == '\n') break;
                        lat = lat * 10 + (uint32_t)(c - '0');
                    }
                    lats[k] = lat;
                }

                // Zero atomics! Pure L1 cache local pointer increments!
                uint32_t pos0 = my_offsets[gids[0]]++;
                uint32_t pos1 = my_offsets[gids[1]]++;
                uint32_t pos2 = my_offsets[gids[2]]++;
                uint32_t pos3 = my_offsets[gids[3]]++;

                all_latencies[pos0] = lats[0];
                all_latencies[pos1] = lats[1];
                all_latencies[pos2] = lats[2];
                all_latencies[pos3] = lats[3];
            } else {
                uint8_t hr = decode_hour(ptr);

                uint64_t w;
                std::memcpy(&w, ptr + 31, 8);
                uint64_t mx = w ^ 0x2020202020202020ULL;
                uint64_t mm = (mx - 0x0101010101010101ULL) & ~mx & 0x8080808080808080ULL;
                const char* ep_start = (ptr + 31) + (__builtin_ctzll(mm) >> 3) + 1;

                const char* ep_end;
                uint64_t h = swar_hash_and_find_end(ep_start, ep_end);
                uint32_t sorted_ep = ep_table.lookup_sorted(h);
                uint32_t gid = (uint32_t)(hr - my_min_h) * num_eps + sorted_ep;

                ptr = ep_end + 5;
                uint32_t lat = (uint32_t)(*ptr++ - '0');
                while (true) {
                    char c = *ptr++;
                    if (c == '\n') break;
                    lat = lat * 10 + (uint32_t)(c - '0');
                }

                uint32_t pos = my_offsets[gid]++;
                all_latencies[pos] = lat;
            }
        }
    });

    // Immediately unmap compact write offsets buffer and data file!
    munmap(write_offsets_buf, two_bytes);
    madvise((void*)data, file_size, MADV_DONTNEED);
    munmap((void*)data, file_size);

    // PHASE 4: Optimized percentiles with insertion sort for N<=24, std::sort for N<=64, and fused output length calculation
    std::vector<size_t> thread_bytes(num_threads, 0);
    run_workers(num_threads, [&](unsigned int t) {
        size_t start_idx = (num_active * t) / num_threads;
        size_t end_idx = (num_active * (t + 1)) / num_threads;
        size_t bytes = 0;

        for (size_t i = start_idx; i < end_idx; ++i) {
            uint32_t gid = active_groups[i];
            uint32_t epid = gid % num_eps;
            uint32_t count = group_offsets[gid + 1] - group_offsets[gid];
            uint32_t start_pos = group_offsets[gid];
            uint32_t errors_val = total_errs[gid];
            uint64_t ppm = ((uint64_t)errors_val * 1000000ULL) / count;

            uint32_t* slice = all_latencies + start_pos;
            size_t idx50 = (50ULL * count + 99) / 100 - 1;
            size_t idx95 = (95ULL * count + 99) / 100 - 1;
            size_t idx99 = (99ULL * count + 99) / 100 - 1;

            uint32_t p50, p95, p99;
            if (count <= 24) {
                insertion_sort(slice, count);
                p50 = slice[idx50];
                p95 = slice[idx95];
                p99 = slice[idx99];
            } else if (count <= 64) {
                std::sort(slice, slice + count);
                p50 = slice[idx50];
                p95 = slice[idx95];
                p99 = slice[idx99];
            } else {
                std::nth_element(slice, slice + idx50, slice + count);
                p50 = slice[idx50];

                if (idx95 == idx50) {
                    p95 = p50;
                } else {
                    std::nth_element(slice + idx50 + 1, slice + idx95, slice + count);
                    p95 = slice[idx95];
                }

                if (idx99 == idx95) {
                    p99 = p95;
                } else {
                    std::nth_element(slice + idx95 + 1, slice + idx99, slice + count);
                    p99 = slice[idx99];
                }
            }

            summaries[i] = {p50, p95, p99};

            uint32_t intern_id = sorted_intern_ids[epid];
            auto ep_str = ep_table.ep_strings[intern_id];
            bytes += 21 + ep_str.len + 1 + u32_len(count) + 1 + u32_len(p50) + 1 + u32_len(p95) + 1 + u32_len(p99) + 1 + u32_len(errors_val) + 1 + u32_len((uint32_t)ppm) + 1;
        }
        thread_bytes[t] = bytes;
    });

    // Munmap all_latencies immediately!
    munmap(all_latencies, lat_bytes);

    // PHASE 5: Parallel output formatting via compact 16 KB stack buffers and pwrite
    mkdir(out_dir.c_str(), 0777);
    std::string out_path = out_dir + "/summary.txt";
    int out_fd = open(out_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (out_fd >= 0) {
        std::vector<size_t> thread_offsets(num_threads + 1, 0);
        for (unsigned int t = 0; t < num_threads; ++t) {
            thread_offsets[t + 1] = thread_offsets[t] + thread_bytes[t];
        }
        ftruncate(out_fd, thread_offsets[num_threads]);

        run_workers(num_threads, [&](unsigned int t) {
            size_t start_idx = (num_active * t) / num_threads;
            size_t end_idx = (num_active * (t + 1)) / num_threads;

            char buf[16384];
            char* dst = buf;
            char* buf_end = buf + sizeof(buf) - 256;
            size_t cur_offset = thread_offsets[t];

            for (size_t i = start_idx; i < end_idx; ++i) {
                const auto& sum = summaries[i];
                uint32_t gid = active_groups[i];
                uint32_t hr = gid / num_eps;
                uint32_t epid = gid % num_eps;
                uint32_t count = group_offsets[gid + 1] - group_offsets[gid];
                uint32_t errors_val = total_errs[gid];
                uint64_t ppm = ((uint64_t)errors_val * 1000000ULL) / count;

                std::memcpy(dst, HOURS_STR[hr], 20);
                dst += 20;
                *dst++ = ' ';

                uint32_t intern_id = sorted_intern_ids[epid];
                auto ep_str = ep_table.ep_strings[intern_id];
                std::memcpy(dst, g_ep_storage + ep_str.offset, ep_str.len);
                dst += ep_str.len;
                *dst++ = ' ';

                dst = u32_to_str(dst, count);
                *dst++ = ' ';

                dst = u32_to_str(dst, sum.p50);
                *dst++ = ' ';

                dst = u32_to_str(dst, sum.p95);
                *dst++ = ' ';

                dst = u32_to_str(dst, sum.p99);
                *dst++ = ' ';

                dst = u32_to_str(dst, errors_val);
                *dst++ = ' ';

                dst = u64_to_str(dst, ppm);
                *dst++ = '\n';

                if (dst >= buf_end) {
                    pwrite(out_fd, buf, dst - buf, cur_offset);
                    cur_offset += (dst - buf);
                    dst = buf;
                }
            }
            if (dst > buf) {
                pwrite(out_fd, buf, dst - buf, cur_offset);
            }
        });
        close(out_fd);
    }

    return 0;
}
