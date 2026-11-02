#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <atomic>
#include <cstring>
#include <pthread.h>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

inline uint32_t parse_u32(const char*& ptr) {
    uint32_t val = *ptr++ - '0';
    while (*ptr >= '0') val = val * 10 + (*ptr++ - '0');
    return val;
}

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

template<typename F>
void run_threads(unsigned int num_threads, F&& func) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 65536);

    struct ThreadData {
        const F* fn;
        unsigned int id;
    };
    std::vector<pthread_t> threads(num_threads);
    std::vector<ThreadData> tdata(num_threads);

    auto trampoline = [](void* arg) -> void* {
        ThreadData* td = static_cast<ThreadData*>(arg);
        (*(td->fn))(td->id);
        return nullptr;
    };

    for (unsigned int t = 0; t < num_threads; ++t) {
        tdata[t] = {&func, t};
        pthread_create(&threads[t], &attr, trampoline, &tdata[t]);
    }
    pthread_attr_destroy(&attr);

    for (unsigned int t = 0; t < num_threads; ++t) {
        pthread_join(threads[t], nullptr);
    }
}

// 4-ary Min-Heap with packed (dist << 21 | u)
struct Fast4AryHeap {
    std::vector<uint64_t> data;
    void reserve(size_t cap) { data.reserve(cap); }
    void clear() { data.clear(); }
    bool empty() const { return data.empty(); }
    size_t size() const { return data.size(); }
    uint64_t top() const { return data[0]; }

    void shrink_if_large() {
        if (__builtin_expect(data.capacity() > 16384, 0)) {
            data.shrink_to_fit();
            data.reserve(16384);
        }
    }

    void push(uint64_t val) {
        size_t idx = data.size();
        data.push_back(val);
        while (idx > 0) {
            size_t parent = (idx - 1) >> 2;
            if (val < data[parent]) { data[idx] = data[parent]; idx = parent; }
            else break;
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
                if (child + 3 < n) {
                    size_t best = child;
                    if (data[child + 1] < data[best]) best = child + 1;
                    if (data[child + 2] < data[best]) best = child + 2;
                    if (data[child + 3] < data[best]) best = child + 3;
                    if (data[best] < last) { data[idx] = data[best]; idx = best; }
                    else break;
                } else if (child < n) {
                    size_t best = child;
                    if (child + 1 < n && data[child + 1] < data[best]) best = child + 1;
                    if (child + 2 < n && data[child + 2] < data[best]) best = child + 2;
                    if (data[best] < last) { data[idx] = data[best]; idx = best; }
                    else break;
                } else {
                    break;
                }
            }
            data[idx] = last;
        }
        return ret;
    }
};

// Compact flat hash map: 8 bytes per slot packed (dist << 21 | node)
struct FlatMap64 {
    std::vector<uint64_t> slots;
    std::vector<uint32_t> touched;
    uint32_t mask;
    uint32_t count;

    FlatMap64(size_t cap = 16384) {
        slots.resize(cap, UINT64_MAX);
        mask = (uint32_t)(cap - 1);
        count = 0;
        touched.reserve(cap);
    }

