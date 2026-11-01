#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <thread>
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

struct HeapItem {
    uint64_t dist;
    uint32_t u;
    bool operator>(const HeapItem& o) const {
        return dist > o.dist;
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

    unsigned int num_threads = 12;
    if (num_threads > std::thread::hardware_concurrency() && std::thread::hardware_concurrency() > 0) {
        num_threads = std::thread::hardware_concurrency();
    }

    // Partition edges for multithreaded degree counting
    std::vector<const char*> chunk_starts(num_threads + 1);
    chunk_starts[0] = edges_start;
    chunk_starts[num_threads] = gend;
    for (unsigned int t = 1; t < num_threads; ++t) {
        const char* approx = edges_start + (edges_bytes / num_threads) * t;
        while (approx < gend && *approx != '\n') approx++;
        if (approx < gend) approx++;
        chunk_starts[t] = approx;
    }

    std::vector<std::vector<uint32_t>> thread_degrees(num_threads, std::vector<uint32_t>(n, 0));
    std::vector<uint8_t> has_in_edge(n, 0);
    std::vector<std::vector<uint8_t>> thread_has_in(num_threads, std::vector<uint8_t>(n, 0));

    std::vector<std::thread> workers;
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            const char* ptr = chunk_starts[t];
            const char* end = chunk_starts[t + 1];
            auto& deg = thread_degrees[t];
            auto& in_deg = thread_has_in[t];
            while (ptr < end) {
                uint32_t u = parse_u32(ptr);
                ptr++; // ' '
                uint32_t v = parse_u32(ptr);
                ptr++; // ' '
                // skip weight
                while (*ptr >= '0' && *ptr <= '9') ptr++;
                if (ptr < end && *ptr == '\n') ptr++;
                deg[u]++;
                in_deg[v] = 1;
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // Sum degrees to build CSR head array
    std::vector<uint32_t> head(n + 1, 0);
    for (uint32_t u = 0; u < n; ++u) {
        uint32_t d = 0;
        uint8_t in_e = 0;
        for (unsigned int t = 0; t < num_threads; ++t) {
            d += thread_degrees[t][u];
            in_e |= thread_has_in[t][u];
        }
        head[u] = d;
        has_in_edge[u] = in_e;
    }
    std::vector<std::vector<uint8_t>>().swap(thread_has_in);

    // Prefix sum
    uint32_t total_edges = 0;
    for (uint32_t u = 0; u < n; ++u) {
        uint32_t cnt = head[u];
        head[u] = total_edges;
        total_edges += cnt;
    }
    head[n] = total_edges;

    // Compute write offsets per thread
    std::vector<std::vector<uint32_t>> thread_offsets(num_threads, std::vector<uint32_t>(n, 0));
    for (uint32_t u = 0; u < n; ++u) {
        uint32_t cur = head[u];
        for (unsigned int t = 0; t < num_threads; ++t) {
            thread_offsets[t][u] = cur;
            cur += thread_degrees[t][u];
        }
    }
    std::vector<std::vector<uint32_t>>().swap(thread_degrees);

    std::vector<Edge> edges(total_edges);

    // Parallel scatter of edges
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            const char* ptr = chunk_starts[t];
            const char* end = chunk_starts[t + 1];
            auto& offsets = thread_offsets[t];
            while (ptr < end) {
                uint32_t u = parse_u32(ptr);
                ptr++; // ' '
                uint32_t v = parse_u32(ptr);
                ptr++; // ' '
                uint32_t w = parse_u32(ptr);
                if (ptr < end && *ptr == '\n') ptr++;
                uint32_t pos = offsets[u]++;
                edges[pos] = {v, w};
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

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

    // Parallel query answering
    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&, t]() {
            size_t start_q = (num_queries * t) / num_threads;
            size_t end_q = (num_queries * (t + 1)) / num_threads;

            std::vector<uint64_t> dist(n, UINT64_MAX);
            std::vector<uint32_t> visited_epoch(n, 0);
            uint32_t epoch = 0;
            std::priority_queue<HeapItem, std::vector<HeapItem>, std::greater<HeapItem>> pq;

            for (size_t qi = start_q; qi < end_q; ++qi) {
                uint32_t s = queries[qi].s;
                uint32_t trg = queries[qi].t;

                if (s == trg) {
                    answers[qi] = 0;
                    continue;
                }

                // Instant reachability prune:
                // If target has in-degree == 0, impossible to reach!
                if (!has_in_edge[trg]) {
                    answers[qi] = -1;
                    continue;
                }
                // If source has out-degree == 0, impossible to leave!
                if (head[s + 1] == head[s]) {
                    answers[qi] = -1;
                    continue;
                }

                epoch++;
                dist[s] = 0;
                visited_epoch[s] = epoch;
                while (!pq.empty()) pq.pop();
                pq.push({0, s});

                int64_t ans = -1;
                while (!pq.empty()) {
                    auto top = pq.top();
                    pq.pop();

                    if (top.dist > dist[top.u]) continue;
                    if (top.u == trg) {
                        ans = (int64_t)top.dist;
                        break;
                    }

                    uint32_t start_e = head[top.u];
                    uint32_t end_e = head[top.u + 1];
                    for (uint32_t e = start_e; e < end_e; ++e) {
                        uint32_t v = edges[e].to;
                        uint64_t nd = top.dist + edges[e].weight;
                        if (visited_epoch[v] != epoch) {
                            visited_epoch[v] = epoch;
                            dist[v] = nd;
                            pq.push({nd, v});
                        } else if (nd < dist[v]) {
                            dist[v] = nd;
                            pq.push({nd, v});
                        }
                    }
                }
                answers[qi] = ans;
            }
        });
    }
    for (auto& w : workers) w.join();
    workers.clear();

    // 3. Write output
    mkdir(out_dir.c_str(), 0777);
    std::string out_path = out_dir + "/distances.txt";
    int out_fd = open(out_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (out_fd < 0) return 1;

    std::string out_buf;
    out_buf.reserve(num_queries * 16);
    char line_buf[64];
    for (size_t i = 0; i < num_queries; ++i) {
        char* p = i64_to_str(line_buf, answers[i]);
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
