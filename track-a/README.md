# Track A: Ingest (DemonicSpeed)

High-performance zero-dependency C++23 aggregation engine for 3.1M structured HTTP log lines.

## Approach

1. **Zero-Copy Memory-Mapped Parsing (`mmap`)**:
   - The input file `/data/requests.log` is mapped into process address space using `mmap` with `MADV_WILLNEED | MADV_SEQUENTIAL`.
   - The dataset is partitioned across 12 hardware threads, adjusting slice boundaries to the nearest newline character.

2. **Fixed-Time Arithmetic Timestamp Decoding**:
   - The spec pins timestamps strictly to a 72-hour calendar window (`2026-10-30T18` to `2026-11-02T17`).
   - Rather than invoking expensive calendar/timezone parsing, the month, day, and hour bytes are extracted directly and mapped to `hour_index in [0, 71]` using a constant-time branchless formula.

3. **Compact Open-Addressing Endpoint Interning**:
   - Endpoints are interned thread-locally into an 8,192-slot open-addressing hash table using FNV-1a.
   - At phase consolidation, endpoints across all threads are unified and sorted lexicographically in unsigned byte order (`memcmp`).
   - Group index is defined as `group_id = hour_index * num_endpoints + endpoint_id`, ensuring natural ascending order across all 126,908 groups without requiring a global sort.

4. **Contiguous Parallel Scatter & In-Place Selection**:
   - A single contiguous buffer of 3.1M `uint32_t` latencies is populated using thread-specific prefix-sum offsets, avoiding heap fragmentation and pointer indirection.
   - For groups with $n \le 16$, small sorting routines are used; for $n > 16$, `std::nth_element` is invoked on the required ranks ($r_{50}, r_{95}, r_{99}$).

5. **Buffered Concurrent Output Generation**:
   - Active groups are formatted into pre-sized thread-local buffers and flushed directly via sequential block writes to `/out/summary.txt`.

## Dependencies

None. Zero non-standard-library runtime dependencies ($D = 1.0$).

## With another 24 hours

- Implement AVX2 vectorization for scanning spaces and delimiter boundaries.
- Replace integer-to-string formatting with branchless SWAR / lookup table routines.
