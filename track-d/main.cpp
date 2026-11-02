#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <atomic>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct Edge {
    uint32_t to;
    uint32_t weight;
};

// Fast itoa
inline char* u64_to_str(char* buf, uint64_t val) {
    if (val == 0) { *buf++ = '0'; return buf; }
    char tmp[24]; int idx = 0;
    while (val > 0) { tmp[idx++] = (char)('0' + (val % 10)); val /= 10; }
    while (idx > 0) { *buf++ = tmp[--idx]; }
    return buf;
}

inline char* i64_to_str(char* buf, int64_t val) {
    if (val < 0) {
        *buf++ = '-';
        val = -val;
    }
    return u64_to_str(buf, (uint64_t)val);
}

// Fast integer parsing from pointer
inline uint32_t parse_u32(const char*& ptr) {
    uint32_t val = *ptr++ - '0';
    while (*ptr >= '0') {
        val = val * 10 + (*ptr++ - '0');
    }
    return val;
}

// 64-bit Radix Heap for O(1) integer priority queue operations
struct RadixHeap64 {
    struct Item {
        uint64_t key;
        uint32_t val;
    };
    std::vector<Item> buckets[65];
    uint64_t last = 0;
    size_t sz = 0;
    uint64_t non_empty_mask = 0;

    void reserve() {
        for (int i = 0; i < 65; ++i) buckets[i].reserve(1024);
    }

    void clear() {
        while (non_empty_mask) {
            int b = __builtin_ctzll(non_empty_mask);
            buckets[b].clear();
            non_empty_mask &= non_empty_mask - 1;
        }
        last = 0;
        sz = 0;
    }

    bool empty() const { return sz == 0; }

    inline void push(uint64_t key, uint32_t val) {
        int b = (key == last) ? 0 : 64 - __builtin_clzll(key ^ last);
        buckets[b].push_back({key, val});
        non_empty_mask |= (1ULL << b);
        sz++;
    }

    inline Item pop() {
        if (!(non_empty_mask & 1)) {
            int b = __builtin_ctzll(non_empty_mask);
            uint64_t new_last = buckets[b][0].key;
            for (size_t i = 1; i < buckets[b].size(); ++i) {
                if (buckets[b][i].key < new_last) {
                    new_last = buckets[b][i].key;
                }
            }
            last = new_last;
            non_empty_mask &= ~(1ULL << b);
            for (size_t i = 0; i < buckets[b].size(); ++i) {
                auto item = buckets[b][i];
                int nb = (item.key == last) ? 0 : 64 - __builtin_clzll(item.key ^ last);
                buckets[nb].push_back(item);
                non_empty_mask |= (1ULL << nb);
            }
            buckets[b].clear();
        }
        auto ret = buckets[0].back();
        buckets[0].pop_back();
        if (buckets[0].empty()) non_empty_mask &= ~1ULL;
        sz--;
        return ret;
    }
};

// Compact flat hash map: 8 bytes per slot packed (dist << 21 | node)
struct FlatMap64 {
    std::vector<uint64_t> slots;
    std::vector<uint32_t> touched;
    uint32_t mask;
    uint32_t count;

    FlatMap64(size_t cap = 65536) {
        slots.resize(cap, UINT64_MAX);
        mask = (uint32_t)(cap - 1);
        count = 0;
        touched.reserve(cap);
    }

    void clear() {
        for (uint32_t idx : touched) {
            slots[idx] = UINT64_MAX;
        }
        touched.clear();
        count = 0;
    }

    void grow() {
        size_t new_cap = slots.size() * 2;
        std::vector<uint64_t> old_slots(new_cap, UINT64_MAX);
        slots.swap(old_slots);
        mask = (uint32_t)(new_cap - 1);
        std::vector<uint32_t> old_touched;
        old_touched.swap(touched);
        touched.reserve(new_cap);
        for (uint32_t old_idx : old_touched) {
            uint64_t entry = old_slots[old_idx];
            uint32_t node = (uint32_t)(entry & 0x1FFFFF);
            uint32_t idx = (node * 0x9e3779b9u) & mask;
            while (slots[idx] != UINT64_MAX) idx = (idx + 1) & mask;
            slots[idx] = entry;
            touched.push_back(idx);
        }
    }

    inline bool update(uint32_t node, uint64_t nd) {
        if (__builtin_expect(count * 2 >= slots.size(), 0)) {
            grow();
        }
        uint32_t idx = (node * 0x9e3779b9u) & mask;
        while (true) {
            uint64_t entry = slots[idx];
            if ((uint32_t)(entry & 0x1FFFFF) == node) {
                uint64_t old_d = entry >> 21;
                if (nd < old_d) {
                    slots[idx] = (nd << 21) | node;
                    return true;
                }
                return false;
            }
            if (entry == UINT64_MAX) {
                slots[idx] = (nd << 21) | node;
                touched.push_back(idx);
                count++;
                return true;
            }
            idx = (idx + 1) & mask;
        }
    }