    void clear() {
        if (__builtin_expect(slots.size() > 16384, 0)) {
            slots.assign(16384, UINT64_MAX);
            slots.shrink_to_fit();
            mask = 16383;
            touched.clear();
            touched.reserve(16384);
            count = 0;
            return;
        }
        for (uint32_t idx : touched) slots[idx] = UINT64_MAX;
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
        if (__builtin_expect(count * 2 >= slots.size(), 0)) grow();
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

struct OverflowEntry {
    uint32_t edge_idx;
    uint32_t weight;
};

inline uint32_t lookup_overflow(uint32_t e, const OverflowEntry* arr, size_t sz) {
    size_t l = 0, r = sz;
    while (l < r) {
        size_t mid = (l + r) >> 1;
        if (arr[mid].edge_idx < e) l = mid + 1;
        else r = mid;
    }
    return arr[l].weight;
}

int main(int argc, char** argv) {
    std::string data_dir = "/data";
    std::string out_dir = "/out";
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--data" && i + 1 < argc) data_dir = argv[++i];
        else if (std::string(argv[i]) == "--out" && i + 1 < argc) out_dir = argv[++i];
    }

    int gfd = open((data_dir + "/graph.txt").c_str(), O_RDONLY);
    if (gfd < 0) return 1;
    struct stat gst;
    if (fstat(gfd, &gst) < 0) { close(gfd); return 1; }
    size_t gsize = gst.st_size;
    const char* gdata = (const char*)mmap(nullptr, gsize, PROT_READ, MAP_SHARED, gfd, 0);
    madvise((void*)gdata, gsize, MADV_WILLNEED | MADV_SEQUENTIAL);

    const char* gptr = gdata;
    const char* gend = gdata + gsize;

    uint32_t n = parse_u32(gptr);
    while (*gptr == ' ') gptr++;
    uint32_t m = parse_u32(gptr);
    (void)m;
    while (*gptr != '\n') gptr++;
    gptr++;

    const char* edges_start = gptr;
    size_t edges_bytes = gend - edges_start;

    unsigned int num_load_threads = 12;
    if (num_load_threads > std::thread::hardware_concurrency() && std::thread::hardware_concurrency() > 0) {
        num_load_threads = std::thread::hardware_concurrency();
    }

    std::vector<const char*> chunk_starts(num_load_threads + 1);
    chunk_starts[0] = edges_start;
    chunk_starts[num_load_threads] = gend;
    for (unsigned int t = 1; t < num_load_threads; ++t) {
        const char* approx = edges_start + (edges_bytes / num_load_threads) * t;
        while (approx < gend && *approx != '\n') approx++;
        if (approx < gend) approx++;
        chunk_starts[t] = approx;
    }

    // Single unified head buffers: zero duplicate allocations
    std::vector<uint32_t> f_head(n + 1, 0);
    std::vector<uint32_t> b_head(n + 1, 0);
    std::atomic<uint32_t>* af_head = reinterpret_cast<std::atomic<uint32_t>*>(f_head.data());
    std::atomic<uint32_t>* ab_head = reinterpret_cast<std::atomic<uint32_t>*>(b_head.data());

    // Pass 1: count degrees directly (skipping self-loops)
    run_threads(num_load_threads, [&](unsigned int t) {
        const char* ptr = chunk_starts[t];
        const char* end = chunk_starts[t + 1];
        while (ptr < end) {
            uint32_t u = 0;
            while (*ptr >= '0') u = u * 10 + (*ptr++ - '0');
            ptr++;
            uint32_t v = 0;
            while (*ptr >= '0') v = v * 10 + (*ptr++ - '0');
            while (*ptr != '\n') ptr++;
            ptr++;
            if (__builtin_expect(u != v, 1)) {
                af_head[u].fetch_add(1, std::memory_order_relaxed);
                ab_head[v].fetch_add(1, std::memory_order_relaxed);
            }
        }
    });

    // In-place prefix sum: head[i] becomes starting offset for node i
    uint32_t f_tot = 0, b_tot = 0;
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t fd = f_head[i];
        f_head[i] = f_tot;
        f_tot += fd;

        uint32_t bd = b_head[i];
        b_head[i] = b_tot;
        b_tot += bd;
    }
    f_head[n] = f_tot;
    b_head[n] = b_tot;

    std::vector<uint32_t> f_edges(f_tot);
    std::vector<uint32_t> b_edges(b_tot);

    std::vector<std::vector<OverflowEntry>> local_f_ov(num_load_threads);
    std::vector<std::vector<OverflowEntry>> local_b_ov(num_load_threads);

    // Pass 2: scatter edges directly using af_head/ab_head cursors
    run_threads(num_load_threads, [&](unsigned int t) {
        local_f_ov[t].reserve(512);
        local_b_ov[t].reserve(512);
        const char* ptr = chunk_starts[t];
        const char* end = chunk_starts[t + 1];
        while (ptr < end) {
            uint32_t u = 0;
            while (*ptr >= '0') u = u * 10 + (*ptr++ - '0');
            ptr++;
            uint32_t v = 0;
            while (*ptr >= '0') v = v * 10 + (*ptr++ - '0');
            ptr++;
            uint32_t w = 0;
            while (*ptr >= '0') w = w * 10 + (*ptr++ - '0');
            if (ptr < end && *ptr == '\n') ptr++;
            if (__builtin_expect(u == v, 0)) continue;

            uint32_t f_pos = af_head[u].fetch_add(1, std::memory_order_relaxed);
            uint32_t b_pos = ab_head[v].fetch_add(1, std::memory_order_relaxed);

            uint32_t w_code = (w < 2047) ? w : 2047;
            f_edges[f_pos] = (w_code << 21) | v;
            b_edges[b_pos] = (w_code << 21) | u;

            if (__builtin_expect(w_code == 2047, 0)) {
                local_f_ov[t].push_back({f_pos, w});
                local_b_ov[t].push_back({b_pos, w});
            }
        }
    });

