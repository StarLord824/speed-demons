# Performance Log: Track A (Ingest)

## Baseline (Reference Implementation)
- **Time**: 32.05 s
- **Memory**: 228 MB
- **Bottlenecks**: `strptime` parsing of 27-byte RFC 3339 timestamps accounted for >80% of runtime. Dynamic Python list growth per endpoint and sorting each full list at completion incurred heavy allocation churn.

## Step 1: Memory-Mapped Parallel Scanning & Branchless Timestamp Parsing
- **Hypothesis**: Replacing `strptime` with arithmetic day/hour offsets on mmap slices will eliminate >90% of parsing overhead.
- **Implementation**: Mmap `/data/requests.log`, divide into 12 chunks, parse `month`, `day`, `hour` via integer arithmetic directly to `[0, 71]`.
- **Measurement**: Wall time dropped from 32.05 s to 1.15 s (~28x speedup). Peak memory dropped to 48 MB.

## Step 2: Thread-Local Endpoint Interning & Canonical Group IDs
- **Hypothesis**: Group keys `(hour, endpoint)` can be mapped into a dense 1D index `hour * num_endpoints + endpoint_id` if endpoints are sorted once up front.
- **Implementation**: 8,192-slot open-addressing thread-local hash table with FNV-1a. Consolidated and sorted 1,842 endpoints.
- **Measurement**: Eliminated map hashing overhead and final key sorting. Wall time dropped to 0.42 s.

## Step 3: Single Contiguous Latency Buffer & In-Place Selection
- **Hypothesis**: Dynamic vectors per group cause allocator contention. A single pre-sized 12.4 MB flat `uint32_t` array with thread-local prefix offsets will maximize L3 cache locality.
- **Implementation**: Computed prefix-sum offsets across all active groups. Scattered all 3.1M latencies into a flat array. Used `std::nth_element` on ranks 50, 95, 99.
- **Measurement**: Wall time dropped to 0.16 s. Peak memory dropped to 15.6 MB.

## Step 4: Parallel Output Formatting & Block Writes
- **Hypothesis**: Sequential string stream formatting bottlenecks the final phase.
- **Implementation**: Each thread formats its slice of active groups directly into pre-allocated memory buffers using fast integer-to-string routines, written sequentially to disk.
- **Measurement**: Final full-scale wall time dropped to 0.089 s (local) / ~0.016 s (Falkenstein scoring box). Correctness verified with exact SHA-256 match `495ca09682b9673ef836a506496177680bb4a3af54255174befddb1f83481cd3`.
