# Performance Log: Track B (Index)

## Baseline (Reference Implementation)
- **Time**: 56.35 s
- **Memory**: 2.29 GB
- **Bottlenecks**: Building full Python sets for all 609,833 corpus terms and re-tokenizing large candidate lists for phrases consumed >2 GB RAM and took nearly a minute.

## Step 1: Query-First Filtering
- **Hypothesis**: The task only evaluates the given 50,000 queries. Indexing only terms that actually appear in `queries.txt` will drastically cut memory and ingestion time.
- **Implementation**: Read `queries.txt` first, populate a term hash table, and skip unqueried tokens during corpus tokenization.
- **Measurement**: Memory dropped from 2.29 GB to under 50 MB (~45x reduction). Ingestion time dropped to ~0.3 s.

## Step 2: Parallel Document Tokenization
- **Hypothesis**: Slicing the 250,000 documents across 12 threads will scale ingestion linearly.
- **Implementation**: Parallel tokenization with thread-local posting lists, merged into sorted arrays.
- **Measurement**: Multi-threaded ingestion completed in under 0.25 s on local hardware.

## Step 3: Fast Bitset Evaluation & Anchor-First Phrase Verification
- **Hypothesis**: Evaluating boolean expressions on 31 KB bitsets and filtering phrase candidates by the rarest term avoids scanning full documents.
- **Implementation**: Left-to-right bitset boolean evaluation with hardware popcount and ctzll; anchor-first candidate check for phrases.
- **Measurement**: Full-scale wall time dropped from 56.35 s to 1.08 s (local) / ~0.065 s (Falkenstein scoring box). Correctness verified with exact SHA-256 match `f86973c210228141bcb6f1f1b10188ad2a9b65f71e7cd7cf4590b38de4e80927`.

## Step 4: Pre-Materialized Hot Bitsets & Stack-Allocated Phrase Verification
- **Hypothesis**: Re-constructing bitsets for frequently occurring words (like `the`, `and`, `of`) across 50,000 queries caused over 2 billion redundant bit operations. Re-tokenizing candidate documents with heap `std::string` allocations caused allocator churn.
- **Implementation**: Pre-materialize 31 KB bitsets once for frequently queried terms ($df \ge 64$ and query frequency $\ge 3$); verify candidate documents in phrase queries by tokenizing directly into a 1024-element `uint32_t` stack array, eliminating all heap string allocations.
- **Measurement**: Reduced retired instructions by ~3.6 billion and eliminated all heap churn during phrase checking. Correctness verified with exact SHA-256 match `f86973c210228141bcb6f1f1b10188ad2a9b65f71e7cd7cf4590b38de4e80927`.

## Step 5: Contiguous Flat CSR Postings, Parallel Slicing & O(1) Bit-Sum Table
- **Run ID**: `run-69c82b552ff9` (Commit `b841f87`)
- **Hypothesis**: In Step 4, allocating 31 KB bitsets for ~6,000 terms blew peak RSS to 472 MB. Moreover, allocating 908,964 separate `std::vector<uint32_t>` objects caused extreme heap fragmentation and cache thrashing. Slicing `docs.txt` single-threaded cost 123 ms, and extracting set bits for sum computation took hundreds of iterations per word.
- **Implementation**:
  1. Replaced all 75k `std::vector<uint32_t>` postings lists with a single flat CSR array `postings_data` (84.8 MB) and `postings_head`, constructed via a 2-pass parallel scatter with zero dynamic resizing.
  2. Restricted precomputed bitsets strictly to the top ~138 hot terms ($df \ge 1000$ and query frequency $\ge 50$), dropping bitset memory from 187 MB to 4.3 MB.
  3. Parallelized `docs.txt` line slicing across 12 threads.
  4. Implemented a 16-bit lookup table `WORD_BIT_SUM` to compute set-bit position sums in $O(1)$ instructions without looping over individual bits.
- **Measurement**: Falkenstein runner scored $T = 0.5742\text{ s}$, $M = 218.97\text{ MB}$, $S = 777$, $K = 11.392$, **52.83 points**. Correctness 100% verified with exact SHA-256 match `f86973c210228141bcb6f1f1b10188ad2a9b65f71e7cd7cf4590b38de4e80927`.

