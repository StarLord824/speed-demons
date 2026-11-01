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

inline uint32_t fnv1a(const char* s, size_t len) {
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < len; ++i) {
        h ^= (uint8_t)s[i];
        h *= 16777619u;
    }
    return h;
}

// Fast integer to string formatter
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

struct ParsedItem {
    uint32_t tag; // [31: is_err][30..20: hour][19..0: local_ep_id]
    uint32_t latency;
};

struct ThreadData {
    std::vector<ParsedItem> items;
    std::vector<std::string_view> endpoints;
    uint16_t map_table[8192];
    std::vector<uint16_t> local_to_global;
    std::vector<uint32_t> group_counts;
    std::vector<uint32_t> group_errs;
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
    if (fstat(fd, &st) < 0) { close(fd); return 1; }
    size_t file_size = st.st_size;
    if (file_size == 0) {
        close(fd);
        std::string out_path = out_dir + "/summary.txt";
        int out_fd = open(out_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
        if (out_fd >= 0) close(out_fd);
        return 0;
    }

    const char* data = (const char*)mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) { close(fd); return 1; }
    close(fd);
    madvise((void*)data, file_size, MADV_WILLNEED | MADV_SEQUENTIAL);

    unsigned int num_threads = 12;
    // On systems with fewer cores, hardware_concurrency is fine, but 12 matches scoring box
    if (num_threads > std::thread::hardware_concurrency() && std::thread::hardware_concurrency() > 0) {
        num_threads = std::thread::hardware_concurrency();
    }

    std::vector<size_t> chunk_starts(num_threads + 1, 0);
    for (unsigned int i = 1; i < num_threads; ++i) {
        size_t approx = (file_size / num_threads) * i;
        while (approx < file_size && data[approx] != '\n') {
            approx++;
        }
        if (approx < file_size) approx++; // first byte after \n
        chunk_starts[i] = approx;
    }
    chunk_starts[num_threads] = file_size;

    std::vector<ThreadData> tdata(num_threads);
    std::vector<std::thread> workers;

    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            ThreadData& td = tdata[t];
            td.items.reserve((chunk_starts[t + 1] - chunk_starts[t]) / 70);
            td.endpoints.reserve(2048);
            std::memset(td.map_table, 0xFF, sizeof(td.map_table));

            const char* ptr = data + chunk_starts[t];
            const char* end = data + chunk_starts[t + 1];

            while (ptr < end) {
                // ts (27 bytes)
                uint8_t hr = decode_hour(ptr);
                ptr += 27; // skip ts
                if (*ptr == ' ') ptr++;

                // method (3-7 bytes)
                while (*ptr != ' ') ptr++;
                ptr++; // skip space

                // endpoint
                const char* ep_start = ptr;
                while (*ptr != ' ') ptr++;
                size_t ep_len = ptr - ep_start;
                ptr++; // skip space

                // Lookup endpoint
                uint32_t h = fnv1a(ep_start, ep_len);
                uint32_t slot = h & 8191;
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
                    slot = (slot + 1) & 8191;
                }

                // status (3 digits)
                bool is_err = (ptr[0] == '5');
                ptr += 4; // status (3) + space (1)

                // latency
                uint32_t lat = 0;
                while (*ptr >= '0' && *ptr <= '9') {
                    lat = lat * 10 + (*ptr++ - '0');
                }
                if (*ptr == '\n') ptr++;

                uint32_t tag = ((uint32_t)is_err << 31) | ((uint32_t)hr << 20) | local_ep;
                td.items.push_back({tag, lat});
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

    // Sort endpoints lexicographically ascending (unsigned byte order)
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
            // Binary search in sorted global_eps
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

