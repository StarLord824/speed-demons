# Track D: Traverse (DemonicSpeed)

High-performance zero-dependency C++23 graph traversal engine for 5M directed weighted edges and 2,000 shortest-path queries.

## Approach

1. **Parallel CSR (Compressed Sparse Row) Graph Layout**:
   - The graph file is memory-mapped, and out-degrees are counted in parallel across 12 threads.
   - A single contiguous edge array `struct Edge { uint32_t to; uint32_t weight; }` (40 MB) and node offset array `head` (7 MB) are populated via parallel scatter.
   - Total graph representation occupies only 47 MB, maximizing L3 cache utilization and minimizing memory footprint.

2. **$O(1)$ Topological Reachability Pruning**:
   - A bitset tracks which nodes possess at least one incoming edge (`has_in_edge`).
   - If a target node $t$ has in-degree 0 (and $s \ne t$), the query immediately returns -1 in $O(1)$ time, eliminating the pathological worst-case traversals of the 1.74M main body that plagued the naive reference.
   - If a source node $s$ has out-degree 0 (and $s \ne t$), it immediately returns -1.

3. **Multithreaded Query Answering with Lazy Epoch Indexing**:
   - 2,000 queries are distributed evenly across 12 worker threads (~166 queries per thread).
   - Each thread uses a min-priority queue and an epoch-based distance array, lazily initializing visited nodes and avoiding full vector re-initialization per query.

## Dependencies

None. Zero non-standard-library runtime dependencies ($D = 1.0$).

## With another 24 hours

- Implement 4-ary radix heap for $O(1)$ integer priority queue operations.
- Add bidirectional Dijkstra search to halve the explored frontier radius.