    // Shift heads back by 1 index so head[u] is start and head[u+1] is end
    std::memmove(f_head.data() + 1, f_head.data(), (size_t)n * sizeof(uint32_t));
    f_head[0] = 0;
    std::memmove(b_head.data() + 1, b_head.data(), (size_t)n * sizeof(uint32_t));
    b_head[0] = 0;

    madvise((void*)gdata, gsize, MADV_DONTNEED);
    munmap((void*)gdata, gsize);
    close(gfd);

    // Consolidate overflow entries
    std::vector<OverflowEntry> f_overflow, b_overflow;
    size_t total_f_ov = 0, total_b_ov = 0;
    for (unsigned int t = 0; t < num_load_threads; ++t) {
        total_f_ov += local_f_ov[t].size();
        total_b_ov += local_b_ov[t].size();
    }
    f_overflow.reserve(total_f_ov);
    b_overflow.reserve(total_b_ov);
    for (unsigned int t = 0; t < num_load_threads; ++t) {
        f_overflow.insert(f_overflow.end(), local_f_ov[t].begin(), local_f_ov[t].end());
        b_overflow.insert(b_overflow.end(), local_b_ov[t].begin(), local_b_ov[t].end());
        std::vector<OverflowEntry>().swap(local_f_ov[t]);
        std::vector<OverflowEntry>().swap(local_b_ov[t]);
    }
    std::sort(f_overflow.begin(), f_overflow.end(), [](const OverflowEntry& a, const OverflowEntry& b) {
        return a.edge_idx < b.edge_idx;
    });
    std::sort(b_overflow.begin(), b_overflow.end(), [](const OverflowEntry& a, const OverflowEntry& b) {
        return a.edge_idx < b.edge_idx;
    });

    const OverflowEntry* f_ov_ptr = f_overflow.data();
    size_t f_ov_sz = f_overflow.size();
    const OverflowEntry* b_ov_ptr = b_overflow.data();
    size_t b_ov_sz = b_overflow.size();

    // Read queries
    int qfd = open((data_dir + "/queries.txt").c_str(), O_RDONLY);
    if (qfd < 0) return 1;
    struct stat qst;
    if (fstat(qfd, &qst) < 0) { close(qfd); return 1; }
    size_t qsize = qst.st_size;
    const char* qdata = (const char*)mmap(nullptr, qsize, PROT_READ, MAP_SHARED, qfd, 0);
    close(qfd);

    struct Query { uint32_t s, t; };
    std::vector<Query> queries;
    queries.reserve(2500);

    const char* qptr = qdata;
    const char* qend = qdata + qsize;
    while (qptr < qend) {
        if (*qptr == '\r' || *qptr == '\n') { qptr++; continue; }
        uint32_t s = parse_u32(qptr); while (qptr < qend && *qptr == ' ') qptr++;
        uint32_t t = parse_u32(qptr); while (qptr < qend && *qptr != '\n') qptr++;
        if (qptr < qend && *qptr == '\n') qptr++;
        queries.push_back({s, t});
    }
    munmap((void*)qdata, qsize);

    size_t num_queries = queries.size();
    std::vector<int64_t> answers(num_queries, -1);
    std::atomic<size_t> query_idx{0};

    unsigned int num_query_threads = 12;
    if (std::thread::hardware_concurrency() > 0 && num_query_threads > std::thread::hardware_concurrency()) {
        num_query_threads = std::thread::hardware_concurrency();
    }

