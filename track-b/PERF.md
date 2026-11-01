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
