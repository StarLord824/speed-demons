# Performance Log: Track D (Traverse)

## Baseline (Reference Implementation)
- **Time**: 72.69 s
- **Memory**: 915 MB
- **Bottlenecks**: Loading the graph into Python tuples incurred massive allocator churn. Pathological unreachable target queries (targets with in-degree 0) caused forward Dijkstra to explore all 1.74M nodes of the main body, taking ~3 seconds per query.

## Step 1: CSR Graph Representation & Parallel Loading
- **Hypothesis**: Representing edges in a flat 40 MB CSR structure and indexing offsets in a 7 MB array will keep the entire graph within L3/RAM and allow parallel building.
- **Implementation**: Count node degrees in parallel, construct `head` prefix sums, and scatter edges into contiguous memory.
- **Measurement**: Graph ingestion time dropped from ~15 s to ~12 ms. Peak memory dropped from 915 MB to ~50 MB.

## Step 2: In-Degree Topological Pruning
- **Hypothesis**: Checking `has_in_edge[t] == 0` allows immediately answering -1 for unreachable targets in $O(1)$ time.
- **Implementation**: Bitset `has_in_edge` populated during graph ingestion.
- **Measurement**: Completely eliminated the 3-second stalls on pathological queries.

## Step 3: Multithreaded Dijkstra with Lazy Epoch Tracking
- **Hypothesis**: Distributing the 2,000 queries across 12 threads with lazy epoch counters avoids clearing the 1.74M distance array on each iteration.
- **Implementation**: Lazy epoch tracking on distance lookups across 12 concurrent workers.
- **Measurement**: Full-scale wall time dropped from 72.69 s to 0.97 s (local) / ~0.065 s (Falkenstein scoring box). Correctness verified with exact SHA-256 match `a23dbdf0a3e49cc8901310a9ee1dc6a9a569a85297b6da75c2a7002cb2ed277e`.

## Step 4: Lockless Atomic Graph Construction, 4-Ary Heap, and $O(1)$ Island Pruning
- **Hypothesis**: Separate per-thread degree and offset vectors consumed 188 MB of intermediate memory. Pushing unreached island queries from main body nodes traversed all 1.74M nodes of the graph.
- **Implementation**:
  - Replaced per-thread vectors with atomic `fetch_add` on a single 7 MB `head` array.
  - Implemented 4-ary min-heap with packed 64-bit `(dist << 21 | node)` keys.
  - Unified epoch and distance in a single 64-bit `state` array per thread `(epoch << 43 | dist)`.
  - Added $O(1)$ main body component reachability check `in_main[s] && !in_main[trg]` to eliminate multi-second graph stalls on unreachable queries.
- **Measurement**: Remote container memory dropped from 379.2 MB to 108.4 MB (4 threads) / 216.6 MB (12 threads). Score improved from 42.04 to 43.19 points. Exact SHA-256 match `a23dbdf0a3e49cc8901310a9ee1dc6a9a569a85297b6da75c2a7002cb2ed277e`.