## Step 6: Single-Pass Document Tokenization, Direct 64-bit Hash Lookup & Rolling Ring Buffer Phrase Matcher
- **Run ID**: `run-be77c9858252` (Commit `7b3631a`)
- **Hypothesis**: Scanning the 253.8 MB file twice during document indexing doubled L3 cache misses. String `std::memcmp` lookups on every token in the hash table caused branch mispredictions. Re-tokenizing whole documents for phrase candidate matching spent unnecessary time when candidate phrases are only 2 to 8 tokens long.
- **Implementation**:
  1. Implemented a 256-byte `CHAR_MAP` lookup table to categorize and lowercase ASCII bytes in 1 instruction.
  2. Transitioned to single-pass document tokenization: documents are tokenized directly into thread-local arrays with on-the-fly counting, followed by a fast memory-to-memory scatter into the flat CSR postings buffer (19.6 ms).
  3. Replaced string `std::memcmp` in `TermTable::lookup` with direct 64-bit FNV-1a hash matching (0 collisions across 75,747 terms).
  4. Replaced full document tokenization in phrase queries with an 8-entry rolling ring buffer `TokHistory hist[8]` and 32-bit djb2 hash matching, short-circuiting instantly upon candidate discovery.
- **Measurement**: Falkenstein runner scored $T = 0.5828\text{ s}$, $M = 221.10\text{ MB}$, $S = 777$, $K = 11.286$, **52.63 points**. Correctness 100% verified with exact SHA-256 match `f86973c210228141bcb6f1f1b10188ad2a9b65f71e7cd7cf4590b38de4e80927`.

## Step 7: SIMD AVX2 Bitset Queries, Zero-Allocation Output Buffering & Source Size Trimming
- **Hypothesis**: Word-by-word bitset evaluation in 31,000 BOOL queries incurred high instruction overhead. Allocating 50,000 individual `std::string` objects for query outputs caused allocator churn. Candidate filtering in phrase queries repeatedly binary-searched large posting lists even for high-df terms whose bitsets could be tested in $O(1)$ time. 46 lines of dead `#elif defined(__ARM_NEON)` and fallback code penalized the $S$ source size metric.
- **Implementation**:
  1. Implemented explicit AVX2 SIMD intrinsics (`_mm256_and_si256`, `_mm256_or_si256`, `_mm256_andnot_si256`) with clean 3-line inline fallback for bitset boolean queries.
  2. Removed 52 lines of dead code and unused headers (`<iostream>`, `<chrono>`), trimming $S$ from 777 to 725 to directly boost the $(S_{\text{ref}}/S)^{0.15}$ factor.
  3. Substituted per-query `std::string` allocations with thread-local output string buffers (`thread_outputs[t].reserve(...)`), streaming directly to `write(out_fd, ...)`.
  4. Parallelized bitset precomputation across worker threads for terms with $(df \ge 1000 \land freq \ge 20) \lor df \ge 7800$.
  5. In phrase queries, candidate documents are checked against precomputed bitsets via $O(1)$ bit test (`(bptr[d / 64] >> (d % 64)) & 1`), skipping binary search for common terms.
  6. Sized `tids.reserve(num_d * 92)` to eliminate over-reservation across thread buffers.
- **Measurement**:
  - Local wall clock time dropped to **0.40 s** (down from 1.48 s baseline, ~3.7x faster).
  - User CPU time dropped to **2.92 s** (down from 6.85 s baseline).
  - Retired instructions dropped from **36.16 billion to 13.91 billion** (-61.5% retired instructions).
  - Cycles elapsed dropped from **17.10 billion to 7.60 billion** (-55.5% cycles).
  - Source lines $S$ dropped from **777 to 725 lines**.
  - Output SHA-256 hash verified 100% bit-for-bit exact: `f86973c210228141bcb6f1f1b10188ad2a9b65f71e7cd7cf4590b38de4e80927`.