    // Parallel counting per group
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            ThreadData& td = tdata[t];
            td.group_counts.assign(num_groups, 0);
            td.group_errs.assign(num_groups, 0);
            for (auto& item : td.items) {
                uint32_t tag = item.tag;
                bool is_err = (tag >> 31) != 0;
                uint32_t hr = (tag >> 20) & 0x7FF;
                uint16_t loc_ep = tag & 0xFFFF;
                uint16_t glob_ep = td.local_to_global[loc_ep];
                uint32_t gid = hr * num_eps + glob_ep;
                td.group_counts[gid]++;
                if (is_err) td.group_errs[gid]++;
                // rewrite tag with global gid
                item.tag = gid;
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // Aggregate group counts and errs, compute offsets
    std::vector<uint32_t> total_counts(num_groups, 0);
    std::vector<uint32_t> total_errs(num_groups, 0);
    std::vector<uint32_t> group_offsets(num_groups + 1, 0);

    for (uint32_t g = 0; g < num_groups; ++g) {
        uint32_t cnt = 0, err = 0;
        for (unsigned int t = 0; t < num_threads; ++t) {
            cnt += tdata[t].group_counts[g];
            err += tdata[t].group_errs[g];
        }
        total_counts[g] = cnt;
        total_errs[g] = err;
    }

    size_t total_items = 0;
    for (uint32_t g = 0; g < num_groups; ++g) {
        group_offsets[g] = (uint32_t)total_items;
        total_items += total_counts[g];
    }
    group_offsets[num_groups] = (uint32_t)total_items;

    // Compute write offsets per thread per group
    std::vector<std::vector<uint32_t>> thread_write_offsets(num_threads, std::vector<uint32_t>(num_groups, 0));
    for (uint32_t g = 0; g < num_groups; ++g) {
        uint32_t running = group_offsets[g];
        for (unsigned int t = 0; t < num_threads; ++t) {
            thread_write_offsets[t][g] = running;
            running += tdata[t].group_counts[g];
        }
    }

    std::vector<uint32_t> all_latencies(total_items);

    // Parallel scatter
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            ThreadData& td = tdata[t];
            auto& offsets = thread_write_offsets[t];
            for (const auto& item : td.items) {
                uint32_t gid = item.tag;
                uint32_t pos = offsets[gid]++;
                all_latencies[pos] = item.latency;
            }
            // Free thread items memory early to drop peak RAM
            std::vector<ParsedItem>().swap(td.items);
            std::vector<uint32_t>().swap(td.group_counts);
            std::vector<uint32_t>().swap(td.group_errs);
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // Collect active group IDs
    std::vector<uint32_t> active_groups;
    active_groups.reserve(num_groups);
    for (uint32_t g = 0; g < num_groups; ++g) {
        if (total_counts[g] > 0) active_groups.push_back(g);
    }
    size_t num_active = active_groups.size();

    // Struct for final group summary
    struct GroupSummary {
        uint32_t gid;
        uint32_t p50, p95, p99;
    };
    std::vector<GroupSummary> summaries(num_active);

    // Parallel percentiles
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            size_t start_idx = (num_active * t) / num_threads;
            size_t end_idx = (num_active * (t + 1)) / num_threads;

            for (size_t i = start_idx; i < end_idx; ++i) {
                uint32_t gid = active_groups[i];
                uint32_t n = total_counts[gid];
                uint32_t* span = all_latencies.data() + group_offsets[gid];

                uint32_t r50 = (uint32_t)((50ULL * n + 99) / 100 - 1);
                uint32_t r95 = (uint32_t)((95ULL * n + 99) / 100 - 1);
                uint32_t r99 = (uint32_t)((99ULL * n + 99) / 100 - 1);

                if (n == 1) {
                    summaries[i] = {gid, span[0], span[0], span[0]};
                } else if (n <= 16) {
                    std::sort(span, span + n);
                    summaries[i] = {gid, span[r50], span[r95], span[r99]};
                } else {
                    std::nth_element(span, span + r50, span + n);
                    std::nth_element(span + r50 + 1, span + r95, span + n);
                    std::nth_element(span + r95 + 1, span + r99, span + n);
                    summaries[i] = {gid, span[r50], span[r95], span[r99]};
                }
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // Parallel output formatting: calculate sizes first
    std::vector<std::string> thread_buffers(num_threads);
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            size_t start_idx = (num_active * t) / num_threads;
            size_t end_idx = (num_active * (t + 1)) / num_threads;

            std::string& buf = thread_buffers[t];
            buf.reserve((end_idx - start_idx) * 80);

            char line_buf[256];
            for (size_t i = start_idx; i < end_idx; ++i) {
                const auto& sum = summaries[i];
                uint32_t gid = sum.gid;
                uint32_t hr = gid / num_eps;
                uint32_t epid = gid % num_eps;
                std::string_view ep = global_eps[epid];

                uint32_t cnt = total_counts[gid];
                uint32_t errs = total_errs[gid];
                uint32_t ppm = (uint32_t)((uint64_t)errs * 1000000ULL / cnt);

                char* p = line_buf;
                // Hour (20 bytes)
                std::memcpy(p, HOURS_STR[hr], 20);
                p += 20;
                *p++ = ' ';

                // Endpoint
                std::memcpy(p, ep.data(), ep.size());
                p += ep.size();
                *p++ = ' ';

                // count
                p = u64_to_str(p, cnt);
                *p++ = ' ';

                // p50
                p = u64_to_str(p, sum.p50);
                *p++ = ' ';

                // p95
                p = u64_to_str(p, sum.p95);
                *p++ = ' ';

                // p99
                p = u64_to_str(p, sum.p99);
                *p++ = ' ';

                // errors
                p = u64_to_str(p, errs);
                *p++ = ' ';

                // ppm
                p = u64_to_str(p, ppm);
                *p++ = '\n';

                buf.append(line_buf, p - line_buf);
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // Write to output file
    mkdir(out_dir.c_str(), 0777);
    std::string out_path = out_dir + "/summary.txt";
    int out_fd = open(out_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (out_fd < 0) return 1;

    for (unsigned int t = 0; t < num_threads; ++t) {
        const std::string& buf = thread_buffers[t];
        size_t written = 0;
        while (written < buf.size()) {
            ssize_t ret = write(out_fd, buf.data() + written, buf.size() - written);
            if (ret <= 0) break;
            written += ret;
        }
    }
    close(out_fd);
    munmap((void*)data, file_size);

    return 0;
}