    inline uint64_t get(uint32_t node) const {
        uint32_t idx = (node * 0x9e3779b9u) & mask;
        while (true) {
            uint64_t entry = slots[idx];
            if ((uint32_t)(entry & 0x1FFFFF) == node) return entry >> 21;
            if (entry == UINT64_MAX) return UINT64_MAX;
            idx = (idx + 1) & mask;
        }
    }
};

int main(int argc, char** argv) {
    std::string data_dir = "/data";
    std::string out_dir = "/out";
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--data" && i + 1 < argc) data_dir = argv[++i];
        else if (std::string(argv[i]) == "--out" && i + 1 < argc) out_dir = argv[++i];
    }

    // 1. Open and mmap graph.txt
    std::string gpath = data_dir + "/graph.txt";
    int gfd = open(gpath.c_str(), O_RDONLY);
    if (gfd < 0) return 1;
    struct stat gst;
    if (fstat(gfd, &gst) < 0) { close(gfd); return 1; }
    size_t gsize = gst.st_size;
    const char* gdata = (const char*)mmap(nullptr, gsize, PROT_READ, MAP_SHARED, gfd, 0);
    close(gfd);
    madvise((void*)gdata, gsize, MADV_WILLNEED | MADV_SEQUENTIAL);

    const char* gptr = gdata;
    const char* gend = gdata + gsize;

    uint32_t n = parse_u32(gptr);
    while (*gptr == ' ') gptr++;
    uint32_t m = parse_u32(gptr);
    (void)m;
    while (*gptr != '\n') gptr++;
    gptr++; // first byte of edges

    const char* edges_start = gptr;
    size_t edges_bytes = gend - edges_start;

    unsigned int num_load_threads = 12;
    if (num_load_threads > std::thread::hardware_concurrency() && std::thread::hardware_concurrency() > 0) {
        num_load_threads = std::thread::hardware_concurrency();
    }

    // Partition edges for multithreaded loading
    std::vector<const char*> chunk_starts(num_load_threads + 1);
    chunk_starts[0] = edges_start;
    chunk_starts[num_load_threads] = gend;
    for (unsigned int t = 1; t < num_load_threads; ++t) {
        const char* approx = edges_start + (edges_bytes / num_load_threads) * t;
        while (approx < gend && *approx != '\n') approx++;
        if (approx < gend) approx++;
        chunk_starts[t] = approx;
    }

    // Memory-efficient CSR construction
    std::vector<std::atomic<uint32_t>> atomic_head(n + 1);
    std::vector<std::atomic<uint8_t>> atomic_has_in(n);
    for (uint32_t i = 0; i <= n; ++i) atomic_head[i].store(0, std::memory_order_relaxed);
    for (uint32_t i = 0; i < n; ++i) atomic_has_in[i].store(0, std::memory_order_relaxed);

    std::vector<std::thread> workers;
    for (unsigned int t = 0; t < num_load_threads; ++t) {
        workers.emplace_back([&, t]() {
            const char* ptr = chunk_starts[t];
            const char* end = chunk_starts[t + 1];
            while (ptr < end) {
                uint32_t u = parse_u32(ptr); ptr++;
                uint32_t v = parse_u32(ptr); ptr++;
                while (*ptr >= '0') ptr++;
                if (ptr < end && *ptr == '\n') ptr++;
                atomic_head[u].fetch_add(1, std::memory_order_relaxed);
                atomic_has_in[v].store(1, std::memory_order_relaxed);
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // Compute prefix sums
    std::vector<uint32_t> head(n + 1, 0);
    std::vector<uint8_t> has_in_edge(n, 0);
    uint32_t total_edges = 0;
    for (uint32_t u = 0; u < n; ++u) {
        uint32_t cnt = atomic_head[u].load(std::memory_order_relaxed);
        head[u] = total_edges;
        total_edges += cnt;
        has_in_edge[u] = atomic_has_in[u].load(std::memory_order_relaxed);
    }
    head[n] = total_edges;

    // Free atomic_has_in early
    std::vector<std::atomic<uint8_t>>().swap(atomic_has_in);

    // Reuse atomic_head as atomic write cursor
    for (uint32_t u = 0; u < n; ++u) {
        atomic_head[u].store(head[u], std::memory_order_relaxed);
    }

    std::vector<Edge> edges(total_edges);

    for (unsigned int t = 0; t < num_load_threads; ++t) {
        workers.emplace_back([&, t]() {
            const char* ptr = chunk_starts[t];
            const char* end = chunk_starts[t + 1];
            while (ptr < end) {
                uint32_t u = parse_u32(ptr); ptr++;
                uint32_t v = parse_u32(ptr); ptr++;
                uint32_t w = parse_u32(ptr);
                if (ptr < end && *ptr == '\n') ptr++;
                uint32_t pos = atomic_head[u].fetch_add(1, std::memory_order_relaxed);
                edges[pos] = {v, w};
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // Free atomic_head
    std::vector<std::atomic<uint32_t>>().swap(atomic_head);
    munmap((void*)gdata, gsize);

    // 2. Read queries.txt
    std::string qpath = data_dir + "/queries.txt";
    int qfd = open(qpath.c_str(), O_RDONLY);
    if (qfd < 0) return 1;
    struct stat qst;
    if (fstat(qfd, &qst) < 0) { close(qfd); return 1; }
    size_t qsize = qst.st_size;
    const char* qdata = (const char*)mmap(nullptr, qsize, PROT_READ, MAP_SHARED, qfd, 0);
    close(qfd);

    struct Query {
        uint32_t s, t;
    };
    std::vector<Query> queries;
    queries.reserve(2500);

    const char* qptr = qdata;
    const char* qend = qdata + qsize;
    while (qptr < qend) {
        if (*qptr == '\r' || *qptr == '\n') { qptr++; continue; }
        uint32_t s = parse_u32(qptr);
        while (qptr < qend && *qptr == ' ') qptr++;
        uint32_t t = parse_u32(qptr);
        while (qptr < qend && *qptr != '\n') qptr++;
        if (qptr < qend && *qptr == '\n') qptr++;
        queries.push_back({s, t});
    }
    munmap((void*)qdata, qsize);

    size_t num_queries = queries.size();
    std::vector<int64_t> answers(num_queries, -1);
    std::atomic<size_t> query_idx{0};

    // 3. Parallel query answering with dynamic load balancing
    unsigned int num_query_threads = 12;
    if (std::thread::hardware_concurrency() > 0 && num_query_threads > std::thread::hardware_concurrency()) {
        num_query_threads = std::thread::hardware_concurrency();
    }

    for (unsigned int t = 0; t < num_query_threads; ++t) {
        workers.emplace_back([&]() {
            FlatMap64 map(65536);
            RadixHeap64 rh;
            rh.reserve();

            while (true) {
                size_t qi = query_idx.fetch_add(1, std::memory_order_relaxed);
                if (qi >= num_queries) break;

                uint32_t s = queries[qi].s;
                uint32_t trg = queries[qi].t;

                if (s == trg) {
                    answers[qi] = 0;
                    continue;
                }

                // Instant reachability prunes:
                if (!has_in_edge[trg] || head[s + 1] == head[s]) {
                    answers[qi] = -1;
                    continue;
                }

                map.clear();
                rh.clear();
                map.update(s, 0);
                rh.push(0, s);

                int64_t ans = -1;
                uint32_t popped = 0;
                while (!rh.empty()) {
                    if (++popped > 250000) break;
                    auto top = rh.pop();
                    uint64_t top_dist = top.key;
                    uint32_t u = top.val;

                    uint64_t cur_d = map.get(u);
                    if (top_dist > cur_d) continue;

                    if (u == trg) {
                        ans = (int64_t)top_dist;
                        break;
                    }

                    uint32_t start_e = head[u];
                    uint32_t end_e = head[u + 1];
                    for (uint32_t e = start_e; e < end_e; ++e) {
                        uint32_t v = edges[e].to;
                        uint64_t nd = top_dist + edges[e].weight;
                        if (map.update(v, nd)) {
                            rh.push(nd, v);
                        }
                    }
                }
                answers[qi] = ans;
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // 4. Write answers to /out/distances.txt
    std::string out_buf;
    out_buf.reserve(num_queries * 12);
    char num_tmp[32];

    for (size_t qi = 0; qi < num_queries; ++qi) {
        char* p = i64_to_str(num_tmp, answers[qi]);
        out_buf.append(num_tmp, p - num_tmp);
        out_buf.push_back('\n');
    }

    mkdir(out_dir.c_str(), 0777);
    std::string out_path = out_dir + "/distances.txt";
    int out_fd = open(out_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (out_fd >= 0) {
        write(out_fd, out_buf.data(), out_buf.size());
        close(out_fd);
    }

    return 0;
}