## Step 8: Direct-to-Bitset Materialization, 64-Doc Word Alignment, Binary Search Document Partitioning & Sparse Reset
- **Hypothesis**:
  1. The separate document slicing pass (`docs` vector of 250k slices) scanned the entire 1.2 GB corpus, taking 155 ms of redundant I/O before tokenization.
  2. The top 718 terms with query frequency $\ge 15$ account for over 11.46 million postings (51.5% of ALL postings in the corpus). Storing them in intermediate arrays (`thread_tids`) and later scattering them into `postings_data` doubled memory consumption during ingestion to >220 MB.
  3. Pre-materializing bitsets for these 718 hot terms directly during the document scan eliminates 11.46M entries from `thread_tids` and `postings_data`.
  4. Aligning thread document partitions to multiples of 64 guarantees that each thread writes to strictly disjoint 64-bit words in all bitsets, eliminating locks, atomics, and data races.
  5. Slicing can be eliminated completely: finding 12 document boundaries with binary search takes only 615 microseconds, allowing document offsets and lengths (`doc_text_off`, `doc_text_len`) to be recorded on the fly during tokenization.
  6. In boolean query execution, resetting `bitset_tmp` with full 31.2 KB `std::memset` for every non-hot term across 31,000 queries forced 1.8 GB of zeroing overhead. Sparse clearing (`bitset_tmp[d / 64] = 0` for only touched words) eliminates 99.8% of zeroing instructions.
- **Implementation**:
  - Implemented 615-microsecond binary search `find_doc_start` to partition documents into exact multiples of 64 docs per thread.
  - Eliminated the 250,000-element `DocSlice` vector (saved 4 MB dynamic allocation and 155 ms file scan). Recorded `doc_text_off[250000]` and `doc_text_len[250000]` in static memory (1.5 MB).
  - Pre-allocated a contiguous `hot_bitsets` buffer (22.4 MB) for the 718 most queried terms (`query_term_freq >= 15`). During the single-pass document scan, threads populate their disjoint 64-bit words directly without atomics or locks.
  - Omitted hot terms from `thread_tids` and `postings_data`, cutting intermediate buffer sizes and scatter iterations by 51.5% (from 22.2M to 10.78M entries).
  - Implemented sparse resetting of `bitset_tmp` after boolean operations, clearing only the words actually modified.
  - Intersected precomputed bitsets directly for phrase queries where all terms are hot.
- **Measurement**:
  - Local wall clock time dropped to **0.97 s** (internal phase execution dropped from 825 ms to **378 ms**).
  - Peak memory footprint dropped from **223.5 MB to 139.2 MB** (-84.3 MB / -37.7% peak memory reduction).
  - Retired instructions dropped to **13.02 billion**.
  - Output SHA-256 hash verified 100% bit-for-bit exact: `f86973c210228141bcb6f1f1b10188ad2a9b65f71e7cd7cf4590b38de4e80927`.

## Step 9: Seed-Anchored Phrase Verification, AVX2 SIMD Scanner, Single-Pass Line Tokenization & Boolean Early Exit
- **Hypothesis**:
  1. Profiling revealed that phrase query candidate verification (`fast_doc_phrase_match`) consumed 183 ms (over 80% of query evaluation time) because it scanned every single byte of candidate documents (4.8 KB each) and hashed every word with djb2 into a rolling ring buffer.
  2. Because candidate documents were selected using the rarest term in the phrase (`min_idx`), that rarest word appears at most 1 or 2 times in the document. Anchoring the search directly on the rarest term and only checking immediately adjacent words reduces candidate text scanning by >95%.
  3. Scanning for the first character of the seed word can be accelerated with AVX2 SIMD (`_mm256_cmpeq_epi8` / `_mm256_movemask_epi8`), skipping 32 bytes per cycle.
  4. In document tokenization, scanning ahead to find `\n` to calculate `text_len` read the entire 1.2 GB corpus twice. Stopping directly at `\n` during tokenization eliminates the second full-corpus read.
  5. In boolean queries without `OR` operators (53.7% of all boolean queries), as soon as an `AND` operation results in all zeros, the query can early-exit immediately without evaluating remaining operators or iterating over 3,907 words.
- **Implementation**:
  - Implemented `seed_phrase_match`: searches for whole-word occurrences of the rarest phrase word (`p_views[seed_idx]`) using `avx2_find_char_pair` (32 bytes per cycle), and verifies adjacent preceding and following words in $O(\text{phrase\_len})$ without full document tokenization.
  - Replaced the two-pass line scan (`while (*ptr != '\n') ptr++` followed by tokenization) with a single unified pass that stops directly at `\n`.
  - Added boolean query early exit: `bitset_and` uses `_mm256_testz_si256` to detect empty accumulators in 1 cycle. If empty and no subsequent `OR` operators exist, appends `"0 0\n"` and immediately continues to the next query.
  - Sized `tids.reserve(num_d * 53)` and `thread_outputs[t].reserve((q_end - q_start) * 40)` to minimize heap over-allocation.
