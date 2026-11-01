# Track C: Join (DemonicSpeed)

High-throughput zero-dependency C++23 relational aggregation engine for hostile multi-megabyte CSV data.

## Approach

1. **Flat Direct Lookup Array for Regions**:
   - `regions.csv` is mapped into memory and parsed to populate a direct array `uint8_t region_to_country[10000000]`.
   - Because `region_id < 10,000,000` and there are $\le 240$ countries, country IDs fit in 1 byte (with 255 reserved for `_UNMATCHED`).
   - Every join lookup resolves in $O(1)$ time with a single array subscript.

2. **Parallel Boundary Detection in Hostile CSV**:
   - While `note` fields can contain embedded newlines, the specification guarantees notes consist solely of lowercase alphabetical words and never start with digits.
   - Conversely, every valid record starts with digits (optionally preceded by a mid-file BOM `0xEF 0xBB 0xBF`).
   - Therefore, a newline followed by digits (or BOM + digits) followed by a comma uniquely and unambiguously denotes a true record boundary. This allows partitioning the 201 MB file across 12 threads with 100% mathematical certainty.

3. **In-Flight Note Byte Counting**:
   - Because notes only need their unescaped byte length in the output aggregates, note contents are never allocated or stored. Unescaped byte counts are incremented directly while scanning.

4. **Thread-Local Distinct Name Interning**:
   - Distinct names are interned into thread-local hash sets using 64-bit FNV-1a hashes.
   - At phase completion, thread aggregates and distinct name sets are merged into the final 240 country records.

## Dependencies

None. Zero non-standard-library runtime dependencies ($D = 1.0$).

## With another 24 hours

- Add SIMD quote/comma bitmask extraction (`_mm256_movemask_epi8`).
- Pre-allocate memory-mapped output buffer to avoid userspace write buffering.
