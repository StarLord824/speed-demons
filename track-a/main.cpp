#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstring>
#include <cstdint>
#include <chrono>
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

inline uint32_t u32_len(uint32_t v) {
    if (v < 10) return 1;
    if (v < 100) return 2;
    if (v < 1000) return 3;
    if (v < 10000) return 4;
    if (v < 100000) return 5;
    if (v < 1000000) return 6;
    if (v < 10000000) return 7;
    if (v < 100000000) return 8;
    if (v < 1000000000) return 9;
    return 10;
}

inline uint8_t decode_hour(const char* ts) {
    uint32_t d = (ts[8] * 10 + ts[9]) - 528;
    uint32_t h = (ts[11] * 10 + ts[12]) - 528;
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

constexpr size_t MAX_ENDPOINTS = 4096;
constexpr size_t HASH_CAPACITY = 8192;
constexpr size_t HASH_MASK = HASH_CAPACITY - 1;

struct alignas(16) TableEntry {
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
            if (entry_h == h) return slots[slot].intern_id;
            if (entry_h == 0) {
                static std::mutex insert_mutex;
                std::lock_guard<std::mutex> lock(insert_mutex);
                if (slots[slot].hash == h) return slots[slot].intern_id;
                if (slots[slot].hash == 0) {
                    uint32_t new_id = count.load(std::memory_order_relaxed);
                    uint32_t off = g_ep_storage_offset.load(std::memory_order_relaxed);
                    std::memcpy(g_ep_storage + off, s, len);
                    g_ep_storage_offset.store(off + len, std::memory_order_relaxed);

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
};

struct alignas(8) Item {
    uint32_t gid;
    uint32_t lat;
};

constexpr size_t TOTAL_GROUPS_CAP = 72 * 2048;

struct GroupSummary {
    uint32_t p50, p95, p99;
};

static uint32_t old_to_new_gid[TOTAL_GROUPS_CAP];
static GlobalEpTable ep_table;
static std::atomic<uint32_t> counts[TOTAL_GROUPS_CAP];
static std::atomic<uint16_t> errors[TOTAL_GROUPS_CAP];
static std::atomic<uint32_t> write_pos[TOTAL_GROUPS_CAP];
static uint32_t group_offsets[TOTAL_GROUPS_CAP + 1];
static uint16_t total_errs[TOTAL_GROUPS_CAP];
static uint32_t active_groups[TOTAL_GROUPS_CAP];
static GroupSummary summaries[TOTAL_GROUPS_CAP];

struct ThreadItems {
    Item* items{nullptr};
    size_t size{0};
};

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
    if (num_threads > std::thread::hardware_concurrency() && std::thread::hardware_concurrency() > 0) {
        num_threads = std::thread::hardware_concurrency();
    }

    std::vector<size_t> chunk_starts(num_threads + 1, 0);
    for (unsigned int i = 1; i < num_threads; ++i) {
        size_t approx = (file_size / num_threads) * i;
        while (approx < file_size && data[approx] != '\n') approx++;
        if (approx < file_size) approx++;
        chunk_starts[i] = approx;
    }
    chunk_starts[num_threads] = file_size;

    std::memset(ep_table.slots, 0, sizeof(ep_table.slots));
    for (size_t i = 0; i < TOTAL_GROUPS_CAP; ++i) {
        counts[i].store(0, std::memory_order_relaxed);
        errors[i].store(0, std::memory_order_relaxed);
    }

    std::vector<ThreadItems> thread_items(num_threads);
    std::vector<size_t> thread_buf_bytes(num_threads);
    for (unsigned int t = 0; t < num_threads; ++t) {
        size_t chunk_bytes = chunk_starts[t + 1] - chunk_starts[t];
        size_t max_items = (chunk_bytes / 68) + 2048;
        thread_buf_bytes[t] = max_items * sizeof(Item);
        thread_items[t].items = (Item*)mmap(nullptr, thread_buf_bytes[t], PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }

    std::vector<std::thread> workers;

    // PHASE 1: Single Pass parse over requests.log
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            Item* items = thread_items[t].items;
            size_t n = 0;

            const char* ptr = data + chunk_starts[t];
            const char* end = data + chunk_starts[t + 1];

            while (ptr < end) {
                uint8_t hr = decode_hour(ptr);

                const char* ep_start;
                if (ptr[31] == ' ') ep_start = ptr + 32;
                else if (ptr[32] == ' ') ep_start = ptr + 33;
                else if (ptr[35] == ' ') ep_start = ptr + 36;
                else if (ptr[33] == ' ') ep_start = ptr + 34;
                else if (ptr[34] == ' ') ep_start = ptr + 35;
                else {
                    ptr += 28;
                    while (*ptr != ' ') ptr++;
                    ptr++;
                    ep_start = ptr;
                }

                const char* ep_end;
                uint64_t h = swar_hash_and_find_end(ep_start, ep_end);
                uint32_t intern_id = ep_table.get_or_insert(ep_start, ep_end - ep_start, h);

                ptr = ep_end + 1;
                bool is_err = (ptr[0] == '5');

                uint32_t gid = (uint32_t)hr * 2048 + intern_id;
                counts[gid].fetch_add(1, std::memory_order_relaxed);
                if (is_err) {
                    errors[gid].fetch_add(1, std::memory_order_relaxed);
                }

                ptr = ep_end + 5;
                uint32_t lat = *ptr++ - '0';
                while (*ptr != '\n') {
                    lat = lat * 10 + (*ptr++ - '0');
                }
                ptr++;

                items[n++] = {gid, lat};
            }
            thread_items[t].size = n;
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // Drop the entire 238MB file from cache immediately!
    munmap((void*)data, file_size);

    // PHASE 2: Sort endpoints & compute prefix sums
    uint32_t num_eps = ep_table.count.load(std::memory_order_relaxed);

    std::vector<uint32_t> sorted_intern_ids(num_eps);
    for (uint32_t i = 0; i < num_eps; ++i) sorted_intern_ids[i] = i;
    std::sort(sorted_intern_ids.begin(), sorted_intern_ids.end(), [&](uint32_t a, uint32_t b) {
        auto ea = ep_table.ep_strings[a];
        auto eb = ep_table.ep_strings[b];
        const char* pa = g_ep_storage + ea.offset;
        const char* pb = g_ep_storage + eb.offset;
        size_t min_len = std::min(ea.len, eb.len);
        int cmp = std::memcmp(pa, pb, min_len);
        if (cmp != 0) return cmp < 0;
        return ea.len < eb.len;
    });

    uint32_t num_groups = 72 * num_eps;
    size_t total_items = 0;
    size_t num_active = 0;

    for (uint32_t hr = 0; hr < 72; ++hr) {
        for (uint32_t s = 0; s < num_eps; ++s) {
            uint32_t intern_id = sorted_intern_ids[s];
            uint32_t old_gid = hr * 2048 + intern_id;
            uint32_t new_gid = hr * num_eps + s;
            old_to_new_gid[old_gid] = new_gid;
            uint32_t cnt = counts[old_gid].load(std::memory_order_relaxed);
            total_errs[new_gid] = errors[old_gid].load(std::memory_order_relaxed);

            group_offsets[new_gid] = (uint32_t)total_items;
            total_items += cnt;
            write_pos[new_gid].store(group_offsets[new_gid], std::memory_order_relaxed);

            if (cnt > 0) {
                active_groups[num_active++] = new_gid;
            }
        }
    }
    group_offsets[num_groups] = (uint32_t)total_items;

    // Allocate flat latencies array
    size_t lat_bytes = total_items * sizeof(uint32_t);
    uint32_t* all_latencies = (uint32_t*)mmap(nullptr, lat_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // PHASE 3: Fast scatter from L2 cache buffers directly into all_latencies
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            const Item* items = thread_items[t].items;
            size_t n = thread_items[t].size;
            for (size_t i = 0; i < n; ++i) {
                uint32_t new_gid = old_to_new_gid[items[i].gid];
                uint32_t pos = write_pos[new_gid].fetch_add(1, std::memory_order_relaxed);
                all_latencies[pos] = items[i].lat;
            }
            // Free thread buffer immediately!
            munmap(thread_items[t].items, thread_buf_bytes[t]);
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // PHASE 4: Percentiles via specialized small-sort & nth_element
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            size_t start_idx = (num_active * t) / num_threads;
            size_t end_idx = (num_active * (t + 1)) / num_threads;

            for (size_t i = start_idx; i < end_idx; ++i) {
                uint32_t gid = active_groups[i];
                uint32_t count = group_offsets[gid + 1] - group_offsets[gid];
                uint32_t start_pos = group_offsets[gid];

                uint32_t* slice = all_latencies + start_pos;
                size_t idx50 = (50ULL * count + 99) / 100 - 1;
                size_t idx95 = (95ULL * count + 99) / 100 - 1;
                size_t idx99 = (99ULL * count + 99) / 100 - 1;

                uint32_t p50, p95, p99;
                if (count == 1) {
                    p50 = p95 = p99 = slice[0];
                } else if (count == 2) {
                    uint32_t a = slice[0], b = slice[1];
                    if (a > b) std::swap(a, b);
                    p50 = a; p95 = b; p99 = b;
                } else if (count == 3) {
                    uint32_t a = slice[0], b = slice[1], c = slice[2];
                    if (a > b) std::swap(a, b);
                    if (b > c) std::swap(b, c);
                    if (a > b) std::swap(a, b);
                    p50 = b; p95 = c; p99 = c;
                } else if (count <= 16) {
                    std::sort(slice, slice + count);
                    p50 = slice[idx50];
                    p95 = slice[idx95];
                    p99 = slice[idx99];
                } else {
                    std::nth_element(slice, slice + idx50, slice + count);
                    p50 = slice[idx50];

                    std::nth_element(slice + idx50, slice + idx95, slice + count);
                    p95 = slice[idx95];

                    std::nth_element(slice + idx95, slice + idx99, slice + count);
                    p99 = slice[idx99];
                }

                summaries[i] = {p50, p95, p99};
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // Munmap all_latencies immediately!
    munmap(all_latencies, lat_bytes);

    // PHASE 5: Parallel output formatting via compact 64 KB stack buffers and pwrite
    std::vector<size_t> thread_bytes(num_threads, 0);
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            size_t start_idx = (num_active * t) / num_threads;
            size_t end_idx = (num_active * (t + 1)) / num_threads;
            size_t bytes = 0;
            for (size_t i = start_idx; i < end_idx; ++i) {
                const auto& sum = summaries[i];
                uint32_t gid = active_groups[i];
                uint32_t epid = gid % num_eps;
                uint32_t count = group_offsets[gid + 1] - group_offsets[gid];
                uint32_t errors_val = total_errs[gid];
                uint64_t ppm = ((uint64_t)errors_val * 1000000ULL) / count;

                uint32_t intern_id = sorted_intern_ids[epid];
                auto ep_str = ep_table.ep_strings[intern_id];

                bytes += 21 + ep_str.len + 1 + u32_len(count) + 1 + u32_len(sum.p50) + 1 + u32_len(sum.p95) + 1 + u32_len(sum.p99) + 1 + u32_len(errors_val) + 1 + u32_len((uint32_t)ppm) + 1;
            }
            thread_bytes[t] = bytes;
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    mkdir(out_dir.c_str(), 0777);
    std::string out_path = out_dir + "/summary.txt";
    int out_fd = open(out_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (out_fd >= 0) {
        std::vector<size_t> thread_offsets(num_threads + 1, 0);
        for (unsigned int t = 0; t < num_threads; ++t) {
            thread_offsets[t + 1] = thread_offsets[t] + thread_bytes[t];
        }
        ftruncate(out_fd, thread_offsets[num_threads]);

        for (unsigned int t = 0; t < num_threads; ++t) {
            workers.emplace_back([&, t]() {
                size_t start_idx = (num_active * t) / num_threads;
                size_t end_idx = (num_active * (t + 1)) / num_threads;

                char buf[65536];
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
        }
        for (auto& w : workers) w.join();
        workers.clear();
        close(out_fd);
    }

    return 0;
}