- **Measurement**:
  - Query evaluation time dropped from **158 ms to 93 ms** (-41%).
  - Total retired instructions dropped from **13.02 billion to 10.61 billion** (-2.41 billion instructions).
  - CPU cycles dropped from **7.72 billion to 6.14 billion** (-1.58 billion cycles).
  - User CPU time dropped from **3.41 s to 2.64 s** (-22.6%).
  - Peak memory footprint dropped to **137.1 MB**.
  - Output SHA-256 hash verified 100% bit-for-bit exact: `f86973c210228141bcb6f1f1b10188ad2a9b65f71e7cd7cf4590b38de4e80927`.

## Step 10: Compact L2 Term Table, 1M-Bit L2 Filter, Hybrid Sparse-Doc Boolean Evaluation & Ordered Phrase Verification
- **Hypothesis**:
  1. `TermTable`'s 24-byte `Slot` structures totaled 6.29 MB across 262,144 entries, exceeding the 1.25 MB L2 cache of the Intel i5-13500 P-cores and causing heavy L3 cache thrashing during multi-threaded token lookups. Splitting into flat arrays (`hashes[131072]` at 1.04 MB and `tids[131072]` at 0.52 MB) fits 100% within L2 cache.
  2. The 64K-bit (8 KB) query filter had an observed 68.5% false positive rate when populated with 75,747 unique terms, allowing ~14 million unqueried words to trigger expensive hash table lookups. Expanding the filter to 1M bits (128 KB, fits in L2 cache) drops the false positive rate from 68.5% to 7.0%, eliminating 93% of unqueried hash lookups in a single instruction.
  3. Over 44% of boolean queries have no `OR` operator and a sparse initial seed term ($df_0 \le 1024$). Constructing and clearing 31.25 KB bitsets (3,907 `uint64_t` words) and iterating all words with popcount/bit-sum for tiny doc sets wasted millions of instructions. Evaluating these sparse queries directly on a local doc array with $O(1)$ hot bitset filtering eliminates bitset allocations and full-word reductions entirely.
  4. In phrase query candidate checking, verifying terms in arbitrary query order caused large `std::binary_search` lookups even when a hot term bit test would reject the document in 1 cycle. Checking hot terms first ($O(1)$ bit test) followed by non-hot terms sorted in ascending order of posting list size short-circuits candidate verification immediately.
- **Implementation**:
  - Replaced 24-byte `Slot` table with compact L2-resident `TermTable` (`hashes[131072]` and `tids[131072]`), tracking term string views directly upon insertion.
  - Sized `QUERY_TERM_FILTER[16384]` to 1,048,576 bits (128 KB), filtering words via `uint32_t q_bit = (uint32_t)h & 0xFFFFF`.
  - Added hybrid boolean execution: queries without `OR` and $df_0 \le 1024$ execute via direct array filtering (`cur_docs[1024]`) with $O(1)$ hot-term bit tests and binary searches on sparse lists, calculating count/sum/top-10 in $O(k)$ rather than scanning 3,907 bitset words.
  - Reordered phrase candidate checks to test hot term bitsets first in $O(1)$ before conducting binary searches ordered by posting list size.
- **Measurement**:
  - Total retired instructions dropped from **11.28 billion to 10.35 billion** (-930 million instructions).
  - CPU cycles elapsed dropped from **6.57 billion to 5.52 billion** (-1.05 billion cycles / -16.0%).
  - Real wall time dropped to **0.84 s** (internal phase execution dropped under 500 ms).
  - Peak memory footprint dropped to **132.8 MB**.
  - Output SHA-256 hash verified 100% bit-for-bit exact: `f86973c210228141bcb6f1f1b10188ad2a9b65f71e7cd7cf4590b38de4e80927`.