    run_threads(num_query_threads, [&](unsigned int /*t*/) {
        FlatMap64 map_f(16384), map_b(16384);
        Fast4AryHeap pq_f, pq_b;
        pq_f.reserve(16384);
        pq_b.reserve(16384);

        while (true) {
            size_t qi = query_idx.fetch_add(1, std::memory_order_relaxed);
            if (qi >= num_queries) break;

            uint32_t s = queries[qi].s;
            uint32_t trg = queries[qi].t;

            if (s == trg) { answers[qi] = 0; continue; }
            if (b_head[trg + 1] == b_head[trg] || f_head[s + 1] == f_head[s]) {
                answers[qi] = -1;
                continue;
            }

            map_f.clear();
            map_b.clear();
            pq_f.clear();
            pq_b.clear();
            pq_f.shrink_if_large();
            pq_b.shrink_if_large();

            map_f.update(s, 0);
            map_b.update(trg, 0);
            pq_f.push(s);
            pq_b.push(trg);

            uint64_t best_dist = UINT64_MAX;

            while (!pq_f.empty() && !pq_b.empty()) {
                uint64_t top_f = pq_f.top() >> 21;
                uint64_t top_b = pq_b.top() >> 21;

                if (top_f + top_b >= best_dist) break;

                if (pq_f.size() <= pq_b.size()) {
                    uint64_t top = pq_f.pop();
                    uint64_t d = top >> 21;
                    uint32_t u = (uint32_t)(top & 0x1FFFFF);

                    uint64_t cur_df = map_f.get(u);
                    if (d > cur_df) continue;

                    uint64_t cur_db = map_b.get(u);
                    if (cur_db != UINT64_MAX && d + cur_db < best_dist) {
                        best_dist = d + cur_db;
                    }

                    if (!pq_f.empty()) {
                        uint32_t nu = (uint32_t)(pq_f.top() & 0x1FFFFF);
                        __builtin_prefetch(&f_head[nu], 0, 1);
                        __builtin_prefetch(&f_edges[f_head[nu]], 0, 1);
                    }

                    uint32_t start_e = f_head[u];
                    uint32_t end_e = f_head[u + 1];
                    for (uint32_t e = start_e; e < end_e; ++e) {
                        uint32_t packed = f_edges[e];
                        uint32_t v = packed & 0x1FFFFF;
                        uint32_t w = packed >> 21;
                        if (__builtin_expect(w == 2047, 0)) {
                            w = lookup_overflow(e, f_ov_ptr, f_ov_sz);
                        }
                        uint64_t nd = d + w;
                        if (nd >= best_dist) continue;
                        if (v == trg) best_dist = std::min(best_dist, nd);
                        if (map_f.update(v, nd)) {
                            pq_f.push((nd << 21) | v);
                        }
                    }
                } else {
                    uint64_t top = pq_b.pop();
                    uint64_t d = top >> 21;
                    uint32_t u = (uint32_t)(top & 0x1FFFFF);

                    uint64_t cur_db = map_b.get(u);
                    if (d > cur_db) continue;

                    uint64_t cur_df = map_f.get(u);
                    if (cur_df != UINT64_MAX && cur_df + d < best_dist) {
                        best_dist = cur_df + d;
                    }

                    if (!pq_b.empty()) {
                        uint32_t nu = (uint32_t)(pq_b.top() & 0x1FFFFF);
                        __builtin_prefetch(&b_head[nu], 0, 1);
                        __builtin_prefetch(&b_edges[b_head[nu]], 0, 1);
                    }

                    uint32_t start_e = b_head[u];
                    uint32_t end_e = b_head[u + 1];
                    for (uint32_t e = start_e; e < end_e; ++e) {
                        uint32_t packed = b_edges[e];
                        uint32_t v = packed & 0x1FFFFF;
                        uint32_t w = packed >> 21;
                        if (__builtin_expect(w == 2047, 0)) {
                            w = lookup_overflow(e, b_ov_ptr, b_ov_sz);
                        }
                        uint64_t nd = d + w;
                        if (nd >= best_dist) continue;
                        if (v == s) best_dist = std::min(best_dist, nd);
                        if (map_b.update(v, nd)) {
                            pq_b.push((nd << 21) | v);
                        }
                    }
                }
            }

            if (best_dist != UINT64_MAX) answers[qi] = (int64_t)best_dist;
            else answers[qi] = -1;
        }
    });

    mkdir(out_dir.c_str(), 0777);
    std::string out_path = out_dir + "/distances.txt";
    int out_fd = open(out_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (out_fd >= 0) {
        char num_tmp[32];
        std::string out_buf;
        out_buf.reserve(num_queries * 12);
        for (size_t qi = 0; qi < num_queries; ++qi) {
            char* p = i64_to_str(num_tmp, answers[qi]);
            out_buf.append(num_tmp, p - num_tmp);
            out_buf.push_back('\n');
        }
        write(out_fd, out_buf.data(), out_buf.size());
        close(out_fd);
    }

    return 0;
}
