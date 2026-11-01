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
    uint32_t val = 0;
    while (*ptr >= '0' && *ptr <= '9') {
        val = val * 10 + (*ptr++ - '0');
    }
    return val;
}

// 4-ary Min-Heap with packed (dist << 21 | u)
struct Fast4AryHeap {
    std::vector<uint64_t> data;

    void reserve(size_t cap) { data.reserve(cap); }
    void clear() { data.clear(); }
    bool empty() const { return data.empty(); }

    void push(uint64_t val) {
        size_t idx = data.size();
        data.push_back(val);
        while (idx > 0) {
            size_t parent = (idx - 1) >> 2;
            if (val < data[parent]) {
                data[idx] = data[parent];
                idx = parent;
            } else {
                break;
            }
        }
        data[idx] = val;
    }

    uint64_t pop() {
        uint64_t ret = data[0];
        uint64_t last = data.back();
        data.pop_back();
        if (!data.empty()) {
            size_t idx = 0;
            size_t n = data.size();
            while (true) {
                size_t child = (idx << 2) + 1;
                if (child >= n) break;
                size_t best = child;
                size_t end_child = std::min(child + 4, n);
                for (size_t c = child + 1; c < end_child; ++c) {
                    if (data[c] < data[best]) best = c;
                }
                if (data[best] < last) {
                    data[idx] = data[best];
                    idx = best;
                } else {
                    break;
                }
            }
            data[idx] = last;
        }
        return ret;
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

    // Memory-efficient CSR construction:
    // head array of atomics (only 7 MB)
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
                uint32_t u = parse_u32(ptr);
                ptr++; // ' '
                uint32_t v = parse_u32(ptr);
                ptr++; // ' '
                while (*ptr >= '0' && *ptr <= '9') ptr++;
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

    // Reuse atomic_head as atomic write cursor (head_cur)
    for (uint32_t u = 0; u < n; ++u) {
        atomic_head[u].store(head[u], std::memory_order_relaxed);
    }

    std::vector<Edge> edges(total_edges);

    for (unsigned int t = 0; t < num_load_threads; ++t) {
        workers.emplace_back([&, t]() {
            const char* ptr = chunk_starts[t];
            const char* end = chunk_starts[t + 1];
            while (ptr < end) {
                uint32_t u = parse_u32(ptr);
                ptr++; // ' '
                uint32_t v = parse_u32(ptr);
                ptr++; // ' '
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

    // 2. Discover the Main Body component via quick BFS from high-degree node
    uint32_t max_deg_node = 0;
    uint32_t max_deg = 0;
    for (uint32_t u = 0; u < n; ++u) {
        uint32_t d = head[u + 1] - head[u];
        if (d > max_deg) {
            max_deg = d;
            max_deg_node = u;
        }
    }

    std::vector<uint8_t> in_main_body((n + 7) / 8, 0);
    auto set_main = [&](uint32_t u) { in_main_body[u >> 3] |= (1 << (u & 7)); };
    auto is_main = [&](uint32_t u) -> bool { return (in_main_body[u >> 3] & (1 << (u & 7))) != 0; };

    {
        std::vector<uint32_t> q;
        q.reserve(n);
        q.push_back(max_deg_node);
        set_main(max_deg_node);
        size_t qhead = 0;
        while (qhead < q.size()) {
            uint32_t u = q[qhead++];
            uint32_t start_e = head[u];
            uint32_t end_e = head[u + 1];
            for (uint32_t e = start_e; e < end_e; ++e) {
                uint32_t v = edges[e].to;
                if (!is_main(v)) {
                    set_main(v);
                    q.push_back(v);
                }
            }
        }
    }

    // 3. Read queries.txt
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

    // 4. Parallel query answering with 4 threads
    unsigned int num_query_threads = 4;
    if (num_query_threads > std::thread::hardware_concurrency() && std::thread::hardware_concurrency() > 0) {
        num_query_threads = std::thread::hardware_concurrency();
    }

    for (unsigned int t = 0; t < num_query_threads; ++t) {
        workers.emplace_back([&, t]() {
            size_t start_q = (num_queries * t) / num_query_threads;
            size_t end_q = (num_queries * (t + 1)) / num_query_threads;

            // state[u]: [63..43: epoch (21 bits)][42..0: dist (43 bits)]
            std::vector<uint64_t> state(n, 0);
            uint32_t epoch = 0;
            Fast4AryHeap pq;
            pq.reserve(65536);

            for (size_t qi = start_q; qi < end_q; ++qi) {
                uint32_t s = queries[qi].s;
                uint32_t trg = queries[qi].t;

                if (s == trg) {
                    answers[qi] = 0;
                    continue;
                }

                // Instant reachability prunes:
                if (!has_in_edge[trg]) {
                    answers[qi] = -1;
                    continue;
                }
                if (head[s + 1] == head[s]) {
                    answers[qi] = -1;
                    continue;
                }
                // Disconnected component prune:
                if (is_main(s) && !is_main(trg)) {
                    answers[qi] = -1;
                    continue;
                }

                epoch++;
                state[s] = ((uint64_t)epoch << 43); // dist = 0
                pq.clear();
                pq.push(s); // (dist 0 << 21) | s

                int64_t ans = -1;
                while (!pq.empty()) {
                    uint64_t top = pq.pop();
                    uint64_t top_dist = top >> 21;
                    uint32_t u = (uint32_t)(top & ((1ULL << 21) - 1));

                    uint64_t u_state = state[u];
                    if ((u_state >> 43) != epoch) continue;
                    uint64_t cur_d = u_state & ((1ULL << 43) - 1);
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
                        uint64_t v_state = state[v];
                        if ((v_state >> 43) != epoch) {
                            state[v] = ((uint64_t)epoch << 43) | nd;
                            pq.push((nd << 21) | v);
                        } else {
                            uint64_t old_d = v_state & ((1ULL << 43) - 1);
                            if (nd < old_d) {
                                state[v] = ((uint64_t)epoch << 43) | nd;
                                pq.push((nd << 21) | v);
                            }
                        }
                    }
                }
                answers[qi] = ans;
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // 5. Write answers to /out/distances.txt
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
