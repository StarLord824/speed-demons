# Track B: Index (DemonicSpeed)

High-performance zero-dependency C++23 search and indexing engine for 250,000 legal text documents and 50,000 boolean and phrase queries.

## Approach

1. **Query-First Indexing**:
   - Rather than indexing all 609,833 distinct corpus tokens across 254 MB, `/data/queries.txt` is ingested first.
   - Unique query terms across the 50,000 queries are extracted into an open-addressing term table.
   - Any token in the corpus that does not appear in any query is skipped during document ingestion, reducing index memory by >90%.

2. **Parallel Corpus Ingestion**:
   - Documents are partitioned across 12 worker threads.
   - ASCII lowercasing and maximal `[a-z0-9]` tokenization strictly adhere to Section 2.2 of the specification.
   - Thread-local posting lists are populated and concatenated into unified sorted posting lists.

3. **AVX2-Friendly Bitset Operations for Boolean Queries**:
   - For boolean evaluation, document matching is executed using a compact 31 KB bitset (3,907 `uint64_t` words) per thread.
   - `AND`, `OR`, and `NOT` operations are performed via bitwise instructions.
   - Counting and ID extraction leverage `__builtin_popcountll` and `__builtin_ctzll` (TZCNT) for near-instantaneous output aggregation.

4. **Anchor-First Candidate Intersection for Phrase Queries**:
   - Over 98% of phrase queries contain a rare anchor term.
   - The term with the shortest posting list is selected as the seed; candidate documents are filtered via binary search on the remaining phrase terms.
   - Candidate documents are verified with an exact token-window scan directly from memory-mapped text.

## Dependencies

None. Zero non-standard-library runtime dependencies ($D = 1.0$).

## With another 24 hours

- Implement bit-packed or Elias-Fano delta compression for inverted lists to reduce peak memory toward 30 MB.
- Direct SIMD-vectorized character scanning and token boundary detection.