## Step 11: 4-Block Partitioning, 16-Bit Local Postings & Block-Scoped Intermediate Memory Freeing
- **Run ID**: `run-a522966ca1a9` (Commit `68999161d7dd753c211a55e87da35558c4688aee`)
- **Hypothesis**:
  1. Storing 10.78 million non-hot postings as global `uint32_t` values required 43.1 MB in `postings_data` and another 43.1 MB in `thread_tids`, causing a peak allocation spike of >130 MB when both existed simultaneously.
  2. Partitioning the 250,000 documents into 4 disjoint blocks of $\le 62,528$ documents ensures that every local document ID relative to the block's base offset strictly fits into a `uint16_t` (2 bytes). This cuts the flat postings storage by exactly 50% (from 43.1 MB down to 21.56 MB).
  3. By tokenizing and scattering each block sequentially across the 12 worker threads, thread intermediate vectors (`thread_tids`, `thread_doc_start`) are sized for only 1/4 of the corpus at any given time (~5 MB total across all 12 threads) and are immediately freed upon block completion via `.swap()`, preventing intermediate buffers from coexisting with the full corpus index.
  4. In query evaluation, phrase candidate checking performs 16-bit binary searches within localized block slices, doubling cache line density (32 postings per 64-byte cache line) and improving L2 cache hit rates.
- **Implementation**:
  - Divided the 250,000 document corpus into 4 blocks of 977 64-doc words ($\le 62,528$ docs per block).
  - Allocated per-block posting tables `block_postings_head[4]` (1.21 MB total) and `block_postings_data[4]` using `std::vector<uint16_t>`.
  - Processed document ingestion block-by-block: 12 threads tokenize block $b$ into localized buffers, compute prefix sums for block $b$, scatter `uint16_t` offsets directly into `block_postings_data[b]`, and immediately release intermediate thread memory before advancing to block $b+1$.
  - Updated hybrid boolean evaluation and phrase candidate checking to iterate over the 4 blocks, naturally generating matches in ascending document order without sorting.
- **Measurement**:
  - Live Falkenstein runner scored: $T = 0.4023\text{ s}$, $M = 83.32\text{ MB}$ (dropped from 137.56 MB, -54.2 MB / -39.4%), $S = 1079$, $K = 15.718$, **59.820 points** (all gates passed, new personal best).
  - Output SHA-256 hash verified 100% bit-for-bit exact: `f86973c210228141bcb6f1f1b10188ad2a9b65f71e7cd7cf4590b38de4e80927`.

## Step 12: Zero-Intermediate-Postings 2-Pass Block Ingestion & 64 KB POSIX Thread Stacks
- **Hypothesis**:
  1. In Step 11, allocating intermediate vectors (`thread_tids`, `thread_doc_start`) for each block still incurred ~5 MB of transient heap allocations and allocator churn. Because `docs.txt` is mmapped directly in memory cache, executing a two-pass scan per block (Pass 1 counts per-term frequencies in a compact 1.8 MB `thread_counts` array; Pass 2 writes `uint16_t` local doc IDs directly into `block_postings_data[b]`) completely eliminates all intermediate posting vectors from RAM.
  2. Standard `std::thread` on Linux pre-allocates an 8 MB virtual stack per thread (96 MB virtual memory across 12 threads) and creates separate glibc heap arenas. Switching to a lightweight POSIX `ThreadRunner` with `pthread_attr_setstacksize(&attr, 64 * 1024)` caps thread stack space to 64 KB per thread (0.75 MB total), avoiding glibc memory pool bloat.
  3. Increasing the hot-term bitset threshold from query freq $\ge 15$ to $\ge 35$ retains the 212 highest-frequency terms while shrinking `hot_bitsets` from 22.4 MB to 6.47 MB (-15.9 MB reduction).
- **Implementation**:
  - Implemented `ThreadRunner` using POSIX `pthread_attr_setstacksize` (64 KB stack per worker).
  - Implemented 2-pass per-block ingestion: Pass 1 calculates per-thread term frequencies in `uint16_t thread_counts[12][75747]` (1.81 MB); prefix sums compute exact offsets; Pass 2 streams `uint16_t` doc IDs directly into `block_postings_data[b]`. Zero intermediate posting vectors allocated.
  - Trimmed `hot_bitsets` to 6.47 MB (212 hot terms).
- **Measurement**:
  - Local peak memory footprint dropped from 84.6 MB to **59.6 MB** (-25.0 MB / -29.5% peak RSS reduction, projected ~38-42 MB in Linux cgroups).
  - Wall-clock time on local benchmark dropped from 0.80 s to **0.57 s** (-28.7% wall time).
  - Clean warning-free compilation with `-std=c++20 -O3 -Wall -Wextra -Werror -march=native -flto -pthread`.
  - Output SHA-256 hash verified 100% bit-for-bit exact: `f86973c210228141bcb6f1f1b10188ad2a9b65f71e7cd7cf4590b38de4e80927`.






