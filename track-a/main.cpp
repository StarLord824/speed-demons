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

inline uint8_t decode_hour(const char* ts) {
    uint32_t m = (ts[5] - '0') * 10 + (ts[6] - '0');
    uint32_t d = (ts[8] - '0') * 10 + (ts[9] - '0');
    uint32_t h = (ts[11] - '0') * 10 + (ts[12] - '0');
    uint32_t day_off = (m == 10) ? (d - 30) : (d + 1);
    return (uint8_t)(day_off * 24 + h - 18);
}

inline uint64_t fast_hash(const char* s, size_t len) {
    uint64_t h = len * 0x517cc1b727220a95ULL;
    while (len >= 8) {
        uint64_t v;
        std::memcpy(&v, s, 8);
        h = (h ^ v) * 0xbf58476d1ce4e5b9ULL;
        s += 8;
        len -= 8;
    }
    if (len > 0) {
        uint64_t v = 0;
        std::memcpy(&v, s, len);
        h = (h ^ v) * 0xbf58476d1ce4e5b9ULL;
    }
    return h;
}

inline char* u64_to_str(char* buf, uint64_t val) {
    if (val == 0) {
        *buf++ = '0';
        return buf;
    }
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

#pragma pack(push, 1)
struct RawItem {
    uint8_t hr;
    uint16_t local_ep;
    uint32_t lat;
};
#pragma pack(pop)

struct ThreadData {
    std::vector<std::string_view> endpoints;
    uint16_t map_table[4096];
    std::vector<uint16_t> local_to_global;
    std::vector<RawItem> items;
    uint32_t local_errs[72 * 2048];
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

    std::vector<ThreadData> tdata(num_threads);
    std::vector<std::thread> workers;

    // SINGLE PASS PARSER: Parse all fields in 1 pass into fast local vector
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            ThreadData& td = tdata[t];
            td.endpoints.reserve(2048);
            std::memset(td.map_table, 0xFF, sizeof(td.map_table));
            std::memset(td.local_errs, 0, sizeof(td.local_errs));
            td.items.reserve(300000);

            const char* ptr = data + chunk_starts[t];
            const char* end = data + chunk_starts[t + 1];

            while (ptr < end) {
                uint8_t hr = decode_hour(ptr);
                ptr += 28; // ts (27) + space (1)

                // method
                while (*ptr != ' ') ptr++;
                ptr++; // skip space

                // endpoint
                const char* ep_start = ptr;
                while (*ptr != ' ') ptr++;
                size_t ep_len = ptr - ep_start;
                ptr++; // skip space

                uint64_t h = fast_hash(ep_start, ep_len);
                uint32_t slot = (uint32_t)h & 4095;
                uint16_t local_ep = 0xFFFF;
                while (true) {
                    uint16_t id = td.map_table[slot];
                    if (id == 0xFFFF) {
                        local_ep = (uint16_t)td.endpoints.size();
                        td.endpoints.emplace_back(ep_start, ep_len);
                        td.map_table[slot] = local_ep;
                        break;
                    }
                    std::string_view existing = td.endpoints[id];
                    if (existing.size() == ep_len && std::memcmp(existing.data(), ep_start, ep_len) == 0) {
                        local_ep = id;
                        break;
                    }
                    slot = (slot + 1) & 4095;
                }

                // status
                bool is_err = (ptr[0] == '5');
                ptr += 4; // status (3) + space (1)

                uint32_t lat = 0;
                while (*ptr >= '0' && *ptr <= '9') {
                    lat = lat * 10 + (*ptr++ - '0');
                }
                if (ptr < end && *ptr == '\n') ptr++;

                td.items.push_back({hr, local_ep, lat});
                if (is_err) {
                    td.local_errs[(uint32_t)hr * 2048 + local_ep]++;
                }
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // Consolidate global endpoints
    std::vector<std::string_view> global_eps;
    global_eps.reserve(2048);
    for (unsigned int t = 0; t < num_threads; ++t) {
        for (const auto& ep : tdata[t].endpoints) {
            bool found = false;
            for (const auto& gep : global_eps) {
                if (gep == ep) { found = true; break; }
            }
            if (!found) global_eps.push_back(ep);
        }
    }

    std::sort(global_eps.begin(), global_eps.end(), [](std::string_view a, std::string_view b) {
        size_t min_len = std::min(a.size(), b.size());
        int cmp = std::memcmp(a.data(), b.data(), min_len);
        if (cmp != 0) return cmp < 0;
        return a.size() < b.size();
    });

    uint32_t num_eps = (uint32_t)global_eps.size();
    uint32_t num_groups = 72 * num_eps;

    // Map local_to_global per thread
    for (unsigned int t = 0; t < num_threads; ++t) {
        ThreadData& td = tdata[t];
        td.local_to_global.resize(td.endpoints.size());
        for (size_t l = 0; l < td.endpoints.size(); ++l) {
            std::string_view lep = td.endpoints[l];
            auto it = std::lower_bound(global_eps.begin(), global_eps.end(), lep,
                [](std::string_view a, std::string_view b) {
                    size_t min_len = std::min(a.size(), b.size());
                    int cmp = std::memcmp(a.data(), b.data(), min_len);
                    if (cmp != 0) return cmp < 0;
                    return a.size() < b.size();
                });
            td.local_to_global[l] = (uint16_t)(it - global_eps.begin());
        }
    }

    // Remap local errors to global group errors and count group frequencies
    std::vector<uint32_t> total_counts(num_groups, 0);
    std::vector<uint32_t> total_errs(num_groups, 0);
    std::vector<std::vector<uint32_t>> thread_group_counts(num_threads, std::vector<uint32_t>(num_groups, 0));

    for (unsigned int t = 0; t < num_threads; ++t) {
        ThreadData& td = tdata[t];
        auto& tcounts = thread_group_counts[t];
        for (const auto& it : td.items) {
            uint16_t gep = td.local_to_global[it.local_ep];
            uint32_t gid = (uint32_t)it.hr * num_eps + gep;
            tcounts[gid]++;
        }
        for (uint32_t hr = 0; hr < 72; ++hr) {
            for (size_t loc_ep = 0; loc_ep < td.endpoints.size(); ++loc_ep) {
                uint32_t errs = td.local_errs[hr * 2048 + loc_ep];
                if (errs > 0) {
                    uint16_t gep = td.local_to_global[loc_ep];
                    total_errs[hr * num_eps + gep] += errs;
                }
            }
        }
    }

    for (uint32_t g = 0; g < num_groups; ++g) {
        for (unsigned int t = 0; t < num_threads; ++t) {
            total_counts[g] += thread_group_counts[t][g];
        }
    }

    // Compute prefix sum offsets
    std::vector<uint32_t> group_offsets(num_groups + 1, 0);
    size_t total_items = 0;
    for (uint32_t g = 0; g < num_groups; ++g) {
        group_offsets[g] = (uint32_t)total_items;
        total_items += total_counts[g];
    }
    group_offsets[num_groups] = (uint32_t)total_items;

    // Write offsets per thread
    std::vector<std::vector<uint32_t>> thread_write_offsets(num_threads, std::vector<uint32_t>(num_groups, 0));
    for (uint32_t g = 0; g < num_groups; ++g) {
        uint32_t running = group_offsets[g];
        for (unsigned int t = 0; t < num_threads; ++t) {
            thread_write_offsets[t][g] = running;
            running += thread_group_counts[t][g];
        }
    }
    std::vector<std::vector<uint32_t>>().swap(thread_group_counts);

    // Contiguous latencies flat array
    std::vector<uint32_t> all_latencies(total_items);

    // Scatter latencies from local items
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            ThreadData& td = tdata[t];
            auto& offsets = thread_write_offsets[t];
            for (const auto& it : td.items) {
                uint16_t gep = td.local_to_global[it.local_ep];
                uint32_t gid = (uint32_t)it.hr * num_eps + gep;
                uint32_t pos = offsets[gid]++;
                all_latencies[pos] = it.lat;
            }
            std::vector<RawItem>().swap(td.items); // free items immediately!
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    std::vector<std::vector<uint32_t>>().swap(thread_write_offsets);
    std::vector<ThreadData>().swap(tdata);

    // Collect active group IDs
    std::vector<uint32_t> active_groups;
    active_groups.reserve(num_groups);
    for (uint32_t g = 0; g < num_groups; ++g) {
        if (total_counts[g] > 0) active_groups.push_back(g);
    }
    size_t num_active = active_groups.size();

    struct GroupSummary {
        uint32_t gid;
        uint32_t p50, p95, p99;
    };
    std::vector<GroupSummary> summaries(num_active);

    // Parallel percentile selection via nth_element
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            size_t start_idx = (num_active * t) / num_threads;
            size_t end_idx = (num_active * (t + 1)) / num_threads;

            for (size_t i = start_idx; i < end_idx; ++i) {
                uint32_t gid = active_groups[i];
                uint32_t count = total_counts[gid];
                uint32_t start_pos = group_offsets[gid];

                uint32_t* slice = all_latencies.data() + start_pos;
                size_t idx50 = (50ULL * count + 99) / 100 - 1;
                size_t idx95 = (95ULL * count + 99) / 100 - 1;
                size_t idx99 = (99ULL * count + 99) / 100 - 1;

                std::nth_element(slice, slice + idx50, slice + count);
                uint32_t p50 = slice[idx50];

                std::nth_element(slice + idx50, slice + idx95, slice + count);
                uint32_t p95 = slice[idx95];

                std::nth_element(slice + idx95, slice + idx99, slice + count);
                uint32_t p99 = slice[idx99];

                summaries[i] = {gid, p50, p95, p99};
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // Format output strings in parallel
    std::vector<std::string> thread_output(num_threads);
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            size_t start_idx = (num_active * t) / num_threads;
            size_t end_idx = (num_active * (t + 1)) / num_threads;

            std::string& out = thread_output[t];
            out.reserve((end_idx - start_idx) * 90);
            char num_buf[32];

            for (size_t i = start_idx; i < end_idx; ++i) {
                const auto& sum = summaries[i];
                uint32_t gid = sum.gid;
                uint32_t hr = gid / num_eps;
                uint32_t epid = gid % num_eps;
                uint32_t count = total_counts[gid];
                uint32_t errors = total_errs[gid];
                uint64_t ppm = ((uint64_t)errors * 1000000ULL) / count;

                out.append(HOURS_STR[hr]);
                out.push_back(' ');

                std::string_view ep = global_eps[epid];
                out.append(ep.data(), ep.size());
                out.push_back(' ');

                char* p = u64_to_str(num_buf, count);
                out.append(num_buf, p - num_buf);
                out.push_back(' ');

                p = u64_to_str(num_buf, sum.p50);
                out.append(num_buf, p - num_buf);
                out.push_back(' ');

                p = u64_to_str(num_buf, sum.p95);
                out.append(num_buf, p - num_buf);
                out.push_back(' ');

                p = u64_to_str(num_buf, sum.p99);
                out.append(num_buf, p - num_buf);
                out.push_back(' ');

                p = u64_to_str(num_buf, errors);
                out.append(num_buf, p - num_buf);
                out.push_back(' ');

                p = u64_to_str(num_buf, ppm);
                out.append(num_buf, p - num_buf);
                out.push_back('\n');
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    mkdir(out_dir.c_str(), 0777);
    std::string out_path = out_dir + "/summary.txt";
    int out_fd = open(out_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (out_fd >= 0) {
        for (unsigned int t = 0; t < num_threads; ++t) {
            write(out_fd, thread_output[t].data(), thread_output[t].size());
        }
        close(out_fd);
    }
    munmap((void*)data, file_size);

    return 0;
}
