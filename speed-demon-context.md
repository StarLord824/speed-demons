# Speed Demon 2026: complete context for an LLM

Generated from speeddemon.dev on 2026-11-01 14:12 UTC. Paste this whole file into your LLM and ask your questions.

How to read it:
- Speed Demon is a 72-hour performance-engineering championship run by Hackathon Raptors (UK CIC 15557917), October 30, 15:00 UTC to November 2, 15:00 UTC, 2026. Online, free.
- Order of authority when two places disagree: the frozen track specs and harness (github.com/hackathon-raptors/speed-demon-2026), then the website pages, then the clarifications, then the video transcript.
- If the answer is not in this file, ask on Discord: https://discord.com/invite/xfYPDZYqeh. Do not guess.

Instructions for the assistant: answer only from this document, quote the section you rely on, and say plainly when the document does not answer the question.

## Key links

- Website: https://speeddemon.dev
- Plain-English explainer: https://speeddemon.dev/start
- Setup guide (kickstart): https://speeddemon.dev/kickstart
- Live board: https://speeddemon.dev/board
- Register (every team member): https://tally.so/r/q4redG
- Team submission form (one per team, resubmit to change): https://tally.so/r/Ar7g9z
- Starter template: https://github.com/hackathon-raptors/speed-demon-starter (branch speed-demon)
- Specs, generators, references, harness: https://github.com/hackathon-raptors/speed-demon-2026
- Speed Demon Board GitHub App: https://github.com/apps/speed-demon-board
- Image allowlist: https://speeddemon.dev/allowlist.json
- Discord: https://discord.com/invite/xfYPDZYqeh
- Contact: hello@raptors.dev

---

# Page: Home (https://speeddemon.dev/)

## Speed Demon

Every millisecond is a soul. · Same task, same data, same machine. We run your code ourselves and rank what comes back. · 72-hour performance championship · Halloween weekend

track A · ingest · 3.1M log lines · 239 MB · fsn1 · i5-13500 · 12 threads · 8 GB cap

reference implementation · 32.05 s · 228 MB

souls 32,050 souls

what a serious submission looks like · 1.28 s · 40 MB

souls 1,280 souls

✓ sha256 · identical output · 25.0× faster · 5.7× leaner · illustrative efficiency coefficient K = 6.41

- October 30 – November 2, 2026

- Online

- Free

- 4 tracks

- $1,800 prize pool

Register (https://tally.so/r/q4redG) Submit your repo (https://tally.so/r/Ar7g9z) Read the scoring Explain it plainly

The reference implementation takes 25.0 times longer than a strong submission on the same task and the same machine.

AI ask your own LLM

### Lost in the rules? Ask your LLM.

This event has a lot of moving parts. One button copies all of it as a single Markdown file. Paste it into ChatGPT, Claude, Gemini or a local model, and ask anything.

Copy context (https://speeddemon.dev/speed-demon-context.md)

~45k tokens · rebuilt with every site update · open the file ↗ (https://speeddemon.dev/speed-demon-context.md)

Try asking:

- › Which libraries are banned on Track C, and is a CSV parser allowed?

- › Do my writes to /tmp count toward peak memory?

- › I push at 14:58 UTC on November 2. Does that commit count?

- › Explain the K formula with my numbers: 4.2 s, 310 MB, 140 lines, 1 dependency, Track A.

what is inside

- 01 · Every page of this site: rules, tracks, scoring, gates, harness, timeline, prizes, FAQ

- 02 · Clarifications we gave on Discord and by email

- 03 · All four track specs, with banned libraries and exact output formats

- 04 · The starter template README and the harness README

- 05 · The image allowlist, with digests

- 06 · The full transcript of the kickoff video

An LLM can still be wrong. If the answer matters, check the spec it quotes, or ask on Discord.

00 before the freeze

### Changes before freeze (Oct 26)

These rules changed ahead of the October 28 freeze. The rest of this page already reflects every line below.

- 01 · Four tracks, A to D. Track E (Compress) is cut: storing the input raw outscored a real codec.

- 02 · Track B is timed as one total wall time, and its queries are readable from the start. The 40/60 build/query split is gone.

- 03 · One naive Python reference per track, instead of Go, Python and Rust.

- 04 · The scoring machine is in Falkenstein (fsn1), not Helsinki: the same Intel Core i5-13500, the same 64 GB and the same limits.

- 05 · New formula: K = (T_ref/T)^0.50 × (M_ref/M)^0.20 × (S_ref/S)^0.15 × (D_ref/D)^0.15. The scaling axis X is gone, and its weight moved to wall time.

- 06 · Wall time has no cap. Memory is capped at 50×. Source size and dependencies can cost you but never pay.

- 07 · Source size is code lines counted by scc after your formatter, across each track's folder minus registry packages first published before Oct 1, 2026 by people outside the team. Thanks to a participant report, this closes the library-as-dependency loophole.

- 08 · D = 1 + the number of direct non-stdlib runtime dependencies in your manifest. The vendored-bytes term is gone.

- 09 · Speed score = 50 × log10(K), clamped to 0 to 100: K = 10 is half marks and K = 100 is full marks.

- 10 · The speed floor gate is gone. Three gates remain: correctness, the timeout and the sandbox.

- 11 · The overall title is the geometric mean of your best three track totals, from three or more tracks entered.

- 12 · Prizes total $1,800: $800, $400 and $200 overall, $200 for two write-ups, and $50 for the highest K on each track. The lean award is gone.

- 13 · Judges see each team's live-board run history, and PERF.md steps cite run IDs. Gate pass/fail comes out by Nov 4; final scores and full breakdowns come with the winners on Nov 13.

- 14 · Every FROM must name an image from speeddemon.dev/allowlist.json, pinned by digest, and nothing else may enter the image: no COPY --from another image, no ADD of a URL. Builds stay offline and are capped at 10 minutes, images at 2 GB.

- 15 · To use the live board, install the Speed Demon Board GitHub App on your repo (https://github.com/apps/speed-demon-board). It reads only the repos you pick, and repos can stay private until code freeze.

- 16 · Build-time training (PGO, a JVM AOT cache, native-image) is allowed on generator output. /data is not mounted at build time, and a byte-identical copy of our generator in your repo costs nothing in source size.

- 17 · io_uring stays blocked by Docker's default seccomp profile. mmap works.

- 18 · The live board runs on the full public dataset released at kickoff, on the scoring machine itself. It closes at code freeze, so final scoring has the machine to itself, on a holdout seed nobody has seen.

- 19 · Track D's hints now match its data: queries are local, so parsing and memory layout decide most of the time.

- 20 · The Oct 31 teardown of the reference moves into the results write-up.

- 21 · The worked example is recomputed under the new formula and marked illustrative.

- 22 · Reference numbers are now measured on the scoring machine, five runs each: Track A 32.05 s, B 56.35 s, C 14.44 s, D 72.69 s. Timeouts are 10 times those.

- 23 · Rust PGO works offline: halloumee/speed-demon-rust is rust:1-trixie with the llvm-tools component, so llvm-profdata matches rustc. It is on the allowlist at the same compiler, 1.99.1. Thanks to a participant request.

- 24 · Images are built offline on a separate build machine, then run on the scoring box. A track that compiles for the native CPU is built on the scoring box itself, so native means the scoring CPU.

- 25 · Kickoff moves three hours earlier, to October 30 at 15:00 UTC, and code freeze to November 2 at 15:00 UTC. The event is still 72 hours.

- 26 · There is no separate sample release. The full datasets and their expected hashes are published at kickoff; seed 1 at full scale is the board's dataset.

01 the premise

### You can't argue with a stopwatch

Most hackathons ask a panel whether they liked your idea. This one starts by measuring whether your code is fast, and only then asks whether it's any good.

Every track hands you the same three things: a task specification, a dataset, and a deliberately naive reference implementation that solves the task correctly and slowly. Your job is to solve it again — correctly and not slowly. Any language. Any approach. The output has to hash identical to the reference, byte for byte, or you don't get a score at all.

Then we run it. Not you. Every submission is built from your Dockerfile and executed on one dedicated machine in Falkenstein, pinned to the same cores, capped at the same memory, five runs each, serialized so nothing competes for cache. Your laptop's numbers are for your own iteration. Ours are the ones on the board.

What comes out is an efficiency coefficient: a single number that folds wall time, peak memory, source size and dependency weight into one ranking. It is published in full below, with a worked example, before you write a line. That coefficient decides 65% of your score. Human judges decide the rest, because a submission nobody can read is not an engineering achievement.

There is a particular kind of engineer this event is built for: the one who has spent a week making something 40% faster and then had to explain to a room why that mattered. Here it doesn't need explaining. It's the score.

the shape of it

- task spec given

- dataset given

- naive reference given

- a faster one yours

65% · of your score is measured by machine · on a formula published before kickoff

5 · runs per submission · highest and lowest discarded, mean of the middle three

1 · machine runs every submission · cores pinned, turbo off, serialized

0 · self-reported timings accepted · ever

02 A through D

### Four tracks

Each track is one fixed task against one fixed dataset. They stress different muscles on purpose — a parsing specialist and a graph specialist should not be optimising the same loop. Enter as many as you like; three is the minimum to be eligible for the overall title, which counts your best three.

A · The parsing track

#### Ingest

Aggregate 3.1M structured log lines into per-endpoint request counts, p50/p95/p99 latency, and error rates bucketed by hour.

The parsing track, and the one most teams should start with.

The naive version reads lines, splits on delimiters, parses timestamps with a date library, and pushes latencies into a growing list per endpoint before sorting each one at the end. Every part of that is a target.

- Custom line parsing beats any general-purpose reader, by a lot

- Percentiles without sorting the whole thing

- Where most teams discover their allocator

- Timestamp parsing is usually the second-biggest cost and nobody expects it

239 MB records 3.1M lines reference 32.05 s timeout 321 s

live data preview · space-delimited, one request per line

| field | type | note |
| --- | --- | --- |
| ts | RFC3339 | microsecond precision, monotonic per shard but interleaved across shards |
| method | enum | 7 values |
| endpoint | string | 1,842 distinct, heavy Zipf tail |
| status | 100–599 | three digits; 5xx counts toward the error rate |
| latency_us | uint32 | microseconds, not milliseconds |

sample rows

| ts | method | endpoint | status | latency_us |
| --- | --- | --- | --- | --- |
| 2026-10-30T18:07:18.569411Z | GET | /api/v2/products | 200 | 8177 |
| 2026-10-30T18:07:21.136274Z | GET | /beta/regions | 503 | 81573 |
| 2026-10-30T18:07:27.963350Z | PUT | /v2/ab_tests | 200 | 6381 |

Three consecutive lines from the sample set, unmodified.

B · The trade-off track

#### Index

Answer 50,000 boolean and phrase queries over a 250 MB text corpus.

Timed as one total wall time, build and queries together. The queries are readable from the start, and the task is to answer these 50,000.

Two phases means two optimisation problems that pull against each other: an index that builds fast is rarely one that queries fast. Choosing where to spend is the whole track.

- Posting-list compression actually pays at this corpus size

- Intersection order is most of the win on boolean queries

- Phrase queries need positions, and positions are where your memory goes

- The queries are readable up front, so you can plan the index around them

256 MB records 50,000 queries reference 56.35 s timeout 564 s

live data preview · queries.txt — one query per line, prefixed by its kind

| field | type | note |
| --- | --- | --- |
| BOOL | 62% of queries | operators are left-associative, no parentheses |
| PHRASE "..." | 38% of queries | adjacent tokens, exact order, stopwords retained |

sample bytes

```
BOOL bond NOT tebaickplea
PHRASE "in the matter of trixclar"
BOOL ceeproust AND drolslemdio
PHRASE "fyslut cliceen surety"
```

Four lines from the seed-1 query file, unmodified. Most words come from a synthetic vocabulary; the legal terms are real. Boolean queries use AND / OR / NOT; phrase queries must respect token positions.

C · The correctness track

#### Join

Parse a hostile multi-megabyte CSV — quoted commas, escaped quotes, embedded newlines, mixed line endings, a UTF-8 BOM on some chunks and not others — join it against a lookup table, emit sorted aggregates.

The fast path here is easy and the edge cases are the actual task.

This is the track where the most submissions will fail the hash gate, and we are telling you that in advance so you budget time for verification rather than discovering it at hour 68.

- Hash join versus sort-merge, on data where the answer isn't obvious

- Quoting rules are in the spec and they are not negotiable

- A parser that's 3× faster and wrong scores zero

- Most disqualifications will happen here

204 MB records 3.32M records reference 14.44 s timeout 144 s

live data preview · records.csv — bytes as they appear on disk

| field | type | note |
| --- | --- | --- |
| id,name,note,region_id,amount | 5 columns | column count is fixed; everything else about the encoding is not |

sample bytes

```
⟨BOM⟩id,name,note,region_id,amount␍␊
1,"Kaphar, Titus","said ""no"" twice",7,1420.00␍␊
2,"Betts","a note that␊spans two lines",7,88.50␊
3,,"",12,0.00␍␊
4,"trailing space ","comma, quote "" and newline␊together",3,-5.25␊
```

From the spec's worked example. Every hostile feature in the file is visible in these five lines. ␍␊ marks CRLF, ␊ marks a bare LF, and ⟨BOM⟩ marks a U+FEFF that appears mid-file at some chunk boundaries.

D · The algorithms track

#### Traverse

A 5M-edge directed weighted graph and 2,000 source-target shortest-path queries.

Most of the work is getting five million edges off disk and into a layout the CPU likes. The searches themselves are short, with a long tail.

Preprocessing is allowed and counted inside your timed run. Queries are drawn at a bounded search distance, so a heavy index has little time to pay for itself: fast parsing, a compact layout and a lean search loop decide this track.

- A fast submission spends much of its time parsing the edge list

- Memory layout beats asymptotics at this size, consistently

- Most searches are short; the tail and the unreachable ones are not

- Contraction hierarchies are legal, but this query mix rarely repays them

92 MB records 5M edges reference 72.69 s timeout 727 s

live data preview · graph.txt — header, then one edge per line: u v w

| field | type | note |
| --- | --- | --- |
| n m | header | node count, edge count |
| u v w | uint32 ×3 | tail, head, weight — not sorted by u |

sample bytes

```
1740512 5000000
1188446 437285 120
64107 865467 170
472413 359585 122
429961 196139 82
```

Node ids are dense and 0-indexed. Weights are uint32. The graph is directed; a reverse edge, where one exists, is listed separately.

03 what you get, and when

### Nothing about the scoring is a surprise

You will not be handed a spec on the day. Everything that decides your score is published before kickoff and frozen. The only thing held back is bytes.

- LIVE · on this page, now · schema and real sample rows for all four tracks · measured reference timings and dataset sizes · the docker build and docker run commands, in illustrative form · the full efficiency coefficient with an illustrative worked example

- FROZEN · Oct 28 · 15:00 UTC · T-48h · all four track specifications · one naive Python reference per track · the complete scoring harness, with its exact command, to run locally · measured reference numbers replace the estimates on this page · nothing about the scoring changes after this moment

- KICKOFF · Oct 30 · 15:00 UTC · T-0 · full datasets released, with their expected output hashes · the live leaderboard opens · clocks start

The final scoring dataset is generated from the same distribution at a seed you have never seen. You can tune. You cannot overfit to specific bytes.

04 how winners are decided

### The efficiency coefficient

Raw speed is a blunt instrument. A submission that runs in 0.4 seconds by loading 40 GB into RAM, pulling in six libraries and sprawling across 4,000 lines has not beaten a 0.9-second solution that fits in 200 MB and reads cleanly. K is how we say that with arithmetic instead of opinion.

#### Everything is a ratio against the reference

Nothing is measured in absolute seconds, and nothing is normalised against the current leader. Every axis is your number divided into the reference implementation's number, on the same machine, in the same batch. A ratio of 1.0 means you matched the naive version. 10.0 means you beat it tenfold.

This matters more than it looks. Because the reference is fixed and published, your score never changes when somebody else submits. The leaderboard reorders around you. Your number does not move.

Every other design we considered — normalising to the field leader, percentile ranking, z-scores — has the property that a stranger's submission at hour 70 silently rewrites your result. That is intolerable in a competition where people are making trade decisions all weekend based on where they stand.

K = (Tref/T) 0.50 × (Mref/M) 0.20 × (Sref/S) 0.15 × (Dref/D) 0.15

T 0.50 + M 0.20 + S 0.15 + D 0.15 = 1.00 · Four factors, four exponents, and the exponents sum to exactly 1.0.

That last property is what keeps K readable: **K = 6** means "six times the reference, all things considered," and nothing else. It is the weighted geometric mean of your four ratios, which is the correct way to combine normalised ratios. An arithmetic combination would let one axis with a wide natural spread dominate the others regardless of weight.

The exponents are also what stop any single axis from steamrolling the rest. A 100× speed win contributes `100^0.50 = 10.00` to the product. A 100× memory win stops at the 50× cap and contributes `50^0.20 = 2.19`. The ordering of importance is enforced by the mathematics, not by a promise in the rules.

#### Four axes, four weights

Each card prints what a win on that axis is actually worth to the product, after its cap. Source size and dependencies can only cost, so their cards print what going over the reference costs instead. That is the formula telling you where to spend the weekend, and it tells you before kickoff.

T · wall time

weight · 0.50 · no cap

50% of the exponent budget

reference 32.05 s

Mean of the middle three of five runs, highest and lowest discarded. Timed by the harness from process exec to exit, not by any clock inside your program.

This is a performance event, so this is the biggest weight and it is not close.

what a win here buys

2× · 1.41

10× · 3.16

100× · 10.00

a 100× win here contributes 10.00 to the product

M · peak memory

weight · 0.20 · capped at 50×

20% of the exponent budget

reference 228 MB

Peak memory of the run, read from the kernel's peak counter after the run ends. Not self-reported, and not sampled: the kernel's high-water mark, which catches transient spikes your own instrumentation would miss.

Buying speed with unbounded RAM is a real strategy and it should cost you something.

what a win here buys

2× · 1.15

10× · 1.58

100× · 2.19

a 100× win here contributes 2.19 to the product

S · source size

weight · 0.15 · capped at 1.0×

15% of the exponent budget

reference 56 lines

Code lines counted by scc after your language's canonical formatter, where one exists. It covers everything in the track's folder (track-a for Track A) at the recorded commit, which is all the build can see, minus registry packages first published before October 1, 2026 by people outside the team, which are spot-checked at scoring. Byte-identical copies of our published generators and references are free, so copying the generator in for build-time training costs nothing.

Line counts do not care how long your names are, so you can write readable code without paying for it. Expect a strong submission to be larger than the reference here. That is the trade, and it is priced rather than punished.

what going over costs

2× · 0.90

10× · 0.71

100× · 0.50

twice the reference here multiplies K by 0.90

D · dependency weight

weight · 0.15 · capped at 1.0×

15% of the exponent budget

reference 1.00

D = 1 + the number of direct non-stdlib runtime dependencies declared in your manifest. The reference uses none, so D_ref = 1 and every dependency is a straight cost.

Importing the crate that already solves half the task is permitted. It is simply not free, and this is where it gets billed. A zero-dependency submission keeps a factor of 1.0 here. One dependency gives 2^-0.15 = 0.90, about 10% off K before any other work.

what going over costs

2× · 0.90

10× · 0.71

100× · 0.50

twice the reference here multiplies K by 0.90

#### Memory is capped at 50×. Wall time is not.

Wall time has no cap. It is never pathological, so a faster program always earns a higher K.

Memory is the axis where a sloppy naive reference produces absurd ratios. Allocating 4 MB where the reference lazily allocates 3 GB gives a 750× memory ratio, a number that would drown out all the actual performance work in the field. The 50× cap keeps the ranking about engineering rather than about finding the one dimension where the reference happened to be sloppiest.

Source size and dependencies can cost you but never pay. The S factor is capped at 1.0, so code shorter than the reference earns nothing extra, and D never drops below the reference's 1, so every dependency is a cost.

raw memory ratio · 750×

after the cap · 50×

contribution at 0.20 · 2.19

#### K becomes points on a fixed curve

K maps to a 0–100 speed score on a logarithmic curve, against an anchor published now and not adjusted afterwards. K = 1 or below, no better than the naive reference, earns nothing. K = 10 earns exactly half. K = 100 or better earns full marks; past that, raw K still breaks ties and decides the highest-K prize on each track.

The curve is logarithmic because performance work is. Going from 1× to 2× is an afternoon. Going from 12× to 24× is the rest of the weekend and a rewrite. A linear scale would make the first hours worth as much as the last thirty, which is not how any of this works.

50 × log10(K) · clamped to [0, 100]

K = 100 100 pts · K = 10 50 pts · K = 1 nothing

K → · every point plotted from the scoring function itself

|  | 1 | 2 | 5 | 10 | 20 | 35 | 60 | 100+ |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| speed score | 0.0 | 15.1 | 34.9 | 50.0 | 65.1 | 77.2 | 88.9 | 100.0 |

#### Run your own numbers

Every ratio below is your number divided into the reference's. Change one and watch what it buys you. Loaded with Team Nightshade's illustrative run.

Efficiency coefficient calculator

T · wall time

^0.50 = 5.004

M · peak memory

^0.20 = 1.416

S · source size

^0.15 = 0.905

D · dependency weight

^0.15 = 1.000

efficiency coefficient · K = 6.41

speed score · 50 × log10(K) · 40.4 / 100

#### Worked example, start to finish

**Team Nightshade**, Track A. An imagined team, scored against the measured reference. Every number that touches their score, in order. Nothing else feeds in.

| axis | reference | Nightshade | ratio | exponent | contribution |
| --- | --- | --- | --- | --- | --- |
| T · wall time | 32.05 s | 1.28 s | 25.04× | 0.50 | 5.004 |
| M · peak memory | 228 MB | 40 MB | 5.70× | 0.20 | 1.416 |
| S · source size | 56 lines | 109 lines | 0.51× | 0.15 | 0.905 |
| D · dependency weight | 1.00 | 1.00 | 1.00× | 0.15 | 1.000 |
| **K = product** |  |  |  |  | **6.41** |

how K assembles · running product

- — 1.00

- T ×5.004 5.004

- M ×1.416 7.087

- S ×0.905 6.414

- D ×1.000 6.414

S is the only step that costs them — 95% over the reference on source size, paid for a 25× win on wall time. Final K = 6.41

K is computed from raw ratios. The table shows each factor rounded to two places; the product cell shows the raw-product K, not the product of the rounded factors. Those differ in the third decimal.

Note the third row. Nightshade wrote 95% more code than the reference and lost points for it, because the optimised parser is genuinely longer than the naive one. They accepted that cost to buy a 25× speedup, which is exactly the trade the coefficient exists to price.

Note also the second row. 5.7× less memory sounds like a lot and contributes 1.416, less than a third of what the time ratio contributed. If they had spent those hours on the parser instead, they would have scored higher. That is the formula telling you where to work, and it tells you before kickoff rather than after results.

now the full score

| final score | raw | weight | points |
| --- | --- | --- | --- |
| Speed score — from K = 6.41 | 40.4 / 100 | 0.65 | 26.26 |
| Judge score — panel average | 82.0 / 100 | 0.35 | 28.70 |
| **Track A total** |  |  | **54.96** |

#### What the judges score

35% of every track total, on a 5-point scale across three criteria, averaged across every judge who reviewed the submission.

65% machine 35% judges

15%

Code quality

Would you merge this? Optimised code is allowed to be dense. It is not allowed to be unexplainable. Clear structure, honest error handling, and a comment wherever the trick is not self-evident. A judge should be able to follow why the fast version is fast.

12%

Technique

Is the approach genuinely considered, or brute force that happened to land? A well-chosen data layout scores above hand-written SIMD bolted onto the wrong algorithm. We are looking for evidence of decisions, not effort.

8%

The optimisation log

A required PERF.md narrating how you got there. "Naive was 41 s. Flat array instead of hashmap took it to 8 s. Manual line splitting got 2.1 s. Tried SIMD for the timestamp parse, made it slower, reverted." Measurements, not adjectives, and each step cites the live-board run ID that measured it. Judges see your run history and check that the cited runs exist and that each claimed change moved in the claimed direction. The failed experiments count — they are often the most interesting part and judges are told to reward them.

Judges see your code, your write-up and your live-board run history. They never see the final measured scores before they submit their forms. The live board is public during the event, gate pass/fail for every team is published by November 4, and full per-axis breakdowns come only with the winners on November 13. That order keeps a final number from colouring the read of the code.

#### The overall championship

The overall Speed Demon title requires at least three tracks entered and is decided on the **geometric mean** of your best three track totals, not the average. Enter all four and your weakest track is dropped.

This is deliberate and it is standard practice in benchmarking. The geometric mean refuses to let one spectacular track carry two mediocre ones. To win overall you have to be good everywhere, which is what "Speed Demon" is supposed to mean.

one spectacular track · 90 / 40 / 40 · 30 dropped · 52.4

good everywhere · 60 / 60 / 60 · 55 dropped · 60.0

geometric mean of the best three · three tracks minimum

05 before K is computed at all

### Three gates

These are pass/fail. A submission that fails any of them scores zero on that track, regardless of how fast it ran. They are checked automatically, and they are checked on every one of the five runs.

1

#### Correctness

SHA-256 of your output file must match the expected hash exactly

Not "close enough", not "same values in a different order" — the spec pins the output format down to line ordering and float formatting precisely so this check can be binary. The full datasets ship with their expected hashes at kickoff, and the reference prints the right hash for any data you generate, so you can verify locally before you ever submit.

If your submission produces the right answer four times out of five, it fails. Nondeterminism is a correctness bug.

2

#### The timeout

hard kill at 10× the reference implementation's time on that track

A container that hangs is a failed run, not a slow one. The exact ceiling is published per track alongside the reference, so you always know how much rope you have.

Track A: 321 s. Track D: 727 s.

3

#### The sandbox

no network · dataset read-only · root filesystem read-only · Scratch space is a 1 GB tmpfs at /tmp. We diff the container filesystem after every run.

Writing precomputed answers into the image, phoning home, or attempting to touch the dataset mount is disqualification from the event — not just the track.

06 we run it, not you

### One machine, and you know its name

Machine variance between a desktop Ryzen and a fanless laptop is comfortably 5×, before you even reach thermal throttling and background processes. So participants never report a number. You submit a Dockerfile. We build it and run it.

#### The box

A Hetzner dedicated server in Falkenstein, Germany (fsn1), with the same Intel Core i5-13500 as the EX44. Dedicated, not shared — no noisy neighbours, no burst credits, no hypervisor surprises. It is the same physical machine for every submission across the entire event.

fsn1 · Hetzner · dedicated

CPU · Intel Core i5-13500 · 14 cores / 20 threads · 6 P-cores @ 4.80 GHz, 8 E-cores @ 3.50 GHz

Cache · 544 KiB L1d · 704 KiB L1i · 11.5 MiB L2 · 24 MiB L3

Memory · 64 GB DDR4 (2 × 32 GB) · non-ECC

Storage · 2 × 1.92 TB datacenter NVMe (U.2) · mdadm RAID1

Network · 1 Gbit/s (disabled during scoring)

OS · Debian 13 (trixie)

Runtime · Docker · cgroups v2

Location · Falkenstein, Germany

#### Two things we change before scoring starts

no_turbo=1

##### The CPU governor goes to performance and turbo is disabled

Turbo alone introduces 10–20% variance depending on how long the box has been idle, which is more than the gap between third and seventh place. Sustained clocks, deterministic results. You are optimising against a fixed 2.5 GHz base rather than a boost ceiling that depends on the weather.

--cpuset-cpus=0-11

##### Containers are pinned to the six P-cores only

This is a hybrid CPU. Mixing P-cores and E-cores in one run makes results depend on how the scheduler felt that second, and on Raptor Lake the P/E performance gap is large enough to swamp real optimisation work. So you get six physical performance cores, twelve hardware threads, and nothing else. Design your parallelism for exactly that. Spawning 20 workers will hurt you.

#### The run command

This is the shape of a scored run, and it is illustrative. The exact harness command and its timing wrapper ship with the harness on October 28.

scoring harness · illustrative

```
# build each track folder on its own, on the build machine: offline, at most 10 minutes and a 2 GB image
docker build --network=none -t speeddemon/$TEAM-$TRACK:$COMMIT track-$TRACK

# pre-warm the dataset into page cache so we measure your code, not the NVMe
vmtouch -t /srv/speeddemon/data/track-$TRACK

# the scored run — repeated 5x, serialized, nothing else running on the box
docker run --rm \
  --cpuset-cpus="0-11" \
  --cpuset-mems="0" \
  --memory=8g --memory-swap=8g \
  --pids-limit=512 \
  --network=none \
  --read-only \
  --tmpfs /tmp:rw,size=1g,exec \
  --ulimit nofile=8192:8192 \
  -v /srv/speeddemon/data/track-$TRACK:/data:ro \
  -v /srv/speeddemon/out/$RUN:/out:rw \
  -e TRACK=$TRACK \
  speeddemon/$TEAM-$TRACK:$COMMIT

# memory: the kernel's peak counter for the run, read after it ends
```

--cpuset-cpus="0-11"

Twelve threads on six physical P-cores. Hard. Twenty workers will cost you more than they buy.

--memory=8g --memory-swap=8g

Equal values disable swap **entirely** rather than allowing an equal amount of it — the Docker behaviour people most often get wrong. There is 32 GB of swap on the host and none of it is available to you. Exceeding 8 GB is an OOM kill and a failed run, not a slow one.

--network=none · At build time and at run time. Vendor everything into the image.

--read-only + --tmpfs /tmp · Read-only root with 1 GB of scratch at /tmp if you need spill space.

-v ...:/data:ro · The dataset, mounted read-only. Filenames are fixed by the track spec.

peak memory

Read from the kernel's peak counter for the run, after it ends. Not sampled and not self-reported, so it catches transient spikes your own instrumentation would miss.

#### Your Dockerfile contract

Five rules. Nothing else about your image matters to us.

- 1 · Read from /data · Mounted read-only. The filenames are fixed by the track spec. Do not assume any path outside it exists.

- 2 · Write to /out · Exactly one output file, named by the spec. Anything else you leave behind gets flagged by the filesystem diff.

- 3 · Exit 0 · Any non-zero exit is a failed run. Handle your own errors; the harness will not interpret them for you.

- 4 · Build offline · We build with --network=none, on a separate build machine, not on the scoring box. If your Dockerfile fetches anything — apt, cargo, pip, a curl — the build fails and so does your submission. Compiling for the native CPU is fine: when a track asks for it (-march=native, -C target-cpu=native and the like), we build that track on the scoring box itself, so native means the scoring CPU, a Core i5-13500 with AVX2 and no AVX-512. PGO training runs wherever the build runs; its profiles carry over, its timings do not.

- 5 · Build FROM a published toolchain · Every FROM names one of the published toolchain images, each pinned by digest. COPY --from and RUN --mount may only use earlier stages of your own Dockerfile, ADD may not fetch URLs, and # syntax= directives and cache, secret or ssh mounts are refused. Builds are capped at 10 minutes and images at 2 GB.

Dockerfile — the shape we expect

```
# build stage: one of the published toolchain images
FROM rust:1-slim-trixie@sha256:1a939748a7cbab172de91a014e3804a97af1394a8c844faa67fce9401c4e40fb AS build
COPY . /src
WORKDIR /src
RUN ./build.sh          # no network available here

# runtime stage: slim, and also from the published list
FROM debian:13-slim@sha256:62bd423aeee912639ca8ee681c2b762cb3e25137e0a92b20be455898a9f034ad
COPY --from=build /src/run /app/run
# reads /data, writes /out, exits 0
ENTRYPOINT ["/app/run"]
```

The allowlist, with every digest, is at <a href="/allowlist.json">speeddemon.dev/allowlist.json</a>: debian:13-slim, python:3.13-slim, rust (plus a rust:1-trixie build with llvm-tools, for PGO), golang, gcc 15 and 16, Eclipse Temurin 25 (JDK and JRE), GraalVM native-image 25, Node 22 and 24, .NET 10, Swift 6.2 and Haskell 9. Need another? Ask in Discord by October 28. Build time is not scored, only the run: if you want to spend six of your ten minutes on profile-guided optimisation, that is free.

#### Timing protocol

5 · runs per submission

2 · discarded — highest and lowest

3 · middle runs, meaned, become your T

5% · spread above this is re-run automatically

2% · reference drift that throws out a whole batch

We run the reference implementation before and after every scoring batch. If the reference drifts by more than 2% across a batch, that batch is thrown out and rescored. That calibration is what lets us say these numbers are comparable and mean it — and because your score is a ratio to the reference rather than an absolute time, mild drift partly cancels out anyway. Runs are strictly serialized. Nothing else executes on the box during scoring.

#### The live leaderboard

During the 72 hours you can push to a submission branch and get an automated run against the full public dataset released at kickoff, on the same Falkenstein machine. Your latest push runs, at most once per 5 minutes per team, capped at three times the reference's time, and a team has at most one run in the queue. Those results go straight onto a public board so you can watch the field move and know whether your last change was worth it.

Your repo can stay private until code freeze. To use the board, install the Speed Demon Board GitHub App (https://github.com/apps/speed-demon-board) on that repo. It gets read-only access to the repos you pick and nothing else, through tokens that last an hour. Every run gets an ID, and the steps in your PERF.md cite those IDs.

Final scoring runs on the holdout dataset after code freeze, and that is the only set that counts. The gap between your board position and your final position is the price of overfitting.

07 the receipts

### What you submit

Per track. A repo we can build without asking you a single question.

$ ls -la ./track-a · 6 entries · 5 required

- Dockerfile · meets the five rules · Builds offline from a clean checkout.

- PERF.md · worth 8% of your total · The optimisation log. Every step from naive to final, citing the live-board run ID that measured it. The part judges enjoy most.

- README.md · prose · The approach in plain prose, your dependency list if any, and what you would do with another 24 hours.

- .speed-demon.toml · machine-readable · Track letter, team name, language. The dependency count comes from your manifest; the declared count is used only when there is no manifest.

- track-a/ … track-d/ · OSI licence, public at freeze · One public GitHub repository per team, one folder per track you entered, on the speed-demon branch.

- demo.mp4 optional · under 3 min · Optional and not scored, but the good ones end up in the results write-up and on the archive page.

08 pre, during, after

### Timeline

All times UTC. The hack runs across Halloween weekend, which is not an accident.

Pre-event · 01

October 2

#### Registration opens

Join the Discord. Start arguing about languages early.

Pre-event · 02

October 26

#### Find your team

1–4 people per team. Solo entries welcome. Teams can change until code freeze: resubmit the team form, the latest response counts.

Pre-event · 03

October 28 · 15:00 UTC

#### Specs, references and harness published

All four track specifications, one naive Python reference per track, and the complete scoring harness with its exact run command and timing wrapper. Measured reference numbers replace the estimates on this page. Run it locally before kickoff. Nothing about the scoring changes after this date.

The hack · 04

October 30 · 15:00 UTC

#### Kickoff

Full datasets and their expected hashes published. Live leaderboard opens. Clocks start.

The hack · 05

October 31

#### Midnight, and nothing is haunted but the cache

Standings posted. No costumes, no call, no ceremony — you have a profiler open and we respect that.

The hack · 06

November 2 · 15:00 UTC

#### Code freeze

Repositories locked at the recorded commit. Scoring queue begins.

Post-event · 07

November 2–4

#### Scoring runs

Every submission built and run five times on the holdout dataset. Gate pass/fail for every team is published by November 4, including exactly which gate a failing submission hit. Full per-axis breakdowns wait for the winners on November 13.

Post-event · 08

November 3–12

#### Judging window

Each submission reviewed independently by multiple judges on structured forms. Written feedback to every team, not just the winners.

Post-event · 09

November 10

#### Write-Up side quest closes

Deadline for the $200 write-up prize. Two winners, $100 each.

Post-event · 10

November 13

#### Winners announced

The overall Speed Demon and podium, the highest K on each track, side quest results, full per-axis breakdowns for every team, and the full scoring dataset published so anyone can reproduce the board. The teardown of every deliberately terrible decision in the reference implementation follows in the results write-up.

09 five ways to win

### Prizes

$1,800 total.

- $800 · Speed Demon · Highest geometric mean of your best three track totals, from three or more tracks entered. Fast everywhere, not fast once.

- $400 · Runner-up · Second on the overall board. Same bar, narrower margin.

- $200 · Write-Up side quest · $100 × 2. Publish a post about your optimisation journey — the profiler reading you misread, the rewrite that made it slower, the two-line change that halved it. Post it on a public blog (dev.to, Medium, Hashnode or your own site; not a README or a GitHub Pages repo), mention Hackathon Raptors and link your repo, by November 10. Judged on insight, not audience size.

- $200 · Third place · Third on the overall board.

- $200 · Highest K on each track · $50 × 4. The largest efficiency coefficient recorded on each of Tracks A to D. Specialists welcome: you do not need three tracks for this one.

total pool · $1,800

10 ten of them

### Rules

- 01 · Any language · If it builds in a Linux container offline and runs on the scoring box's x86-64 CPU (Core i5-13500: AVX2, no AVX-512), it is eligible. Assembly is eligible. Your own compiler is eligible. The coefficient does not care what you wrote it in, and neither do we.

- 02 · Dependencies are allowed and priced · This is not a standard-library purity contest. Import what you want — the D axis charges you for it. What is banned, per track, is the library that solves the entire task: no ready-made search engine in Track B, no analytics dataframe library in Tracks A or C. Those are listed explicitly in each track spec, by name. Vendored code counts toward your source size unless it is a registry package first published before October 1, 2026 by someone outside your team.

- 03 · Correct output or no score · SHA-256 must match. The specs pin down ordering and formatting so this is unambiguous. Verify locally with check.py and the published hash before you submit.

- 04 · No precomputed answers · The scoring dataset is generated at a seed you have never seen. Shipping cached results, or code that recognises the input and short-circuits, is disqualification from the event.

- 05 · New code only · All of your own solution code is written during the 72-hour window. Reading up on algorithms, sketching designs, preparing your environment, and building on our public starter template are all fine and expected. Committing your own solution code before kickoff disqualifies the submission.

- 06 · AI tools are expected · Claude Code, Cursor, Codex, Copilot, local models — bring whatever you use. We do not gatekeep on how the code was produced. We gatekeep on whether it survives five runs on our hardware and whether PERF.md shows a human understood what was happening. Both are hard to fake, which is rather the point of measuring instead of judging.

- 07 · Team size 1–4 · Solo entries are welcome. Find teammates on the Raptors Discord before or during.

- 08 · Source public at code freeze · Public GitHub repo under an OSI-approved licence, made public within one hour after code freeze, at the commit we recorded. Until then it can stay private; to use the live board, install the Speed Demon Board GitHub App (https://github.com/apps/speed-demon-board) on that repo. It can only read the repos you pick. Anonymous usernames are fine; the team must be reachable for written follow-up during judging.

- 09 · The harness is the referee · Its numbers are final. If you believe a run was mis-measured you have 48 hours after publication to request a re-run, and we will do it. What we will not do is accept a timing from your machine as evidence. Attacking the harness, the scoring box, the board or another team disqualifies the team and bans it from future Hackathon Raptors events.

- 10 · The formula does not change after kickoff · Every weight, cap, anchor and gate on this page is frozen from October 28. If we find a flaw in the coefficient during the event, it gets documented publicly and fixed for the next edition. It does not get patched mid-competition.

11 find your track

### Who this is for

If you have ever opened a flame graph at 1am out of curiosity rather than obligation, this is your hackathon.

| who you are | A | B | C | D |
| --- | --- | --- | --- | --- |
| Systems engineers Cache lines, branch prediction, allocator behaviour. The tracks where memory layout beats cleverness. |  |  |  |  |
| Data engineers You have parsed worse CSVs than ours at work, and you have opinions about how the pipeline should have been written. |  |  |  |  |
| Search and database folk Posting lists, skip pointers, join strategies. Track B is the one you have been mentally designing for years. |  |  |  |  |
| Algorithms people Competitive programming background, and finally a contest where the constant factor counts as much as the complexity class. |  |  |  |  |
| Anyone with something to prove Four tracks, one reference implementation, and a public number at the end of it. No pitch, no slides, no interpretation. |  |  |  |  |

12 who reads your code

### Judges

The 35% that is not measured is decided by engineers and technical leaders who build and run production systems at scale, from low-latency search and payments to site reliability and cloud infrastructure.

Judges review submissions on structured forms, independently, with each team's live-board run history beside the code. They never see the final measured scores before they submit. That order matters: it stops a final number from colouring the read of the code, and it means the two parts of your score come from two different places.

the split · 35% · decided by people, on structured forms, before the final scores are out

- Karthik Chandrasekaran · Principal Software Engineering Manager · Microsoft · Directs the M365 Copilot Agent Query Matching Platform at Microsoft: vector and semantic indexing pipelines at tens of thousands of QPS. Previously led low-latency card enrollment infrastructure at Apple Pay and architected high-concurrency identity and auth systems at Capital One, delivering 10x performance gains and sub-10ms response times through distributed caching and circuit breaking. Has spent 18+ years on core infrastructure, enterprise databases and low-latency search. · LinkedIn: Karthik Chandrasekaran (https://www.linkedin.com/in/tokarthikc/)

- Rushikesh Hayatnagarkar · Senior Product Manager · Amazon · Hands-on AI builder specialising in technical infrastructure, logistics and autonomous multi-agent systems. Judges through two lenses, engineering execution and real-world product viability, and looks past the demo at architectural integrity, code feasibility and scalability. Draws on custom Python orchestration and high-scale platforms to give feedback that turns prototypes into production-ready systems. · LinkedIn: Rushikesh Hayatnagarkar (https://www.linkedin.com/in/rushikeshhayatnagarkar/)

- Baskaran Jeyarajan · Senior Cloud Platform, SRE and DevOps Engineer · Has spent 20+ years in enterprise technology, specialising in cloud infrastructure, production reliability and performance engineering. Troubleshoots production systems through metrics, logs, latency and failure patterns, and weighs trade-offs beyond the numbers: where the bottleneck is, and why implementations behave differently. Author or co-author of 11 research publications and reviewer of 38 conference papers. · LinkedIn: Baskaran Jeyarajan (https://www.linkedin.com/in/baskaran-jeyarajan-28a487112/)

- Karthik Kapula · Enterprise Automation and AI Architect · ICS Global Soft · Has spent 15+ years across scalable systems, databases, cloud infrastructure, APIs, CI/CD, observability and performance optimisation. Led enterprise solutions that process high-volume workloads while improving reliability and throughput. A 3x UiPath MVP and IEEE Senior Member, with experience in technical judging and peer review. · LinkedIn: Karthik Kapula (https://www.linkedin.com/in/karthik-kapula-a84109179/)

- Lu Zhang · Senior Product Leader · Has led systems spanning transaction orchestration, payments, inventory, pricing, risk and fulfilment at TikTok Shop and other global logistics and commerce platforms. Expertise covers distributed workflows, state-machine correctness, API contracts, capacity constraints, failure recovery and performance under peak demand. Judges whether an optimisation is measurable, well reasoned, reproducible and maintainable, not merely fast. · LinkedIn: Lu Zhang (https://www.linkedin.com/in/zoezoezhang/)

- Ohad Assulin · Computer Science Lecturer · Reichman University · AI entrepreneur and product leader. Previously founded and sold an AI trading startup, served as Chief Data Scientist at HPE, and was a member of the W3C HTML Working Group. Holds 12 patents across several areas of computer science, with expertise in AI, data science, software development and product innovation. · LinkedIn: Ohad Assulin (https://www.linkedin.com/in/mrohad/)

- Prasanna Balasubramani · Staff Software Engineer · SoFi · Specialises in distributed monetisation systems, low-latency infrastructure and high-throughput transaction processing. In engineering leadership roles at SoFi and Amazon (AWS EC2 and Outposts Pricing, Ads Tech), has architected engines where millisecond performance, memory footprint and database throughput directly govern scale. Looks for systems-level optimisation, algorithmic efficiency, architectural resilience and genuine mechanical sympathy. · LinkedIn: Prasanna Balasubramani (https://www.linkedin.com/in/prasanna-r-balasubramani/)

- Victoria Drazdova · Founder and Head Coach · VSA · Sports technology entrepreneur and researcher, and founder of VSA, a global digital training platform. Leads the development of technology-enabled products across AI, computer vision and athlete performance analysis, with hands-on experience scaling a multi-market online platform. Has published peer-reviewed research on AI and technology-assisted sports performance and collaborates with academic researchers on computer vision, biomechanics and human-movement analysis. · LinkedIn: Victoria Drazdova (https://www.linkedin.com/in/victoria-drazdova/)

- Ramanjaneyulu Mallisetty · Software Architecture and Engineering Leader · MeridianLink · Has spent 25+ years in enterprise software, distributed systems and applied research, from signal processing to large-scale software architecture at CA Technologies (Broadcom) and MeridianLink. Holds seven U.S. patents and has written peer-reviewed and industry publications. As a judge, focuses on technical innovation, architecture, engineering quality and scalability. · LinkedIn: Ramanjaneyulu Mallisetty (https://www.linkedin.com/in/ramanjaneyulu-malisetti-98901416/)

- Sushrutha Sreevathsa · Site Reliability Engineer · Yahoo · Technology leader with 13+ years in large-scale production systems, infrastructure reliability, observability, incident management and performance-focused operations. At Yahoo, has worked on globally distributed systems behind high-traffic internet services, across SRE, DevOps, monitoring architecture and operational analytics. Background in evaluating reliability, scalability, architecture and real-world performance trade-offs in complex production environments. · LinkedIn: Sushrutha Sreevathsa (https://www.linkedin.com/in/sushsreevathsa/)

also on the panel

- Narasimha Reddy Annapareddy, LinkedIn (https://www.linkedin.com/in/narasimha-annapareddy/) · Manager · NetSuite

- Mohit Baid, LinkedIn (https://www.linkedin.com/in/mohitlinked/) · Senior Software Engineer · Google

- Yevhen Bozhenko, LinkedIn (https://www.linkedin.com/in/ybozhenko/) · QA Automation Engineer · MedeAnalytics

- Tenny Enoch Devadas, LinkedIn (https://www.linkedin.com/in/tenny-enoch-devadas/) · Principal Engineer and Architect · Fidelity Investments

- Harish Gaggar, LinkedIn (https://www.linkedin.com/in/harish-gaggar-76552722/) · Technology leader, AI and data infrastructure · Intuit

- Naresh Babu Goolla, LinkedIn (https://www.linkedin.com/in/naresh-goolla-1b69a228/) · Enterprise systems architecture · Truist Bank

- Nail Iarmukhametov, LinkedIn (https://www.linkedin.com/in/restaq/) · Lead DevOps and Platform Engineer

- Anuj Kapoor, LinkedIn (https://www.linkedin.com/in/anuj-kapoor-38b6a899/) · Senior Software Engineer · Microsoft

- Aniket Khandelwal, LinkedIn (https://www.linkedin.com/in/aniket-khandelwal/) · Founding Engineer · WisdomAI

- Nikhil Kokal, LinkedIn (https://www.linkedin.com/in/nikhilkokal/) · Distributed systems at Disney, Google and Salesforce

- Janardhana Naidu Kola, LinkedIn (https://www.linkedin.com/in/janardhana-kola/) · Director of Business Intelligence · ADP

- Pratik Koshiya, LinkedIn (https://www.linkedin.com/in/pratik-k-05880013/) · Enterprise security and Zero Trust

- Pavani Totli Kuruba, LinkedIn (https://www.linkedin.com/in/pavani-totli-kuruba/) · Senior Program Manager II · lululemon athletica

- Dr. Pramodini Girish Mahendrakar, LinkedIn (https://www.linkedin.com/in/pramodini-girish-mahendrakar-88799515/) · Senior Automotive Systems Engineer

- Rohit Mishra, LinkedIn (https://www.linkedin.com/in/rohit-mishra-bb83b932/) · Google

- Nissan Modi, LinkedIn (https://www.linkedin.com/in/nissanmodi/) · Staff Software Engineer · Coinbase

- Aswin Nagarajan, LinkedIn (https://www.linkedin.com/in/aswin-nagarajan-a957b8149/) · Research Engineer, model post-training infrastructure

- Surajit Paul, LinkedIn (https://www.linkedin.com/in/surajitpaul/) · Lead Platform Engineer and AI Solutions Architect

- Fedir Plotnikov, LinkedIn (https://www.linkedin.com/in/fplotnikov/) · Cloud infrastructure and platform engineering · BrightSteps Rewards

- Andrei Podymov, LinkedIn (https://www.linkedin.com/in/andrei-podymov/) · Technical founder and CTO, leads Keyabl

- Ruslan Shaydullin, LinkedIn (https://www.linkedin.com/in/ruslan-shaydullin-80030a28b/) · Lead DevOps Engineer · T1 Innotech

- Itendra Kumar Singh, LinkedIn (https://www.linkedin.com/in/itendra-singh-3553a174/) · End-to-end validation architectures

- Vladyslav Telychko, LinkedIn (https://www.linkedin.com/in/vladislav-telichko/) · Lead Frontend Developer · Breeze

- Swadhin Ajay Thakkar, LinkedIn (https://www.linkedin.com/in/swadhinthakkar/) · Senior Software Engineer · Microsoft

- Oleg Trenin, LinkedIn (https://www.linkedin.com/in/oleg-t-a6480629b/) · Senior Software Engineer · X-FLOW

- Robert Vetter, LinkedIn (https://www.linkedin.com/in/2005-robert-vetter/) · Founding Engineer · Certus AI

- Abhinav Wagle, LinkedIn (https://www.linkedin.com/in/abhinavwagle1/) · Engineer, data and distributed systems · Together AI

The Speed Demon panel is drawn from the Hackathon Raptors judging pool, which across recent events has included staff and principal engineers from:

Apple, Microsoft, Amazon Web Services, Meta, SpaceX, Oracle, Uber, IBM.

13 the short answers

### FAQ

01 Do I have to enter all four tracks?

No. Enter one if you want. The per-track prizes, one for the highest K on each track, are open to anyone. Only the overall championship requires three or more, because “Speed Demon” should mean broadly fast rather than narrowly lucky. It counts your best three.

02 My laptop is much faster than an i5-13500. Does that hurt me?

Not at all — you never submit a timing. Build and tune wherever you like. Just be aware that optimisations tuned to your cache sizes may not transfer, and that you get twelve threads on six physical cores at fixed clocks. The harness ships on October 28 so you can run locally under the same limits.

03 Can I use a library that does most of the work?

Sometimes. Each track spec bans the specific class of library that trivialises that task: the search engine, the dataframe. Everything else is permitted, and the D axis charges you 15% weight for the privilege. A zero-dependency submission starts with a real advantage there. Vendored code also counts toward your source size, unless it is a registry package first published before October 1, 2026 by someone outside your team.

04 What if my optimised code is longer than the reference?

It almost certainly will be, and that is priced into the formula rather than punished by it. In the illustrative worked example above, the optimised approach was 95% larger and still reached K = 6.41. Source size is 15%, wall time is 50%. Spend lines where they buy you time.

05 Won't everyone just write it in C or Rust?

Many will. K is a ratio against a naive Python reference, so the language you choose is part of your result, and nothing in the formula is built to cancel that out. Wall time is measured end to end, startup included. Managed runtimes can cut their startup with a build-time AOT cache or GraalVM native-image. Assembly, incidentally, tends to lose badly on source size.

06 Can I train at build time?

Yes. Profile-guided optimisation, a JVM AOT cache (JEP 514) and GraalVM native-image may all train on generator output inside the build. /data is not mounted at build time, and shipping cached outputs is banned under rule 04.

07 What happens if my container gets OOM-killed?

That run fails, and the gates are checked on every run, so you score zero on the track. Memory discipline is part of the event.

08 Can I submit multiple attempts?

Yes, against the full public dataset released at kickoff, once per 5 minutes per team, until code freeze. The final scoring run uses whatever is at your recorded commit at 15:00 UTC on November 2.

09 How is source size measured, exactly?

Code lines as `scc` counts them, which leaves out blank lines and comments, after your language's canonical formatter where one exists. It covers everything in that track's folder (`track-a/` for Track A) at the recorded commit, minus registry packages first published before October 1, 2026 by people outside your team, which we spot-check at scoring. That folder is all the build can see, so your other tracks never count toward it, and code shared between tracks counts in every folder that copies it. There is no identifier normalisation: line counts do not care how long your names are, so write readable code. The exact measurement script ships with the harness on October 28.

10 Why the geometric mean for the overall title?

Because the arithmetic mean lets one enormous result carry a mediocre record. It is the standard choice for aggregating normalised benchmark ratios, for exactly this reason. A team scoring 90/40/40 should not beat a team scoring 60/60/60, and under the geometric mean it does not.

11 Why cap memory at 50× and not wall time?

Because the reference implementation is naive by design, and on memory naive is very naive. Without a cap, the team that noticed the reference allocates 3 GB where 4 MB would do could post a memory ratio in the hundreds and win on a single observation. The cap makes that a strong result rather than a decisive one. Wall time is never pathological in that way, so it has no cap, and a faster program always earns a higher K. Source size and dependencies are capped at 1.0: they can cost you but never pay.

12 Is this just competitive programming?

No. There is no clever-trick puzzle to spot, and the algorithm is usually obvious within an hour. The difficulty is engineering — memory layout, I/O strategy, parallelism, and the discipline to measure before changing things. Competitive programmers do well here, but so do people who have spent years making production systems faster.

13 Where do the datasets come from?

Synthetic, generated from published generators that ship with the harness so you can produce as much extra training data as you want. The scoring set uses the same generators at an unpublished seed. Realistic distributions, no personal data, no licensing questions. The full public datasets and their sha256 hashes are on the board page: speeddemon.dev/board/#datasets (https://speeddemon.dev/board/#datasets).

14 Can I use multiple threads?

Yes, up to the twelve you are given. Parallelism is a legitimate and expected strategy. Just remember that the memory cap is shared across all of them and that oversubscribing twelve threads with twenty workers costs you more than it buys.

15 What if two teams tie?

Ties on the final total are broken by raw K, then by wall time, then by earliest submission commit.

16 Can I use io_uring?

No. Docker's default seccomp profile blocks it, and the harness does not change that profile. mmap works.

72 hours. one number.

### Every millisecond is a soul

Four tracks. One reference implementation to beat. One machine in Falkenstein that does not care how good your idea sounded.

Register (https://tally.so/r/q4redG) Submit your repo (https://tally.so/r/Ar7g9z) Join the Discord (https://discord.com/invite/xfYPDZYqeh)

1.28s · what the board rewards

## "Explain it plainly" drawer (https://speeddemon.dev/#brief)

What this actually is · Short Medium The whole thing · Close

How much do you want?

- Short 20 seconds

- Medium 1 minute

- The whole thing 3 minutes

You pick a track. **Or several.**

We hand you a task, a dataset, and a program that already solves it — **correctly, and slowly, on purpose.**

You write it again. Faster. **Any language.** · We run both on **our** machine, not yours, and compare. That is the whole event.

the same job, twice

the reference

32.05 s · 228 MB

a strong submission

1.28 s · 40 MB

25.0× the wall time, 5.7× the memory, identical output

Everything else on this page is the detail behind these sentences.

You pick a track. There are four. Enter one, or enter all of them.

Each track gives you three things: a **task specification**, a **dataset**, and a **reference implementation** that is correct and deliberately slow.

You solve it again — any language, any approach — and ship it with a Dockerfile.

We build that Dockerfile offline on a separate build machine, then run it ourselves on one machine in Falkenstein, **5 times**, cores pinned and turbo off. Your laptop's numbers are for your own iteration. Ours are the ones on the board.

what happens to your code

- you · 01 · Pick a track · Spec, dataset, and a slow reference

- you · 02 · Write it faster · Any language, any approach

- you · 03 · Ship a Dockerfile · So it builds the same way for us

- us · 04 · We build and run · 5 runs, cores pinned, turbo off

- us · 05 · 3 gates · Correctness · The timeout · The sandbox

- us · 06 · One coefficient · 65% of your track total

Nothing you measure on your own machine is used. We re-run everything.

Your output has to **hash identical** to the reference, byte for byte. Three gates decide whether you get a score at all.

If you pass, you get one number: how much faster and leaner you were than the reference. That is **65%** of your score. Human judges read your code for the other **35%**.

how a track total is built

65% · measured by machine

35% · read by people

A submission nobody can read is not an engineering achievement — which is what the other 35% is for.

Win a track by having the best total on it. Win the championship by being fast across **three or more**. Your best three count.

Everything else on this page is the detail behind these sentences.

You pick a track. There are four. Enter one, or enter all of them.

Each track gives you three things: a **task specification**, a **dataset**, and a **reference implementation** that is correct and deliberately slow. The reference is published before kickoff and **it does not change**.

You solve it again. Any language, any approach, any trick you can defend — and you ship it with a Dockerfile, so it builds the same way for us as it does for you.

what happens to your code

- you · 01 · Pick a track · Spec, dataset, and a slow reference

- you · 02 · Write it faster · Any language, any approach

- you · 03 · Ship a Dockerfile · So it builds the same way for us

- us · 04 · We build and run · 5 runs, cores pinned, turbo off

- us · 05 · 3 gates · Correctness · The timeout · The sandbox

- us · 06 · One coefficient · 65% of your track total

Nothing you measure on your own machine is used. We re-run everything.

We build that Dockerfile offline on a separate build machine, then run it ourselves on one machine in Falkenstein: **5 runs**, cores pinned, turbo off, one submission at a time so nothing competes for cache. Highest and lowest are discarded. **Self-reported timings are never accepted.**

Before anything is measured, three gates run. Your output has to **hash identical** to the reference, byte for byte. Fail a gate and you score **zero** on that track — and we publish which gate you failed.

before anything is measured

- 1 · Correctness · SHA-256 of your output file must match the expected hash exactly

- 2 · The timeout · hard kill at 10× the reference implementation's time on that track

- 3 · The sandbox · no network · dataset read-only · root filesystem read-only

Fail one and you score zero on that track — and we publish which one you failed.

If you pass, we measure four things: wall time, peak memory, source size and dependency weight. Each becomes a ratio against the reference, and the ratios **multiply** into one number — the **efficiency coefficient**.

Wall time carries the most weight, because this is a performance event, and it has no cap. Memory is capped at **50×** so one lucky dimension cannot carry a submission, and source size and dependencies can cost you but never pay.

the four things we measure

- T · wall time · 0.50

- M · peak memory · 0.20

- S · source size · 0.15

- D · dependency weight · 0.15

Each is a ratio against the reference. Wall time has no cap, memory stops at 50×, and source size and dependencies can cost but never pay.

Because the reference is fixed, **your number never moves when somebody else submits.** The board reorders around you. Your score does not.

The coefficient is **65%** of your track total. Judges read your code, your write-up and your live-board runs for the other **35%**, on structured forms, **without seeing the final scores**.

how a track total is built

65% · measured by machine

35% · read by people

A submission nobody can read is not an engineering achievement — which is what the other 35% is for.

During the 72 hours you can push and get an automated run against the **full public** dataset released at kickoff, once every 5 minutes. Final scoring runs on a **holdout** dataset from the same generator at a seed you have never seen. The gap between your board position and your final one is the price of overfitting.

two boards, one that counts

during the 72 hours · The live board

- Push to a submission branch

- Automated run on the **full public** dataset

- One run per team per 5 minutes

- Public, so you can watch the field move

practice

after code freeze · The scoring run

- Whatever is at your recorded commit

- Run on the **holdout** dataset

- Five runs, highest and lowest discarded

- Per-axis breakdowns published with the winners

this is your score

The gap between your board position and your final one is the price of overfitting.

Afterwards we publish every team's per-axis breakdown and the scoring dataset, so **anyone can reproduce the board**.

Win a track by having the best total on it. Win the championship on the **geometric mean** of your best three tracks, from three or more entered: fast everywhere, not fast once.

Everything else on this page is the detail behind these sentences.

---

# Page: Start (plain English) (https://speeddemon.dev/start)

START in plain english

## So, what is this?

You pick a track. **Or several.**

We hand you a task, a dataset, and a program that already solves it. **Correctly, and slowly, on purpose.**

You write it again. Faster. **Any language.** · We run both on **our** machine, not yours, and compare. That is the whole event.

How to start → (https://speeddemon.dev/kickstart) See the live board → (https://speeddemon.dev/board) Submit your repo → (https://tally.so/r/Ar7g9z)

WHAT WINS the four things

### What makes a great project

You are racing our slow reference program. Beat it on four things, and mostly, just be fast.

the four things we measure

- T · wall time · 0.50

- M · peak memory · 0.20

- S · source size · 0.15

- D · dependency weight · 0.15

Each is a ratio against the reference. Wall time has no cap, memory stops at 50×, and source size and dependencies can cost but never pay.

- 01 · Speed · Make it run fast. This is half your score, and there is no ceiling. Faster always wins.

- 02 · Memory · Do not hog RAM. Buying speed with huge memory should cost you, so it does.

- 03 · Short code · Keep it small. Long code can cost you a little. It can never earn you points.

- 04 · Few libraries · Every library you lean on costs a little. Zero extra libraries is free.

Those four are the machine score, about two thirds of your total. Human judges read your code and your write-up for the rest.

Want the exact formula behind this? It is all in the Scoring section ↗ (https://speeddemon.dev/#coefficient).

COMPARE the shape of a winner

### A strong entry vs a weak one

Strong

- Runs many times faster than the reference

- Light on memory

- Tight, readable code

- Few or no extra libraries

Weak

- Barely faster, or even slower

- Eats memory to buy that speed

- Sprawling, bloated code

- A long list of dependencies

RULES the only hard rules

### Three things you cannot skip

Pass these and you get a score. Fail one and that track scores zero, and we tell you which.

- Correct · Your answer must match the reference exactly, byte for byte.

- In time · Finish within ten times the reference's time.

- Sealed · No internet. Read the data, write one file, and that is it.

TRY your move

### Try it. It is free, and any language works.

Use the template, push to your branch, and your score shows up on the live board within minutes. No fee, no lock-in, and no maths degree required.

How to start → (https://speeddemon.dev/kickstart) See the live board → (https://speeddemon.dev/board) Submit your repo → (https://tally.so/r/Ar7g9z)

---

# Page: Kickstart (setup guide) (https://speeddemon.dev/kickstart)

KICKSTART the starting pack

## Your first run, in five steps

The template is a working entry for all four tracks: the organisers' naive reference, packaged exactly the way the live board builds and runs it. Clone it, check it end to end, then replace the reference with your own code and watch the board move.

Use the template ↗ (https://github.com/hackathon-raptors/speed-demon-starter) See the board → (https://speeddemon.dev/board)

01 five steps

### From zero to on the board

- 01 · Use the template · Open speed-demon-starter (https://github.com/hackathon-raptors/speed-demon-starter) and press **Use this template** to make your own repo. It can stay private until code freeze.

- 02 · Install the board app · Install the Speed Demon Board app (https://github.com/apps/speed-demon-board) on that repo. It gets read-only access to the repos you pick and nothing else.

- 03 · Register your team · Fill the submission form (https://tally.so/r/Ar7g9z) with your repo URL and the tracks you are entering. One form per team.

- 04 · Push to speed-demon · The `speed-demon` branch is the default, so a plain `git push` does it. The board builds every `track-*` folder and runs it once on the full public dataset. Your result appears on the board within minutes.

- 05 · Make it fast · Log every step in the track's `PERF.md` with the board run ID that measured it. At code freeze (Nov 2, 15:00 UTC) make the repo public under an OSI licence.

One repo per team holds all your tracks, as `track-a` to `track-d`. Delete the folders of the tracks you are not entering. A folder you leave unchanged is the reference itself, so the board reports it as unchanged and does not run it.

02 your repo

### What a track folder holds

Each track is built and scored on its own folder. Your other tracks never count toward it, and code shared between tracks is copied into each folder that uses it and counts in each.

| file | what it is |
| --- | --- |
| Dockerfile | Builds the folder offline, from an allowlisted image pinned by digest |
| .speed-demon.toml | Track, team, language, dependency count; set `team` |
| PERF.md | Your optimisation log, worth 8% of your total |
| README.md | Your approach, your dependencies, your next 24 hours |
| your code | Replace the reference with your own; one output file to /out |

03 how it runs

### The contract your program meets

Your program reads its inputs from /data (read-only) and writes exactly one file to /out, then exits 0. TRACK is set to a, b, c or d. There is no network at build or run time, so commit your dependencies.

| limit | value |
| --- | --- |
| CPU | 12 threads · 6 P-cores · turbo off |
| Memory | 8 GB · no swap |
| Processes | 512 |
| Scratch | /tmp · 1 GB tmpfs |
| Network | none · build and run |
| Filesystem | read-only · /data is ro · write one file to /out |
| Board time cap | 3x the reference |
| Final time cap | 10x the reference |

Your output must match the reference's byte for byte: the correctness gate is a SHA-256 of the one file you write. The output names are `summary.txt` (A), `results.txt` (B), `aggregates.txt` (C) and `distances.txt` (D).

04 before you push

### Check it locally first

check.py builds a track offline and runs it under the board's sandbox, then prints the output's hash. Run it once on the unchanged folder to learn the right answer, then pass that hash back so the check fails on a wrong answer.

```
# generate data with the public generators (seed 1, 10% sample)
git clone https://github.com/hackathon-raptors/speed-demon-2026
python3.13 speed-demon-2026/generators/a_ingest.py --seed 1 --scale sample --out data/a

# build a track the way the board does, then compare the output hash
python3 check.py a --data data/a
python3 check.py a --data data/a --expected <sha256>
```

The full datasets and their hashes are published at kickoff, Oct 30, 15:00 UTC, linked from this site. On Colima, keep the repo and the data under your home folder so Docker can mount them.

05 your own language

### Bring any allowlisted toolchain

Replace the folder's contents with your code and a Dockerfile that builds it offline. A two-stage build keeps the image small. Every allowlisted image and its digest is in allowlist.json.

```
FROM rust:1-slim-trixie@sha256:1a939748a7cbab172de91a014e3804a97af1394a8c844faa67fce9401c4e40fb AS build
WORKDIR /src
COPY . .
RUN cargo build --release --offline

FROM debian:13-slim@sha256:62bd423aeee912639ca8ee681c2b762cb3e25137e0a92b20be455898a9f034ad
COPY --from=build /src/target/release/solution /app/solution
ENTRYPOINT ["/app/solution"]
```

Here `solution` is the package name in your `Cargo.toml`. Each direct runtime dependency costs a little on the D axis, so count them in `.speed-demon.toml`. See the full list at /allowlist.json (https://speeddemon.dev/allowlist.json).

06 guardrails

### What the board refuses before it builds

- A `FROM` that is not on the allowlist, or not pinned by digest.

- `ADD` of a URL or a git repo, and `COPY --from` an image (an earlier stage is fine).

- A `# syntax=` line; cache, secret or ssh mounts; `RUN --network`, `--security` or `--device`; `ONBUILD`.

- A build over 10 minutes, or an image over 2 GB.

07 common questions

### Two things teams ask

#### Is source size counted across all my tracks?

No. Each track is scored on its own folder: `track-a/` is Track A's build context and everything its source-size count covers, so your other track folders never count toward it. Code shared between tracks is copied into each folder that uses it, and counts in each.

#### One repository or several?

One repository per team, with your tracks as `track-a` to `track-d` on the `speed-demon` branch. Everything else about scoring and the gates is on the main page (https://speeddemon.dev/#coefficient).

Use the template ↗ (https://github.com/hackathon-raptors/speed-demon-starter) Ask on Discord ↗ (https://discord.com/invite/xfYPDZYqeh)

---

# Page: Board (live leaderboard; the table itself is live data, not included) (https://speeddemon.dev/board)

Live leaderboard Copy context (https://speeddemon.dev/speed-demon-context.md) Datasets Twitch live in **--:--:--** (https://twitch.tv/hackathonraptors)

LIVE the board

### The board, live

Every push is built and run on the Falkenstein box and scored on the machine axes, within minutes. This is the practice board: the machine score only. Judging is added after code freeze, and the holdout run is the one that counts.

code freeze in · --:--:--

Oct 2 kickoff Oct 30 freeze Nov 2 winners Nov 13 (https://speeddemon.dev/#timeline)

on the box now

runner · reading the feed…

tape the 30 latest new bests, rank moves and failed runs

- waiting for the next run…

the box Falkenstein · i5-13500 · 12 P + 8 E cores · 64 GB

−30 min now

languages in use · no scored runs yet

clicking devils 🔥 respect + 😈 + 🎃 + cans, all time

- no clicks yet

Loading the board…

Legend

A push, start to finish

- 01 queued waiting its turn; one run per team per 5 min

- 02 building the image builds, offline

- 03 queued to run built, waiting for the single runner

- 04 running the timed run, on the P-cores

- 05 · scored the lime number on the board · failed wrong output, timeout or sandbox · unchanged the starter template, nothing to run

idle nothing in flight. A thin bar under a badge means a container is building or running.

The demon

? 8 · level 8

? 27 ★★★ · level 27 · gold

? 38 ✦✦ ♛ · level 38 · cosmic

- **8** the level, from the team's machine score: 1 to 30 follow points

- **★** the top demon levels, 25 to 30

- **✦** cosmic levels 31 to 50, past full marks (K above 100)

- **♛** the top three overall

- **needs 3** fewer than three tracks scored, so no overall yet

- respect fire: the more respect a team gets, the hotter its demon burns

- effects: · ·

raw feed → (https://s3.amazonaws.com/raptors.dev/speeddemon.dev/2026/board/board.json)

DATA the datasets

### Full public datasets

Seed 1 at full scale, 790.5 MB in all: the exact files the live board runs against. Check every download against its sha256, then pass the expected output hash to `check.py --expected`. Final scoring uses a hidden holdout seed, not these files.

SHA256SUMS.json ↗ (https://s3.amazonaws.com/raptors.dev/speeddemon.dev/2026/data/SHA256SUMS.json) SHA256SUMS.txt ↗ (https://s3.amazonaws.com/raptors.dev/speeddemon.dev/2026/data/SHA256SUMS.txt)

| track | file | size | sha256 |  |
| --- | --- | --- | --- | --- |
| **A** Ingest | requests.log | 238.8 MB |  | download ↓ (https://s3.amazonaws.com/raptors.dev/speeddemon.dev/2026/data/track-a/full-seed1/requests.log) |
|  | → summary.txt | expected |  | check.py a --expected |
| **B** Index | docs.txt | 253.8 MB |  | download ↓ (https://s3.amazonaws.com/raptors.dev/speeddemon.dev/2026/data/track-b/full-seed1/docs.txt) |
|  | queries.txt | 1.8 MB |  | download ↓ (https://s3.amazonaws.com/raptors.dev/speeddemon.dev/2026/data/track-b/full-seed1/queries.txt) |
|  | → results.txt | expected |  | check.py b --expected |
| **C** Join | records.csv | 201.9 MB |  | download ↓ (https://s3.amazonaws.com/raptors.dev/speeddemon.dev/2026/data/track-c/full-seed1/records.csv) |
|  | regions.csv | 2.2 MB |  | download ↓ (https://s3.amazonaws.com/raptors.dev/speeddemon.dev/2026/data/track-c/full-seed1/regions.csv) |
|  | → aggregates.txt | expected |  | check.py c --expected |
| **D** Traverse | graph.txt | 92.0 MB |  | download ↓ (https://s3.amazonaws.com/raptors.dev/speeddemon.dev/2026/data/track-d/full-seed1/graph.txt) |
|  | queries.txt | 29 KB |  | download ↓ (https://s3.amazonaws.com/raptors.dev/speeddemon.dev/2026/data/track-d/full-seed1/queries.txt) |
|  | → distances.txt | expected |  | check.py d --expected |

---

# Clarifications given to participants

Answers the organisers gave on Discord and by email before kickoff. They match the frozen harness. All times are UTC.

**Kickoff and freeze times?**
Kickoff is Friday, October 30, 15:00. Code freeze is Monday, November 2, 15:00. An older email said 18:00; 15:00 is correct. The dates do not move.

**Can I set up before kickoff?**
Yes. Making your repo from the template, installing the board app, filling in the team form and preparing your environment are all fine before kickoff. What you may not do before kickoff is commit your own solution code into a track folder.

**Does tooling written before kickoff count as project code?**
No, as long as it stays outside the four track folders. Benchmark scripts and tools in the repo root, a tools folder or a separate repo are never scored and never count toward S. Code inside a track folder is project code: it must be written during the 72 hours and it counts toward S. A committed data file inside a track folder (for example a `.profdata` profile) is data, not code.

**Which branch triggers a board run, and which tracks run?**
Only the `speed-demon` branch. A new commit on it queues a run. Other branches do nothing. One push runs every `track-*` folder that differs from the published starter. A folder is "unchanged" when every file except Markdown and `.speed-demon.toml` is a byte-identical copy of the starter; unchanged folders are reported as `unchanged` and never run. There is no per-folder change detection: if track-b also differs from the starter, editing track-a re-runs track-b too. Each track gets one timed run per push on the board.

**Is the 15-minute limit per team or per track?**
Per team, across all tracks, counted from your last dispatched run. A newer push replaces the commit waiting in the queue, so your latest push is the one that runs.

**One repo or several?**
One repo per team, on the `speed-demon` branch, with tracks in `track-a` to `track-d`. Each folder is its own build context. Code shared between tracks is copied into each folder that uses it, and counts toward S in each.

**What counts toward source size S?**
Code lines as `scc` counts them (no blank lines, no comments), per track folder, after the organisers' formatter. Counted: the Dockerfile, build scripts (shell, `build.rs`), and all your source. Not counted: `README.md`, `PERF.md`, manifests (`Cargo.toml`, `package.json`, `go.mod` and so on), lockfiles, `.speed-demon.toml`. The starter's files are free only as byte-identical published copies; edit one and it counts. Registry packages first published before October 1, 2026 by people outside your team do not count.

**Is my formatter config (rustfmt.toml, .clang-format, .prettierrc, ruff.toml, fourmolu.yaml) used for S?**
No. The harness formats with its own neutral settings (for Rust: rustfmt defaults, edition from your Cargo.toml, else 2021). Formatter configs in your repo are ignored.

**Does a DSL, effects or thread-pool library count as a dependency?**
Yes. D counts every direct, non-standard-library runtime dependency in your manifest, whatever the library does. One dependency multiplies K by 0.90. If you write that layer yourself, it is your own code and lands in S instead. S is capped at 1, so source can cost you but never pays, and a dependency only ever costs you on D.

**Which memory counter is M? Does the /data page cache count?**
M is the container's cgroup v2 `memory.peak`, read on the host after your container exits. It is not `ru_maxrss`. It covers everything charged to your run's cgroup: your allocations, what you write into the 1 GB `/tmp` tmpfs, and any page your process is first to fault in. The `/data` input does not count: the host loads it into the page cache before your container starts.

**Can my program read /data? What does "touching the dataset mount" mean?**
Your program must read its input from `/data`, and it is mounted read-only. Disqualification is for tampering with the dataset or the mount, not for reading it. `mmap` of the input works.

**Rust PGO: where is llvm-profdata?**
Use the allowlisted image `halloumee/speed-demon-rust:1.99.1-trixie-llvm-tools`, which ships the LLVM tools. The plain `rust:1-slim-trixie` and `rust:1-trixie` images do not. Alternatively, merge the profile on your own machine and commit the `.profdata`; it must match the pinned toolchain (rustc 1.99.1, LLVM 23.1.1) or the build fails. The allowlist is closed; no new images are added.

**When must the repo be public?**
Within one hour after code freeze (November 2, 15:00), under an OSI-approved licence, at the commit we recorded. Keep the Speed Demon Board app installed through the freeze, because that is how the commit is recorded.

**Can the repo belong to a GitHub organisation?**
Yes. The registered person who submits the team form remains the entrant and the contact.

**Is there a submission besides the board app?**
Yes: one team form per team (https://tally.so/r/Ar7g9z) with the repo URL, tracks and members. Submit it again to change anything; the latest response counts. Team members can change until code freeze, and every member must be registered (https://tally.so/r/q4redG).

**Where can the write-up side quest post live?**
On a public blog: dev.to, Medium, Hashnode or your own site. A README or a GitHub Pages repo does not count. Mention Hackathon Raptors and link your repo. There is no special tag. Deadline: November 10. Two prizes of $100, judged on insight, not audience size.

**Are there country or age restrictions on prizes?**
No. Winners give their legal name and payment details privately. If a bank transfer does not work for your country, the prize can be paid in USDT.

---

# Speed Demon 2026 kickoff video: full transcript

The narration of the official kickoff video, shown on the Hackathon Raptors Twitch stream at kickoff (Oct 30, 15:00 UTC). Spoken numbers are written as digits here.

### Cold open

Every millisecond is a soul.

Welcome to Speed Demon, a seventy-two hour performance championship from Hackathon Raptors.

You get slow, correct code. Your job is to make it fast. Not a little faster. Demonically faster.

### The idea

Every track gives you three things. A task specification. A dataset. And a naive Python reference that solves the task correctly, and slowly.

You solve it again, correctly and not slowly. Any approach.

You never send us a timing. We build your code and run it ourselves, on one dedicated machine in Falkenstein, Germany. Same task, same data, same machine for everyone.

The machine score decides sixty-five percent of your result. Human judges decide the other thirty-five, because code nobody can read is not engineering.

Four tracks. $1,800 in prizes. Online, and free.

### Timeline

All times are UTC.

Kickoff is Friday, October thirtieth, at 15:00. From then, the full datasets and their expected hashes are on the site, and the live board is open. If an older email said 18:00, ignore it.

Code freeze is Monday, November second, at 15:00. The board closes, and we record the latest commit on your speed-demon branch. That commit is your submission.

By November fourth, we publish which gates each team passed. The write-up side quest closes on November tenth. Winners, full breakdowns and the scoring dataset come out on Friday, November thirteenth.

### Setup

Now, setup. You do it once.

First, every team member registers on speeddemon.dev, with the Register button.

Step one. On GitHub, open hackathon-raptors/speed-demon-starter, and press Use this template. That gives you your own repo. It can stay private until code freeze. One repo holds all of your team's tracks.

The template is a working entry for all four tracks. Each track has its own folder: track-a, track-b, track-c and track-d. Each folder is built on its own. It is the whole build context, so code you share between tracks must be copied into each folder.

Each folder holds a Dockerfile, a .speed-demon.toml file with your team name, a PERF.md, a README, and the code. The code starts as our naive reference, packaged exactly the way the board builds it. You replace it with your own. A folder you leave unchanged does not run, so you can delete the tracks you skip.

Step two. Install the Speed Demon Board GitHub App on that repo. It only reads the repos you pick. Keep it installed through code freeze, because that is how we record your final commit.

Step three. Fill in the team form, linked on the site. Give your team name, a contact email, your repo, your tracks and your languages. A team has one to four people. You can change your team until code freeze. Send the form again, and the latest answer counts.

Step four. Push to the speed-demon branch. It is the default branch, so a plain git push works. Other branches do nothing. The board builds each changed track folder, runs it, and shows the score.

Step five. Make it fast, and write it down. Every step goes into that track's PERF.md, with the board run ID that measured it.

At code freeze, you have one hour to make the repo public, under any OSI licence. The starter is already MIT. We score the commit we recorded at freeze.

### Tracks

There are four tracks. Each one writes one output file, and that file must match the reference byte for byte.

Track A, Ingest. You get three point one million log lines. For each hour and endpoint, you output the request count, the p50, p95 and p99 latency, and the errors. The file is summary.txt. The reference takes thirty-two seconds. Dataframe engines and SQL engines are banned.

Track B, Index. You get two hundred and fifty thousand documents and fifty thousand queries. Most are boolean, with AND, OR and NOT, read strictly left to right. The rest are phrase queries. Index build and queries are timed together, as one wall time. The file is results.txt. The reference takes fifty-six seconds. Search engines and full-text indexes are banned, and that includes SQLite full-text search.

Track C, Join. You get a hostile CSV file with three point three million rows. It has quoted commas, escaped quotes, newlines inside fields, mixed line endings, and a byte order mark in the middle of the file. Join it to a regions table, and write per-country totals to aggregates.txt, with money in integer cents. The reference takes fourteen seconds. Dataframes and query engines are banned, and that includes Python's sqlite3 module. CSV parser libraries are allowed. A parser that is three times faster and wrong scores zero.

Track D, Traverse. You get a directed, weighted graph with five million edges, and two thousand shortest-path queries. Write each distance to distances.txt, or minus one when the target cannot be reached. Preprocessing counts toward your time. The reference takes about seventy-three seconds. Shortest-path and routing libraries are banned, and so is any library that builds contraction hierarchies or hub labels for you. A plain graph container is fine.

Each spec names the banned libraries, and pins the exact output format. Read your track's spec before you write a line.

### Gates

Before we compute a score, a run must pass three gates. Fail one, and that track scores zero.

Gate one is correctness. The SHA-256 hash of your output file must match the expected hash exactly. In final scoring, it must match on all five runs. Nondeterminism is a correctness bug.

Gate two is the timeout. In final scoring, we kill a run at ten times the reference time for that track.

Gate three is the sandbox. There is no network. The dataset and the root filesystem are read-only. You get one gigabyte of scratch space in /tmp. Precomputed answers, phoning home, or tampering with the dataset disqualify you from the whole event.

### Machine

Here is the machine. An Intel Core i5-13500, with turbo off and a fixed clock. Your container gets twelve threads on six performance cores, eight gigabytes of memory with no swap, and five hundred and twelve processes. It has AVX2, but no AVX-512.

Your program reads its input from /data, writes exactly one file to /out, and exits with zero. The file names are in each spec, and the starter already follows them.

Your Dockerfile builds with no network. So commit your dependencies into the track folder. Registry packages first published before October first, twenty twenty-six, by people outside your team, do not count toward your source size.

Every FROM line uses an image from our allowlist, pinned by its digest. The list covers Debian, Python, Rust, Go, GCC, Java, GraalVM, Node, .NET, Swift and Haskell, and it is now closed. A build gets ten minutes, and the image can be up to two gigabytes. The board refuses some Dockerfile tricks, like ADD from a URL. The kickstart page lists them all.

You can train at build time. Profile-guided optimisation, a JVM AOT cache and GraalVM native image all work. /data is not there during the build, so train on data from the public generators. For Rust PGO, use our Rust image with the LLVM tools. If you build with -march=native, we build that track on the scoring machine itself. And io_uring is blocked, but mmap works.

### Scoring

Now, the score. The machine part is one number: the efficiency coefficient, K. It compares your run with the reference on four axes. Each axis is a ratio against the reference, and K multiplies them together, each raised to its weight.

T is wall time, from process start to exit, startup included. It carries half the weight, and it has no cap. A faster program always earns a higher K.

M is the peak memory of your container, as the kernel counts it. It carries twenty percent, and it is capped at fifty times better than the reference. The input files do not count. What you write to /tmp does.

S is source size: the code lines in that track's folder, counted by scc after we format your code with our own settings. Your formatter config is ignored. The Dockerfile counts. The README, PERF.md, manifests and lockfiles do not.

D is dependencies: one, plus the direct runtime dependencies in your manifest. With no dependencies, you lose nothing. One dependency multiplies K by zero point nine.

S and D carry fifteen percent each, and they can only cost you. Code longer than the reference costs a little. Code shorter than the reference earns nothing extra.

Then K becomes points: 50 × log10(K). A K of one or less gives zero. A K of ten gives fifty. A K of one hundred gives the full hundred. Points stop there, but K does not, and raw K still breaks ties.

Your track total is sixty-five percent points, and thirty-five percent judges. Judges score three things on a five-point scale. Code quality, fifteen percent. Technique, twelve. And your PERF.md optimisation log, eight.

The overall title uses the geometric mean of your best three track totals. So you need at least three tracks, and balance beats one huge result. Ties go to raw K, then wall time, then the earliest commit.

### Board and final

While you work, you have the live board, at speeddemon.dev/board. Each push runs once, on the full public dataset, which is seed one of the public generators.

Your team gets one run every fifteen minutes at most. One push runs all of your changed tracks, and counts as one run. A newer push replaces the one in the queue. On the board, a run stops at three times the reference time.

Near the freeze, the queue gets long. Do not leave your last push to the last hour.

Every run gets an ID. Put those IDs in PERF.md. Judges read your board history.

Next to your team, a demon grows with your machine score. Past a K of one hundred, it goes cosmic.

To test before you push, run check.py from the starter. It builds a track the way the board does, runs it in the same sandbox, and prints the output hash. Run it once on the unchanged reference to get the right hash for any data you generate. Then pass that hash back, and the check fails on a wrong answer.

The board is practice. Final scoring is different. After freeze, we run your recorded commit on a new dataset, from the same generators, at a seed you have never seen. Five runs. We drop the fastest and the slowest, and take the mean of the middle three. That is the number that counts.

### Rules and prizes

AI tools are expected. Use whatever you like. We judge the result, and your PERF.md shows that a human understood it.

All solution code is new. You write it in these seventy-two hours. The starter template, and tools outside the track folders, are fine. Committing your own solution code before kickoff disqualifies the submission.

The harness is the referee. If you think a run was measured wrong, you have forty-eight hours after the results to ask for a re-run. And the formula is frozen for the whole event.

The prizes. The Speed Demon title pays eight hundred dollars. Second place pays four hundred. Third pays two hundred.

The highest K on each track wins fifty dollars. You do not need three tracks for that one.

And there is a write-up side quest, with two prizes of one hundred dollars. Publish a post on a public blog, like dev.to, Medium, Hashnode or your own site. A README or a GitHub Pages repo does not count. Mention Hackathon Raptors, link your repo, and publish by November tenth. We judge insight, not audience size.

There are no country or age limits on prizes.

### Outro

One last time: keep the board app installed, and go public within an hour of freeze.

Everything in this video is on speeddemon.dev, and the kickstart page walks you through setup. Questions go to the Hackathon Raptors Discord.

The clock is running. Every millisecond is a soul. Go.

---

# Starter template README (github.com/hackathon-raptors/speed-demon-starter)

## Speed Demon 2026 starter

A working entry for all four tracks of [Speed Demon 2026](https://speeddemon.dev). Each `track-*` folder holds the organisers' naive Python reference, packaged the way the live board builds and runs it. Use it to check your setup end to end, then replace the reference with your own code.

### Start in five steps

1. **Use this template** (the green button on GitHub) to create your repo. It can stay private until code freeze.
2. **Install the [Speed Demon Board app](https://github.com/apps/speed-demon-board)** on that repo. It gets read-only access to the repos you pick and nothing else.
3. **Register your team** on the [submission form](https://tally.so/r/Ar7g9z), with the repo URL and the tracks you are entering.
4. **Push to the `speed-demon` branch.** It is this template's default branch, so a plain `git push` does it. The board picks up each push, builds every `track-*` folder and runs it once on the full public dataset. Results appear on speeddemon.dev.
5. **Make it fast.** Log every step in the track's `PERF.md`, with the board run ID that measured it. At code freeze (November 2, 15:00 UTC), make the repo public.

One repo per team holds all your tracks. Delete the folders of the tracks you are not entering. An unchanged folder is the reference itself, so the board reports it as unchanged and does not run it.

### What is in a track folder

| File | What it is |
|---|---|
| `Dockerfile` | Builds the folder offline, from an allowlisted image pinned by digest |
| `.speed-demon.toml` | Track, team, language, dependency count; set `team` |
| `PERF.md` | Your optimisation log, worth 8% of your total |
| `README.md` | Your approach, your dependencies, your next 24 hours |
| `run.py` + module | The reference; replace it with your code |

### How your program runs

- It reads its inputs from `/data`, which is read-only, and writes exactly one file to `/out`, then exits 0. The file is `summary.txt` for A, `results.txt` for B, `aggregates.txt` for C and `distances.txt` for D.
- `TRACK` is set to `a`, `b`, `c` or `d`.
- There is no network, at build time or at run time. Commit your dependencies to the repo (`cargo vendor`, `go mod vendor` and so on).
- It runs as an unprivileged user on a read-only filesystem. `/tmp` is 1 GB of scratch space.
- It gets 12 hardware threads (six P-cores, turbo off), 8 GB of memory with no swap, and 512 processes.
- The live board stops a run at 3x the reference's time. Final scoring stops it at 10x.
- Your output must match the reference's byte for byte (sha256) on every run.

### Check before you push

`check.py` builds a track offline and runs it with the board's Docker settings, then prints the output's sha256 and a rough time. You need Docker and Python 3.

```
python3 check.py a --data data/a
```

To get data, generate it with the public generators. Seed 1 at `full` scale is the board's dataset; `sample` is 10% of it.

```
git clone https://github.com/hackathon-raptors/speed-demon-2026
python3.13 speed-demon-2026/generators/a_ingest.py --seed 1 --scale sample --out data/a
```

No Python 3.13? Run the generator in the same image the reference uses:

```
docker run --rm -v "$PWD:/w" -w /w python:3.13-slim python3 speed-demon-2026/generators/a_ingest.py --seed 1 --scale sample --out data/a
```

Run the check once on the unchanged folder. The sha256 it prints is the correct answer for that data. After every change, pass it back, so the check fails on a wrong answer:

```
python3 check.py a --data data/a --expected <sha256>
```

The full datasets and their expected hashes are also published at kickoff, October 30, 15:00 UTC, linked from speeddemon.dev.

On Colima, keep the repo and the data under your home folder, or Docker cannot mount them.

### Your own language

Replace the folder's contents with your code and a Dockerfile that builds it offline. A two-stage build keeps the image small. For Rust with no crates:

```
FROM rust:1-slim-trixie@sha256:1a939748a7cbab172de91a014e3804a97af1394a8c844faa67fce9401c4e40fb AS build
WORKDIR /src
COPY . .
RUN cargo build --release --offline

FROM debian:13-slim@sha256:62bd423aeee912639ca8ee681c2b762cb3e25137e0a92b20be455898a9f034ad
COPY --from=build /src/target/release/solution /app/solution
ENTRYPOINT ["/app/solution"]
```

Here `solution` is the package name in your `Cargo.toml`. Every allowlisted image and its digest is in [allowlist.json](https://speeddemon.dev/allowlist.json). Each direct runtime dependency costs a little, so count them in `.speed-demon.toml`.

### What the board refuses before it builds

- A `FROM` that is not in the allowlist, or not pinned by digest.
- `ADD` of a URL or a git repo, and `COPY --from` an image (an earlier stage is fine).
- A `# syntax=` line; cache, secret or ssh mounts; `RUN --network`, `--security` or `--device`; `ONBUILD`.
- A build over 10 minutes, or an image over 2 GB.

### Scoring, briefly

Faster than the reference earns points. Using less memory helps, up to a cap. Code beyond the reference's size, and every dependency, cost a little. Each track folder is built and measured on its own: your other tracks never count toward it, and code shared between tracks is copied into each folder that uses it and counts in each. A run that fails correctness, the timeout or the sandbox scores nothing. The full formula and the judging criteria are on speeddemon.dev.

### Licence

The reference code is copyright Hackathon Raptors CIC, under the MIT licence in `LICENSE`. Your repo has to be public under an OSI-approved licence at code freeze; keeping MIT is the simplest way.

---

# Harness repository README (github.com/hackathon-raptors/speed-demon-2026)

## Speed Demon 2026

Track specs, dataset generators, naive references and the scoring harness for [Speed Demon 2026](https://speeddemon.dev), October 30 to November 2, 2026. To enter, start from the [starter template](https://github.com/hackathon-raptors/speed-demon-starter).

| Track | Spec | Generator | Reference |
|---|---|---|---|
| A Ingest | [`specs/a-ingest.md`](specs/a-ingest.md) | [`generators/a_ingest.py`](generators/a_ingest.py) | [`reference/a_ingest.py`](reference/a_ingest.py) |
| B Index | [`specs/b-index.md`](specs/b-index.md) | [`generators/b_index.py`](generators/b_index.py) | [`reference/b_index.py`](reference/b_index.py) |
| C Join | [`specs/c-join.md`](specs/c-join.md) | [`generators/c_join.py`](generators/c_join.py) | [`reference/c_join.py`](reference/c_join.py) |
| D Traverse | [`specs/d-traverse.md`](specs/d-traverse.md) | [`generators/d_traverse.py`](generators/d_traverse.py) | [`reference/d_traverse.py`](reference/d_traverse.py) |

- [`harness/`](harness/README.md) scores every submission, and you can run it yourself.
- [`board/`](board/README.md) is the live board that runs each push from kickoff to code freeze.
- [`CONVENTIONS.md`](CONVENTIONS.md) is the contract every track follows.

### Running it

Python 3.11 or newer, standard library only.

```
python3 generators/a_ingest.py --seed 1 --scale sample --out .data/a/sample-seed1
python3 -m unittest discover -s tests
```

The same seed and scale give byte-identical files on any machine. Seed 1 at `full` scale is the live board's dataset; the full datasets and their expected hashes are also published on speeddemon.dev at kickoff, October 30, 15:00 UTC.

MIT licensed.

---

# Track spec: specs/a-ingest.md (github.com/hackathon-raptors/speed-demon-2026)

## Track A: Ingest

### 1. Summary

Read 3,100,000 HTTP request log lines and, for every hour and endpoint that saw traffic, report the request count, the p50, p95 and p99 latency, and the number and rate of 5xx responses. The input is one space-delimited text file covering 72 hours. It is not sorted by time: eight log shards each write in time order, and their chunks are interleaved as they arrive. The output is one text file with one line per (hour, endpoint) group, in a fixed total order, so a correct answer has exactly one SHA-256.

### 2. Input files

One file, `/data/requests.log`.

**Encoding.** ASCII (so also valid UTF-8), no BOM. Every line ends with a single LF, including the last. No CR, no empty lines, no header, no comments. Every line matches the grammar below; a solution may assume well-formed input.

**Grammar** (ABNF, `SP` is one space `0x20`, `LF` is `0x0A`):

```
line      = ts SP method SP endpoint SP status SP latency LF
ts        = 4DIGIT "-" 2DIGIT "-" 2DIGIT "T" 2DIGIT ":" 2DIGIT ":" 2DIGIT "." 6DIGIT "Z"
method    = "GET" / "POST" / "PUT" / "PATCH" / "DELETE" / "HEAD" / "OPTIONS"
endpoint  = "/" *124echar
echar     = ALPHA / DIGIT / "/" / "-" / "_" / "." / "{" / "}" / ":"
status    = ("1" / "2" / "3" / "4" / "5") 2DIGIT
latency   = "0" / (%x31-39 *9DIGIT)
```

| Field | Meaning and range |
|---|---|
| `ts` | Request time, RFC 3339 in UTC, always exactly 27 bytes: microsecond precision (six fractional digits) and a trailing `Z`. Always a real calendar date and time. Every timestamp lies in the same 72 hours for every seed and scale: `2026-10-30T18:00:00.000000Z` to `2026-11-02T17:59:59.999999Z` inclusive. The span crosses two midnights and the October to November month boundary. |
| `method` | One of the seven methods above, 3 to 7 bytes. |
| `endpoint` | A path template such as `/v2/orders/{id}`: 1 to 125 bytes from the set `A-Z a-z 0-9 / - _ . { } :`, starting with `/`. Never contains a space. Placeholders like `{id}` are literal text. |
| `status` | HTTP status, always three digits, 100 to 599. The generator emits 200 201 202 204 301 302 304 400 401 403 404 405 409 410 422 429 499 500 501 502 503 504 599; a solution must handle any value in 100 to 599. |
| `latency` | Microseconds (not milliseconds), unsigned 32-bit: 0 to 4294967295, decimal with no leading zeros. |

**Endpoints.** Each seed has a catalogue of exactly 1,842 distinct endpoint templates. Thirteen are the same for every seed: `/`, `/health`, `/healthz`, `/v2/checkout`, the byte-order family `/v2/orders`, `/v2/orders-export`, `/v2/orders.csv`, `/v2/orders/{id}`, `/v2/orders/{id}/items`, `/v2/ordersByDate`, `/v2/orders_legacy`, `/v2/orderstatus`, and the longest template, `/internal/v1/tenants/{tenant_id}/projects/{project_id}/pipelines/{pipeline_id}/runs/{run_id}/artifacts/{artifact_id}:download` (125 bytes). The other 1,829 are composed from a vocabulary and depend on the seed. Popularity follows a Zipf law with exponent 1 over a seed-dependent ranking, so a handful of endpoints carry most of the traffic and a long tail sees a few requests a day. `full` and `sample` contain all 1,842 endpoints; `tiny` contains a subset.

**Order.** Lines are not sorted. The generator models eight shards, each writing non-decreasing timestamps (ties allowed). Shards 1 to 7 ship a chunk every 1 to 120 seconds of log time, 0 to 30 seconds after it closes; shard 0 uploads each hour as one chunk, 5 to 10 minutes after the hour ends. The file is the chunks in arrival order. The shard is not a field, and **no bound on how far a line can be out of order is promised**: a solution must not rely on one.

**Edge cases.** Each is injected on purpose, at a fixed rate in `sample` and `full` and at least once in `tiny`, for every seed. A *group* is an (hour, endpoint) pair (see the Task section). Rates are per million lines, so `full` gets ten times the `sample` count.

| # | Case | sample / full | tiny |
|---|---|---|---|
| E1 | A group with exactly one request | 100 per million (31 / 310) | 3 |
| E2 | A group whose latencies are exactly 0 and 4294967295 (p50 = 0, p95 = p99 = 4294967295) | 10 per million (3 / 31) | 1 |
| E3 | A group whose statuses are exactly 200, 499, 500 and 599 (2 errors of 4) | 10 per million (3 / 31) | 1 |
| E4 | A timestamp exactly on an hour boundary, `HH:00:00.000000Z`, paired with `HH:59:59.999999Z` of the hour before, written after it | at each of the 71 boundaries, lines/100,000 pairs (3 / 31) | 3 pairs: 2026-10-30T19:00, 2026-11-01T00:00 (day and month rollover), 2026-11-02T17:00 |
| E5 | The first and last instants of the span, `2026-10-30T18:00:00.000000Z` and `2026-11-02T17:59:59.999999Z` | 1 each | 1 each |
| E6 | Latency 0 outside E2 | 100 per million (31 / 310) | 2 |
| E7 | Latency 4294967295 outside E2 | 100 per million (31 / 310) | 2 |
| E8 | Status 499 next to 500 and 599 | E3, plus ordinary traffic (499 and 599 are rare there) | E3 |
| E9 | Equal timestamps on consecutive lines, different requests | 100 per million pairs | 2 pairs |
| E10 | Equal timestamps on lines far apart (two shards) | 100 per million pairs | 2 pairs |
| E11 | Byte-identical duplicate lines, which count as two requests | 50 per million pairs | 1 pair |
| E12 | A line whose hour is earlier than the hour of a line before it | every E4 pair | every E4 pair |
| E13 | All seven methods | 1 each, plus ordinary traffic | 1 each |
| E14 | All thirteen fixed endpoints, including the 1-byte `/` and the 125-byte template, in one shared hour | 1 each, plus ordinary traffic | 1 each |
| E15 | Every catalogue endpoint at least once (1,842 distinct) | 1 each | not applicable |

### 3. Task

1. **Group key.** Each line belongs to the group (hour, endpoint). The hour is the timestamp truncated to the UTC hour: `2026-10-30T18:59:59.999999Z` is in hour 18:00, and `2026-10-30T19:00:00.000000Z` is in hour 19:00. The endpoint is compared as an exact byte string, with no normalisation of case, slashes or placeholders.
2. **Ignored fields.** `method` does not affect the result. `status` matters only for the error count.
3. **count** is the number of lines in the group. Duplicate lines are separate requests and each one counts.
4. **p50, p95, p99** use the nearest-rank method. Sort the group's n latencies ascending, keeping duplicates; the p-th percentile is the value at 1-indexed rank `ceil(p * n / 100)`, which in integer arithmetic is `(p * n + 99) div 100`. Every percentile is therefore one of the input latencies. With n = 1 all three equal the single value. With n = 101, the ranks are 51, 96 and 100.
5. **errors** is the number of lines in the group with 500 ≤ status ≤ 599. A 499 is not an error.
6. **error_ppm** is `floor(errors * 1000000 / count)`, computed in integers: 1 error in 3 requests is `333333`.
7. **Empty and unknown.** Only groups with at least one line are printed; an hour with no traffic for an endpoint produces no line, and nothing is printed for the catalogue as such. There are no unknown keys: every endpoint string that occurs is its own key. An empty `requests.log` (the generator never writes one) gives an empty `summary.txt` of zero bytes.

### 4. Output

One file, `/out/summary.txt`. UTF-8 (in practice ASCII), LF line endings, a newline after every line including the last, no BOM, no header, no trailing spaces. One line per group, eight fields separated by single spaces:

```
hour endpoint count p50 p95 p99 errors error_ppm
```

- `hour` is the group's hour as `YYYY-MM-DDTHH:00:00Z`, for example `2026-10-30T18:00:00Z`.
- `endpoint` is copied byte for byte from the input.
- The six numbers are decimal integers with no leading zeros and no sign. `count` ≥ 1, `0 ≤ errors ≤ count`, `0 ≤ error_ppm ≤ 1000000`.

**Order.** Lines are sorted by `hour` ascending, then by `endpoint` ascending as unsigned bytes, where a proper prefix sorts first. Each (hour, endpoint) appears exactly once, so the order is total. The fixed-width `hour` field makes chronological order the same as byte order. Byte order is not case-insensitive, locale or per-segment order: within one hour, `/v2/orders` < `/v2/orders-export` < `/v2/orders.csv` < `/v2/orders/{id}` < `/v2/orders/{id}/items` < `/v2/ordersByDate` < `/v2/orders_legacy` < `/v2/orderstatus`, and `/v2/rank100` < `/v2/rank20`.

**Worked example.** This `requests.log`:

```text
2026-10-30T19:00:00.000000Z GET /v2/orders/{id} 200 1500
2026-10-30T18:59:59.999999Z GET /v2/orders/{id} 200 1841
2026-10-30T18:00:00.000000Z POST /v2/checkout 503 204117
2026-10-30T18:00:00.000000Z POST /v2/checkout 201 30000
2026-10-30T18:00:00.115332Z GET /health 200 62
2026-10-30T18:00:00.115332Z GET /health 200 62
2026-10-30T18:42:17.000001Z HEAD /health 200 4294967295
2026-10-30T18:30:00.000001Z OPTIONS / 200 0
2026-10-30T18:12:00.500000Z POST /v2/checkout 500 0
2026-10-30T18:12:00.600000Z POST /v2/checkout 499 1500
2026-10-30T18:12:00.700000Z POST /v2/checkout 599 2500
2026-10-30T18:20:00.000000Z GET /v2/orders/{id} 500 900
2026-10-30T18:21:00.000000Z DELETE /v2/orders/{id} 404 77
2026-10-30T19:05:00.000000Z GET /v2/orders_legacy 200 60
2026-10-30T19:05:00.000001Z POST /v2/orders-export 202 20
2026-10-30T19:05:00.000002Z GET /v2/orders 200 10
2026-10-31T23:59:59.999999Z POST /v2/checkout 502 7
2026-11-01T00:00:00.000000Z POST /v2/checkout 503 8
```

gives exactly these bytes in `summary.txt`:

```text
2026-10-30T18:00:00Z / 1 0 0 0 0 0
2026-10-30T18:00:00Z /health 3 62 4294967295 4294967295 0 0
2026-10-30T18:00:00Z /v2/checkout 5 2500 204117 204117 3 600000
2026-10-30T18:00:00Z /v2/orders/{id} 3 900 1841 1841 1 333333
2026-10-30T19:00:00Z /v2/orders 1 10 10 10 0 0
2026-10-30T19:00:00Z /v2/orders-export 1 20 20 20 0 0
2026-10-30T19:00:00Z /v2/orders/{id} 1 1500 1500 1500 0 0
2026-10-30T19:00:00Z /v2/orders_legacy 1 60 60 60 0 0
2026-10-31T23:00:00Z /v2/checkout 1 7 7 7 1 1000000
2026-11-01T00:00:00Z /v2/checkout 1 8 8 8 1 1000000
```

Reading it: the two `/health` lines at `18:00:00.115332Z` are duplicates and both count, so n = 3 with sorted latencies 62, 62, 4294967295; p50 is rank 2 (62) and p95 and p99 are rank 3. `/v2/checkout` in hour 18 has latencies 0, 1500, 2500, 30000, 204117; p50 is rank 3 (2500); its errors are the 503, the 500 and the 599, not the 499 or the 201, so 3 of 5 is 600000 ppm. The `18:59:59.999999Z` request belongs to hour 18 even though the file lists the `19:00:00.000000Z` one first.

### 5. Sizes

Measured with seed 1 on an Apple M4 Pro, Python 3.13.12. The generator writes the same bytes, at all three scales, on Linux aarch64 in `python:3.13-slim` (Python 3.13.17), and the reference run through `reference/run.py` there gives the same `summary.txt` hashes for `tiny` and `sample`.

| Scale | Lines | `requests.log` bytes | Generation | `requests.log` sha256 |
|---|---|---|---|---|
| full | 3,100,000 | 238,836,234 | 4.5 s | `7ec80ba45bceef51ac7f79d3a14521fc6187b2dcb87b5e5542d08063fe5d962a` |
| sample | 310,000 | 23,870,290 | 0.5 s | `7b22c678acceba2b82120394afe27e827ca4083a677a8e51147defa854c86847` |
| tiny | 1,000 | 76,509 | under 0.1 s | `a99dd14856a3e6e408097dabc1b3ad5ac5a3801fb12f388bacb2802c8591339f` |

| Scale | `summary.txt` lines | `summary.txt` bytes | `summary.txt` sha256 |
|---|---|---|---|
| full | 126,908 | 10,113,024 | `495ca09682b9673ef836a506496177680bb4a3af54255174befddb1f83481cd3` |
| sample | 66,153 | 5,185,225 | `ca6311bbeb536fa55d85265bf6fd3d24cc9586f13de1ac73bd2a23ac3893aac3` |
| tiny | 873 | 68,899 | `c24fa507c963eaeac9224fd8435019b4f4e8c002ece7e32f316681a3957eb92f` |

The reference on `full`: 13.4 s wall time and 237 MB peak RSS on the M4 Pro, measured twice with `/usr/bin/time -l` (13.39 s and 13.44 s, same output hash). The scoring box is calibrated on Oct 26 and 27.

### 6. Banned libraries

Per rule 02, the library that solves the whole task is banned. For this track that means, in any language and through any binding:

- **Dataframe and analytics engines:** pandas, Polars, DuckDB, Apache Arrow compute (pyarrow, Arrow C++ compute and Acero, the arrow-rs compute kernels), Apache DataFusion, ClickHouse (clickhouse-local, chDB), Apache Spark and PySpark, Dask, Modin, Vaex, cuDF, Velox, R data.table and dplyr, Julia DataFrames.jl.
- **SQL engines of any kind, embedded or server:** SQLite, DuckDB, H2, PostgreSQL, MySQL and the like.
- **Command-line tools that compute grouped statistics:** GNU datamash, Miller (`mlr`), xsv, qsv, csvkit.

Everything else is allowed and priced on the D axis: hash maps, sorting and selection routines, parsing and SIMD helpers, date and time libraries, allocators, numpy. Ask the organisers before kickoff if a library is not on this list and you are unsure.

### 7. Reference behaviour

`reference/a_ingest.py` (CPython 3.13, standard library only, single-threaded):

- Reads `requests.log` line by line through Python's text-mode file iterator, which decodes every line into a new string.
- Splits each line on spaces into five strings.
- Parses every timestamp with `datetime.strptime`, an RFC 3339 format with `%z` for the `Z`, then truncates it to the hour with `replace()`. This is most of the run time.
- Keys a dictionary by (hour datetime, endpoint) and appends each latency, as a Python int, to a growing list for that group; a second dictionary counts 5xx responses.
- Keeps every latency until the end of the input, then sorts the group keys, sorts each group's list in full and reads the three percentiles from it.

### 8. Changes from the site

Compared with the Track A entry on https://speeddemon.dev (`late-oct-event/src/data/tracks.json`):

1. **The site's `spec` numbers are the final scoring-box figures.** `dataset_mb` is 239 (238,836,234 bytes for seed 1); `reference_s` is the box-calibrated reference and `timeout_s` is ten times it. The line count, 3.1M, is the binding number; lines of this format average 77 bytes. About 80% of the reference's time is the per-line `strptime`, so with 3.1M lines fixed the time follows the line count rather than the data's shape. (On an Apple M4 Pro the reference runs in about 13.4 s; the scoring box differs.)
2. **The three preview rows** on the site are lines 849 to 851 of the seed-1 sample, unmodified:
   `2026-10-30T18:07:18.569411Z GET /api/v2/products 200 8177`,
   `2026-10-30T18:07:21.136274Z GET /beta/regions 503 81573`,
   `2026-10-30T18:07:27.963350Z PUT /v2/ab_tests 200 6381`.
   The site's example endpoints `/v2/orders/{id}`, `/v2/checkout` and `/health` are in every catalogue.
5. **`status` is narrowed from uint16 to three digits, 100 to 599.**
6. **The task is pinned down where the site is loose:** percentiles are nearest rank, and the error rate is printed as an integer count plus floor parts per million instead of a fraction.
7. **"Monotonic per shard but interleaved across shards"** is kept, with two clarifications: the shard is not a field in the line, and no bound on disorder is promised.
8. **Added detail the site does not give:** the 72-hour span (2026-10-30T18:00Z to 2026-11-02T18:00Z, fixed for every seed) and the eight shards.
9. **"1,842 distinct"** holds for `full` and `sample`. `tiny`, the test scale, has a subset of the catalogue.

---

# Track spec: specs/b-index.md (github.com/hackathon-raptors/speed-demon-2026)

## Track B: Index

### 1. Summary

You get a 254 MB corpus of legal-flavoured text, one document per line, and 50,000 queries: 62% boolean (`AND` / `OR` / `NOT`, strictly left to right) and 38% exact phrase queries. Write one line per query: the number of matching documents, the sum of their ids, and the ten smallest ids. Your score is one wall-clock time for the whole run, so reading the corpus, building whatever index you want and answering the queries are all on the same clock. Tokenization is byte-based and ASCII-only so every language can reproduce it exactly. The queries are in the input from the start: the task is to answer these 50,000 queries, and how much of the corpus you index to do that is your call.

### 2. Input files

Both files are in `/data`, read-only.

#### 2.1 `docs.txt`

- Valid UTF-8, no BOM, LF line endings, one trailing LF.
- Line `i` (counting from 0) is the decimal id `i`, one TAB byte (0x09), the document text, then LF. Ids are dense and ascending, `0` to `N-1`, with no leading zeros.
- The text is zero or more bytes, none of which is TAB, LF or CR. The generator writes printable ASCII (0x20 to 0x7E) and valid 2-, 3- and 4-byte UTF-8 sequences.
- The text may be empty, in which case the line is the id, a TAB and the LF.

#### 2.2 Tokenization

This rule is the whole definition of a token. Apply it to the document text, the bytes after the first TAB and before the LF:

1. ASCII-lowercase: bytes 0x41 to 0x5A (`A` to `Z`) become 0x61 to 0x7A. No other byte changes.
2. A token is a maximal run of bytes in 0x61 to 0x7A (`a` to `z`) or 0x30 to 0x39 (`0` to `9`).
3. Every other byte is a separator. That includes every byte of 0x80 or above, even inside a letter: `Zürich` is the two tokens `z` and `rich`, `naïve` is `na` and `ve`, `273K` written with the KELVIN SIGN (U+212A) is `273`, and `İSTANBUL` is `stanbul`.
4. Tokens are numbered 0, 1, 2, ... in the order they occur. The number of separator bytes between two tokens does not matter.

There is no Unicode case folding or normalisation, no stemming and no stopword removal. Tokens are byte strings, so `007` and `7` are different tokens. `habeas-corpus`, `court's` and `U.S.C.` are 2, 2 and 3 tokens. No token in a generated corpus is longer than 32 bytes (the longest is 17).

In Python this is `re.findall(rb"[a-z0-9]+", text.lower())` on `bytes`, because `bytes.lower()` only touches `A` to `Z`. A tokenizer that decodes UTF-8 and uses Unicode-aware lowercasing or word boundaries gets a different answer on this corpus (`"K".lower()` is `k`, `"İ".lower()` is `i` plus a combining dot), and so does NFKC normalisation (`ﬁ` becomes `fi`).

#### 2.3 `queries.txt`

- ASCII, LF line endings, one trailing LF, one query per line.
- Grammar, with single spaces exactly where shown:

```
query   = bool / phrase
bool    = "BOOL" SP term *( SP op SP term )            ; 1 to 6 terms
op      = "AND" / "OR" / "NOT"
phrase  = "PHRASE" SP DQUOTE term 1*( SP term ) DQUOTE  ; 2 to 8 terms
term    = 1*32( %x61-7A / %x30-39 )                     ; a-z and 0-9
```

- There are no parentheses, no leading operator and no empty term. Terms are always lowercase and operators always uppercase, so `and`, `or` and `not` in a term position are ordinary words (`BOOL and AND not` is valid).
- A `BOOL` query may have a single term and no operator.
- The same line can appear more than once. Each occurrence gets its own output line.

#### 2.4 How the corpus is generated

Every scale uses the same distribution, and only the counts change. `sample` is not a prefix of `full`, and the holdout seed changes the synthetic vocabulary, the rank order of the legal words and every draw.

- **Vocabulary.** 600,000 words ranked by frequency. Ranks 1 to 122 are English function words in a fixed order (`the`, `of`, `and`, `to`, `in`, ...). 509 legal words (`indemnity`, `surety`, `writ`, `habeas`, `corpus`, `bail`, `bond`, `recognizance`, `forfeiture`, `matter`, ...) sit at ranks 123 to 7,082 in a seed-dependent order. The remaining ranks hold seed-dependent synthetic words of 3 to 12 lowercase letters.
- **Words.** Each word unit is drawn independently with weight proportional to `1 / (r + 1/2)` for rank `r` (Zipf-Mandelbrot, exponent 1).
- **Documents.** A document has 1 to 999 word units: 1 to 9 (5% of documents), 10 to 79 (35%), 80 to 199 (42%), 200 to 399 (16%), 400 to 999 (2%), uniform inside each bucket. About 2% of units are replaced by one of 93 legal phrases (`writ of habeas corpus`, `in the matter of`, `bail bond`, `and/or`, ...) and 1.5% by a number or citation (`1983`, `42 U.S.C. § 1983`, `No. 07-1234`, `$5,000.00`, `3rd`, `Fed. R. Civ. P. 12(b)(6)`, `Velmora v. Trisk`).
- **Text.** Units are joined by a space (86%) or by `, `, `. `, `; `, `: `, ` (`, `) `, `-`, `/`, `'s `, ` - `, ` & `, `... `, quotes, an em dash (U+2014) with or without spaces, a no-break space (U+00A0) or `’s ` (U+2019). The two possessive separators add the token `s`. The first letter of each sentence is capitalised.

Edge cases are injected into a fixed share of documents at every scale, with a minimum count so `tiny` has them too. A document gets at most one of these:

| Injected case | Share of documents | full | sample | tiny |
|---|---|---|---|---|
| Empty text | 0.05% | 125 | 12 | 2 |
| Text with bytes but no token (`§§ ¶`, `* * *`, `—`, ...) | 0.05% | 125 | 12 | 2 |
| 1 to 3 non-ASCII words from a list of 32 (`café`, `Zürich`, `Straße`, `İstanbul`, `273K`, `ﬁduciary`, `bail bond` with a no-break space, `𝔄ct`, `Ñandú`, ...) | 2% | 5,000 | 500 | 20 |
| Whole text in ASCII upper case | 1% | 2,500 | 250 | 10 |
| Whole text in ASCII title case | 1% | 2,500 | 250 | 10 |
| A run of one word repeated 2, 3 or 4 times (`w w w`) | 0.5% | 1,250 | 125 | 6 |
| A zero-padded case number (`Case No. 2026-cv-00123`) | 0.5% | 1,250 | 125 | 6 |
| A punctuation-joined word from a list of 30 (`habeas-corpus`, `and/or`, `court's`, `U.S.C.`, `plaintiff(s)`, `d/b/a`, ...) | 1% | 2,500 | 250 | 10 |

Outside those documents, every document repeats common words, almost every one has capital letters, and hyphens, slashes, apostrophes, em dashes and no-break spaces join words all over the corpus.

#### 2.5 How the queries are drawn

The document frequency `df(t)` of a token is the number of documents containing it, and `N` is the document count. Tokens fall into five bands:

| Band | df | Terms at full (seed 1) |
|---|---|---|
| VC | df > N/10 | 112 |
| C | N/100 < df ≤ N/10 | 1,032 |
| M | N/1,000 < df ≤ N/100 | 9,347 |
| U | N/10,000 < df ≤ N/1,000 | 90,640 |
| R | 1 ≤ df ≤ N/10,000 | 508,702 |

When a band is empty (at `tiny`, R always is), the nearest non-empty band is used instead, trying the rarer side first.

**BOOL queries (31,000 at full).** 27,200 of them are ordinary:

- **Terms.** 2 to 6 terms: 2 (38%), 3 (32%), 4 (18%), 5 (8%), 6 (4%).
- **Operators.** Each operator is independently `AND` (50%), `OR` (30%) or `NOT` (20%).
- **Bands.** Each query picks one random document with at least one token as its source. Each term picks a band (VC 3%, C 12%, M 30%, U 30%, R 25%). With probability 75% the term is drawn uniformly from the source document's distinct tokens in that band. Otherwise, or if the source has none in that band, it is drawn uniformly from all corpus tokens in the band.

The other 3,800 are edge cases, at a fixed share of all queries:

| Kind | Share of all queries | full | What it is |
|---|---|---|---|
| single | 2% | 1,000 | one term, no operator |
| absent | 1% | 500 | contains a term that is in no document, half of them normalisation traps such as `zurich`, `cafe`, `strasse`, `273k`, `habeascorpus`, `andor` |
| not_empty | 0.5% | 250 | `x NOT x`, `x AND y NOT x` or `x OR y NOT x NOT y`, so `NOT` empties a non-empty set |
| or_chain | 0.1% | 50 | 3 to 6 terms, each from band VC (70%) or C (30%), joined by `OR` |
| op_words | 0.5% | 250 | the lowercase words `and`, `or`, `not` used as terms |
| left_assoc | 1% | 500 | three terms with mixed operators: `OR AND`, `AND OR`, `OR NOT` or `NOT OR`. For `OR AND` and `OR NOT`, AND-before-OR precedence would usually give a different answer |
| digits | 0.5% | 250 | a zero-padded token such as `00123`, alone, `OR` its unpadded form, or `AND` another token |
| utf8 | 0.5% | 250 | a token cut out of a non-ASCII word (`rich`, `stanbul`, `273`) `AND` an anchor (see below) from the same document, or alone if it has none |
| upper | 0.5% | 250 | two tokens of an all-caps document |
| repeat | 0.5% | 250 | the word of a repeated run, alone or `AND` a token of the same document |
| punct | 0.5% | 250 | the parts of a punctuation-joined word, joined by `AND` |

**PHRASE queries (19,000 at full).** A phrase has 2 to 8 terms: 2 (35%), 3 (30%), 4 (20%), 5 (9%), 6 (4%), 7 (1%), 8 (1%). An anchor is a token with df ≤ N/1,000 (bands U and R). 15,000 phrases are ordinary. Each is a window of consecutive tokens copied from a random document. First an anchor occurrence is picked uniformly among that document's anchors, then the window position uniformly among the windows that contain it. Every ordinary phrase therefore matches at least one document. The rest are edge cases:

| Kind | Share of all queries | full | What it is |
|---|---|---|---|
| scrambled | 4% | 2,000 | an ordinary window, reversed or with two neighbouring tokens swapped, so every term is present but usually not in that order |
| absent | 0.5% | 250 | an ordinary window with one term replaced by an absent term |
| repeat | 1% | 500 | from a repeated run of `w`: `w w`, the whole run, the run plus one more `w`, or the run with one or two neighbouring tokens |
| utf8 | 0.5% | 250 | a window around the tokens of a non-ASCII word (`z rich`, `s o paulo`) |
| upper | 0.5% | 250 | an anchored window from an all-caps document |
| digits | 0.5% | 250 | a window around a zero-padded token |
| punct | 0.5% | 250 | a window across a punctuation-joined word (`habeas corpus` from `habeas-corpus`) |
| cross | 0.5% | 250 | a window across a sentence boundary `. ` |

Every phrase contains an anchor or an absent term, except some of the `repeat` kind. The order of all 50,000 queries is shuffled. `sample` has exactly one tenth of every `full` count above. `tiny` (1,000 queries) has `max(3, share × 1,000)` of each kind.

Measured on the seed-1 `full` queries:

- BOOL terms by band, over all 91,168 term occurrences: VC 3.5%, C 11.5%, M 29.5%, U 30.0%, R 25.0%, absent 0.5%.
- PHRASE queries: 18,658 of 19,000 (98.2%) contain an anchor, 250 contain an absent term, and the other 92 are `repeat` phrases with neither.
- BOOL answers: 23.4% match nothing. Median count is 8, 90th percentile 2,661, 99th percentile 79,880, largest 234,103 of the 250,000 documents.
- PHRASE answers: 12.1% match nothing. Median count is 1, 99th percentile 5, largest 38.

### 3. Task

Answer every query in `queries.txt` against the documents in `docs.txt`. You read both files in the same run, in any order, whenever you like. You do not have to build a full inverted index; indexing only the terms the queries use is a legitimate strategy. Only `results.txt` is checked.

Let `S(t)` be the set of ids of the documents whose token sequence contains token `t` at least once. A term that occurs nowhere has `S(t) = ∅`. Documents are counted, not occurrences: a document that says `bail` five times is one match.

- **BOOL** `t1 op1 t2 op2 t3 ...` starts with `R = S(t1)` and applies each operator to the running result, strictly left to right, with no precedence:
  - `AND t` gives `R ∩ S(t)`;
  - `OR t` gives `R ∪ S(t)`;
  - `NOT t` gives `R \ S(t)`, the documents of `R` that do not contain `t`. `NOT` is always binary.

  So `a OR b AND c` means `(a OR b) AND c`, and `a NOT b OR c` means `(a NOT b) OR c`. A single-term query is `S(t1)`.
- **PHRASE** `"t1 t2 ... tk"` matches document `d` when its tokens contain `t1, t2, ..., tk` at k consecutive positions `p, p+1, ..., p+k-1`. A repeated term must repeat in the document (`"x x x"` needs three consecutive `x`). Whatever separates two tokens in the text, whether a space, `. `, a hyphen, an em dash or a no-break space, is invisible, so a phrase can match across a sentence boundary. A phrase never spans two documents.
- Empty documents and documents with no token match nothing.

### 4. Output

Write `/out/results.txt`: ASCII, LF line endings, one trailing LF, no trailing spaces, exactly one line per query in the same order as `queries.txt`.

```
<count> <sum> <id_1> <id_2> ... <id_m>
```

- `count` is the number of matching documents, `|R|`.
- `sum` is the exact sum of the ids of all matching documents. It is at most `N(N-1)/2`, under 2^35 at `full`, so a 64-bit integer holds it at every scale.
- `id_1 < id_2 < ... < id_m` are the `m = min(10, count)` smallest matching ids, ascending.
- Fields are separated by one space. Numbers are decimal with no leading zeros. A query with no match prints `0 0`.

#### Worked example

`docs.txt`, where `<TAB>` stands for the single byte 0x09. `ü` is the bytes C3 BC, `—` is E2 80 94 and `§` is C2 A7. Line 2 is an empty document.

```
0<TAB>The surety posted bail; the BAIL BOND was forfeited.
1<TAB>Writ of Habeas-Corpus: see 42 U.S.C. § 1983 (bail bond).
2<TAB>
3<TAB>Zürich bail—bond, bail bail bond. Case 007.
4<TAB>In the matter of the estate. Bond not posted, No. 7.
```

Their tokens:

| id | tokens |
|---|---|
| 0 | the surety posted bail the bail bond was forfeited |
| 1 | writ of habeas corpus see 42 u s c 1983 bail bond |
| 2 | (none) |
| 3 | z rich bail bond bail bail bond case 007 |
| 4 | in the matter of the estate bond not posted no 7 |

`queries.txt`:

```
BOOL bail AND bond
BOOL bail
BOOL estate OR surety AND bail
BOOL bond NOT bail
BOOL bail NOT bail
BOOL zurich
BOOL rich AND z
BOOL not AND bond
BOOL 7
BOOL the OR writ OR case
PHRASE "bail bond"
PHRASE "bail bail bond"
PHRASE "bond bail"
PHRASE "habeas corpus"
PHRASE "u s c 1983"
PHRASE "estate bond"
PHRASE "forfeited writ"
PHRASE "in the matter of"
```

`results.txt`, exactly:

```
3 4 0 1 3
3 4 0 1 3
1 0 0
1 4 4
0 0
0 0
1 3 3
1 4 4
1 4 4
4 8 0 1 3 4
3 4 0 1 3
1 3 3
1 3 3
1 1 1
1 1 1
1 4 4
0 0
1 4 4
```

Line by line:
- `bail` counts document 3 once although it says `bail` three times.
- `estate OR surety AND bail` is `({4} ∪ {0}) ∩ {0, 1, 3} = {0}`. With AND-before-OR precedence it would be `{0, 4}`.
- `zurich` matches nothing, because `Zürich` is `z rich`.
- `7` does not match `007`.
- `"bond bail"` matches only document 3, where `bond, bail` are adjacent across a comma, although documents 0 and 1 contain both words.
- `"estate bond"` matches across `. `.
- `"forfeited writ"` would need to span documents 0 and 1, so it matches nothing.

### 5. Sizes

Measured with seed 1 on an Apple M4 Pro (Python 3.13.12).

| | full | sample | tiny |
|---|---|---|---|
| Documents | 250,000 | 25,000 | 1,000 |
| `docs.txt` bytes | 253,827,587 | 25,263,400 | 1,000,131 |
| Tokens | 36,276,657 | 3,613,148 | 143,405 |
| Distinct tokens | 609,833 | 378,185 | 46,109 |
| (term, document) pairs | 29,798,541 | 2,971,398 | 117,843 |
| Queries (BOOL / PHRASE) | 50,000 (31,000 / 19,000) | 5,000 (3,100 / 1,900) | 1,000 (620 / 380) |
| `queries.txt` bytes | 1,763,534 | 177,987 | 35,793 |
| `results.txt` bytes | 1,459,301 | 109,450 | 11,919 |
| Generation time | 42.3 s | 4.9 s | 0.9 s |
| Reference wall time | 27.2 s | 1.35 s | 0.06 s |
| Reference peak RSS | 2.29 GB | 344 MB | 40 MB |

SHA-256 of the seed-1 files, to check a local run of the generator and of your solution:

| File | full | sample |
|---|---|---|
| `docs.txt` | `6cd461670d6c39f9c5211162577532d3648edeab12340c71cc7d2ab5f18e66c8` | `e157fabb5874c6058a3f6c7726f51df08b2369664cc76c52dde62335ef57eaa4` |
| `queries.txt` | `77ef7ffc59afde051006b02a2ce8e4c090fbbfbf101acd6f862d0f8e298d39f9` | `930ba8716d7a9b9b962b24f5c9d11b5e0b1efb1b9c24f775f889e50c639f16aa` |
| `results.txt` | `f86973c210228141bcb6f1f1b10188ad2a9b65f71e7cd7cf4590b38de4e80927` | `eeeec1e9319da98a6f3c664c78f93e7078fd603c25c2bb3027174e81e3638a8f` |

The query side grows faster than the corpus. Going from `sample` to `full` multiplies the documents and the queries by 10 each, and posting lists grow about 10 times too, so the work per query grows with them. On the reference the query phase is about 14% of the run at `sample` (0.18 s of 1.25 s) and about 56% at `full` (14.9 s of 26.7 s). The live leaderboard runs on the `full` public dataset (seed 1); final scoring runs on a `full`-size holdout.

### 6. Banned libraries

Per rule 02, the library that solves the whole task is banned. For this track that means any ready-made search engine, information-retrieval library or full-text index, embedded or as a service, including:

- Apache Lucene in any form: Lucene, Lucene.NET, PyLucene, Apache Solr, Elasticsearch, OpenSearch.
- Tantivy (and tantivy-py), Xapian, Bleve, Meilisearch, Typesense, Quickwit, Manticore Search, Sphinx, Vespa, ZincSearch, Sonic, Groonga, Bluge.
- Whoosh, PISA, Terrier, Anserini and Pyserini, Indri, Galago, Lemur, MG4J, ATIRE, JASS.
- Full-text modules of databases: SQLite FTS3, FTS4 and FTS5, PostgreSQL `tsvector`/`tsquery` and `pg_trgm`, MySQL and MariaDB `FULLTEXT`, the DuckDB `fts` extension, RediSearch.
- JavaScript search libraries: Lunr, elasticlunr, FlexSearch, MiniSearch, Fuse.js, Orama.
- A SQL engine (SQLite, DuckDB, PostgreSQL and the like) used to store postings and evaluate the boolean or phrase logic in SQL counts as a search engine.

Everything else is allowed and priced by the D axis: hash maps, sorting, regular expressions, multi-pattern matchers, bitmap and posting-compression libraries (Roaring, StreamVByte, FastPFor), memory-mapped files, thread pools.

### 7. Reference behaviour

`reference/b_index.py`, single-threaded Python, standard library only:

- Reads `docs.txt` line by line, tokenizes each document with the regular expression above, and adds the document id to a Python `set` per distinct term. That is one set for every term in the corpus, 609,833 at `full`. It also keeps every document's raw text in memory.
- Answers `BOOL` queries with Python set operations applied in the order written. Every operation builds a new set.
- Answers `PHRASE` queries by intersecting the terms' sets in phrase order to get candidates, then re-tokenizing each candidate document from its raw text and scanning it for the exact sequence. It stores no positions.
- Formats each answer by sorting the whole result set, then printing its length, its sum and its first ten ids.

At `full` on the M4 Pro it takes 27.2 s wall (27.1 s on a second run): about 11.9 s to read and index the corpus and 14.9 s to answer the queries. Peak RSS is 2.29 GB, under the 4 GB ceiling. Most of that is the posting sets, which hold 29.8 million (term, document) pairs, plus 254 MB of raw text.

### 8. Changes from the site

The site's Track B entry (`src/data/tracks.json`) differs from this spec in these places:

1. **One time, not two.** The site says build and query time are measured separately and combined 40/60 toward queries. The harness times one container run, and there is no trustworthy way to see where a program's build ends and its queries begin, so T is the total wall time. The queries are in the input from the start.
2. **The hint "The build/query split is published in the spec so you can plan the trade"** no longer applies: there is no split to publish. The trade between indexing work and query work now lives inside one run, and this spec publishes the query distribution (section 2.5) so you can plan it.
3. **The sample query `BOOL indemnity AND (nothing) NOT surety`** is `BOOL indemnity AND nothing NOT surety` here. The grammar has no parentheses anywhere, which matches the site's own "no parentheses" note.
4. **"Build an inverted index"** is how most teams will do it, not a requirement. Any method that writes the right `results.txt` is allowed, including indexing only the query terms.
5. **Stopwords** are retained everywhere, in `BOOL` queries as well as phrases. The site only says so for phrases.
6. **The numbers.** The site's `spec` numbers are the final scoring-box figures: `dataset_mb` 256 (a 253.8 MB `docs.txt` plus a 1.8 MB `queries.txt`), the box-calibrated `reference_s`, and `timeout_s` at ten times it. The query count (50,000) and the 62% / 38% mix are unchanged. (On an Apple M4 Pro the reference runs in about 27.2 s; the scoring box differs.)
7. **Added here** because the site does not define them: the corpus format, the tokenizer, the output format, the query distribution and the edge cases.

---

# Track spec: specs/c-join.md (github.com/hackathon-raptors/speed-demon-2026)

## Track C: Join

The correctness track. The fast path here is easy and the edge cases are the actual task.

### 1. Summary

Parse `records.csv`, a hostile CSV file of about 200 MB, with quoted commas, escaped quotes, embedded newlines, mixed CRLF and LF line endings, and a UTF-8 byte order mark at the start of some chunks and not others. Join every record against `regions.csv`, a plain lookup table that maps a `region_id` to a three-letter country code. Then write one line of aggregates per country, sorted. Every edge case the file can contain is listed in section 2.5, appears in every dataset at every scale, and is already visible in the sample. The parsing rules in section 2.1 are not negotiable: a parser that is three times faster and wrong scores zero.

### 2. Input files

Both files are in `/data` (read-only): `records.csv` and `regions.csv`. Nothing else is there.

#### 2.1 `records.csv`: parsing rules

These rules define what every byte sequence means. Section 2.2 then narrows them to what the generator actually writes, and you may rely on both.

```
file        = [bom] header *( [bom] record )
header      = "id,name,note,region_id,amount" terminator
record      = id "," text "," text "," region-id "," amount terminator
terminator  = CRLF / LF                      ; %x0D.0A or %x0A; may be omitted on the final record
id          = 1*DIGIT                        ; ASCII 0-9 only
region-id   = 1*DIGIT
amount      = [ "-" ] 1*DIGIT "." 2DIGIT     ; -?[0-9]+\.[0-9]{2}
text        = quoted / unquoted
quoted      = DQUOTE *( qbyte / DQUOTE DQUOTE ) DQUOTE
qbyte       = any byte except DQUOTE         ; commas, LF, CR, U+FEFF are all literal here
unquoted    = *( any byte except DQUOTE, ",", CR, LF )
bom         = %xEF.BB.BF                     ; U+FEFF
```

- **Columns.** Every record has exactly five fields, in the header's order: `id,name,note,region_id,amount`. The header appears once, as the first record.
- **Quoting.** A field that starts with `"` is quoted. It ends at the next `"` that is not part of a `""` pair, and that closing quote must be followed by `,` or by a record terminator. The value of a quoted field is the bytes between its delimiting quotes, with every `""` replaced by one `"`. Inside quotes, every other byte is literal data, including `,`, LF and U+FEFF. A field that does not start with `"` is unquoted, and its value is its bytes as they are.
- **Empty values.** An empty unquoted field (nothing between the commas) and an empty quoted field (`""`) both have the empty string as their value. They are the same value.
- **Records.** A record terminator is a CRLF or a bare LF that is outside quotes. An LF inside a quoted field is part of the value and does not end the record. The final record may end at end of file without a terminator. (The generator always writes one, see 2.2.) A terminator at the very end of the file does not start another record. An empty line is not a record and never occurs.
- **Byte order marks.** A single U+FEFF (bytes `EF BB BF`) at the very start of the file, or immediately after a record terminator, is a byte order mark. It is not data and is skipped. A U+FEFF anywhere else is data. Inside a quoted field it is three bytes of the value. A second U+FEFF directly after a skipped one would be data at the start of the `id` field, which makes the record invalid. The generator never writes that.
- **Values.** `id` and `region_id` are decimal integers. `amount` is a money value in exact integer cents: sign × (integer part × 100 + the two fraction digits). For example, `-5.25` is -525 cents and `0.00` is 0. `name` and `note` are byte strings compared byte for byte, with no trimming, case folding or Unicode normalisation. The text is UTF-8.

#### 2.2 `records.csv`: what the generator writes

These hold for every seed and every scale, including the holdout.

- **Start and end.** The file starts with a BOM, then the header terminated by CRLF: `EF BB BF` `id,name,note,region_id,amount` `0D 0A`. It ends with the last record's terminator, which is always CRLF. The header appears exactly once.
- **BOMs.** Mid-file BOMs occur only at the start of a record. They never directly follow the header, never come two in a row, and never sit at end of file. Think of the file as chunks concatenated from different exports: some chunks start with a BOM and the rest don't.
- **Terminators.** Each record independently ends with CRLF or LF, about half and half. No CR appears anywhere except as the first byte of a CRLF terminator. In particular there is no CR inside a quoted field and no CR-only line ending.
- **Bytes.** Apart from the BOMs, every byte is printable ASCII (0x20 to 0x7E), LF or CR. There are no other control bytes and no non-ASCII text.
- **`id`.** Always unquoted. Matches `[1-9][0-9]*`. Ids are unique and strictly increasing through the file, usually by 1 and sometimes by 2 to 11.
- **`region_id`.** Always unquoted. Matches `[1-9][0-9]*` and is at most 9,999,999. About 99.6% of records reference a region that is in `regions.csv`. The rest reference either an id inside the lookup's range that the lookup does not contain, or an id above the range (10,000,000 is never reached).
- **`amount`.** Always unquoted. Matches `-?(0|[1-9][0-9]*)\.[0-9]{2}` (no leading zeros) and is never `-0.00`. The range is -999.99 to 99,999.99. About 4% of amounts are negative, some of them between -0.99 and -0.01, and about 2% are `0.00`.
- **`name`.** Either empty (about 0.5% unquoted empty and 0.5% `""`) or a quoted non-empty value. Non-empty names are always quoted, even when they don't need to be, as `"Betts"` is in the site's preview. A name is made of letters and spaces. It may contain a comma (`Kaphar, Titus`) or double quotes (`Ann "Annie" Lee`, written `"Ann ""Annie"" Lee"`), and may end with one or more trailing spaces. Seed 1 has runs of up to six at full scale, and there is no fixed upper bound. It never starts with a space and never contains an LF.
- **`note`.** Either empty (about 1.5% unquoted empty and 1.5% `""`) or a quoted non-empty value. A note is made of lowercase words separated by single spaces or by LFs. It may contain a comma, a quoted word (`said "no" twice`) or a lone `"`, and may end with one trailing space (never more). It has at most four lines (at most three LFs). It never starts or ends with an LF, never has two LFs in a row, never has a space right after an LF, and never starts with a space. A note can begin or end with a quote character, so a quoted field can begin with `"""` or end with `"""`.
- **Distribution (for planning, not a guarantee).** Region popularity is skewed: half the records pick a referenced region uniformly, and half pick by Zipf with weight 1/rank over a random ranking of the regions. Names are drawn uniformly from a pool of 100,000 distinct values, about 5% of which differ from another pool name only by trailing spaces. Notes are drawn uniformly from a pool of 60,000. tiny uses pools of 400 and 300.

#### 2.3 `regions.csv`

The lookup table is deliberately plain. It has no hostile features.

- ASCII, LF line endings, no BOM, no quotes, no spaces. The header line is `region_id,country`, followed by one line per region, `region_id,country`. The last line ends with LF.
- `region_id` matches `[1-9][0-9]*`. Region ids are unique and listed in ascending numeric order. `country` matches `[A-Z]{3}`.
- At full and sample scale there are 200,000 regions with ids between 1 and 999,999, spread over 240 countries (about 830 regions per country). tiny shrinks this to 2,000 regions between 1 and 9,999 and 24 countries, so a test can read it by eye. full and sample share every other parameter.
- Country codes are synthetic and change with the seed. One country has regions in the table that no record references. Another country's records all carry amounts between -0.99 and -0.01.

#### 2.4 Scales

| scale | records | lookup | use |
|---|---|---|---|
| `full` | 3,320,000 | 200,000 regions, 240 countries | scoring |
| `sample` | 332,000 (10% of full) | same shape as full | published with its expected hash |
| `tiny` | 1,000 | 2,000 regions, 24 countries | tests |

`sample` is drawn from the same distribution as `full`, from its own random stream: it is not a prefix of `full` and does not share its lookup.

#### 2.5 Edge cases

Every case below is written on purpose. The generator inserts one block of records containing all of them per 25,000 records, and never fewer than four blocks: that is 4 blocks in tiny, 13 in sample and 132 in full. Ordinary records produce most of them again at the rates in 2.2. `tests/test_c_join.py` asserts that each one is present in tiny and in sample, using the names in brackets.

Encoding:

1. A BOM at the start of the file `[bom_at_file_start]`.
2. A BOM at a mid-file record boundary `[bom_mid_file]`: one after a CRLF-terminated record `[bom_after_crlf]` and one after an LF-terminated record `[bom_after_lf]`.
3. A record directly after a mid-file BOM that ends with CRLF `[crlf_record_after_bom]`, and one that ends with LF `[lf_record_after_bom]`.
4. CRLF and LF terminators mixed in the same file `[crlf_terminator, lf_terminator]`.
5. A quoted field containing a comma, in a name `[name_comma]` and in a note `[note_comma]`.
6. A quoted field containing `""` escapes, in a name `[name_escaped_quote]` and in a note `[note_escaped_quote]`.
7. A note containing one bare LF, so the record spans two lines `[note_two_lines]`.
8. A note spanning several lines (two or three LFs) `[note_several_lines]`.
9. A note containing a comma, a `""` escape and an LF together `[note_comma_quote_lf]`.
10. An empty unquoted field, in a name `[name_empty_unquoted]` and in a note `[note_empty_unquoted]`.
11. An empty quoted field `""`, in a name `[name_empty_quoted]` and in a note `[note_empty_quoted]`.
12. A name with trailing spaces inside the quotes `[name_trailing_space]`.
13. The last record of the file, terminated by CRLF `[last_record_crlf]`.

Values and the join:

14. Negative amounts `[negative_amount]`, including some between -0.99 and -0.01 `[negative_amount_under_one]`.
15. Zero amounts, written `0.00` `[zero_amount]`.
16. An unknown `region_id` inside the lookup's id range, a hole `[unknown_region_in_range]`, and one above it `[unknown_region_above_range]`.
17. Two names in one country that differ only by a trailing space, `Betts` and `Betts ` `[trailing_space_twins_in_one_country]`.
18. An empty unquoted name and an empty quoted name in one country `[both_empty_names_in_one_country]`.
19. A country in `regions.csv` with no records at all `[country_without_records]`.
20. A country whose records all have amounts between -0.99 and -0.01, so its sum, min and max are all negative `[country_all_negative_under_one]`.

### 3. Task

1. Parse `records.csv` per section 2.1.
2. Join each record to `regions.csv` on `region_id`, compared as integers. The record's group is that region's `country`. If `region_id` is not in `regions.csv`, the record's group is `_UNMATCHED`.
3. For every group that has at least one record, compute:

| column | definition |
|---|---|
| `count` | number of records in the group |
| `id_sum` | sum of the group's `id` values. The BOM always sits in front of an `id`, so this column is what makes BOM handling visible in the output. |
| `amount_sum` | sum of the group's amounts, in cents |
| `amount_min` | smallest amount in the group, in cents |
| `amount_max` | largest amount in the group, in cents |
| `names_distinct` | number of distinct `name` values in the group, compared as exact byte strings after unescaping. `Betts` and `Betts ` are two values. `Ann "Annie" Lee` is compared with its quotes unescaped. The empty unquoted name and the empty quoted name are both the empty string and count as one value. |
| `name_bytes` | sum of the UTF-8 byte lengths of the group's `name` values after unescaping. The delimiting quotes count 0, each `""` counts 1, and trailing spaces count. Distinct counting alone would not catch a parser that fails to unescape, because that mistake maps names one to one; this column catches it. |
| `note_bytes` | sum of the UTF-8 byte lengths of the group's `note` values after unescaping. An embedded LF counts 1, each `""` counts 1, and an empty note counts 0. |

4. A country that is in `regions.csv` but has no records produces no line. `_UNMATCHED` produces a line if and only if at least one record has an unknown `region_id`. (In generated data that is always the case.) Group keys are unique, so there are no ties.

**Magnitudes.** `id_sum` and `amount_sum` exceed 2^32 for most groups in `full`. They always fit in a signed 64-bit integer: ids are below 4 × 10^7, so `id_sum` is below 1.4 × 10^14, and `|amount_sum|` is below 3.4 × 10^13 cents. The other columns are below 2^31.

### 4. Output

Write exactly one file, `/out/aggregates.txt`.

- One line per group: `country count id_sum amount_sum amount_min amount_max names_distinct name_bytes note_bytes`. Fields are separated by a single space. No key can contain a space, because codes are `[A-Z]{3}` and the unmatched key is `_UNMATCHED`. There is no header line.
- **Order.** Lines are sorted by key, bytewise ascending. `_UNMATCHED` sorts after every country code because `_` (0x5F) is greater than `Z` (0x5A), so it is always the last line.
- **Integers** (`count`, `id_sum`, `names_distinct`, `name_bytes`, `note_bytes`) are in decimal with no leading zeros and no plus sign.
- **Money** (`amount_sum`, `amount_min`, `amount_max`) is integer cents printed as: `-` if negative, then the integer part with no leading zeros (`0` when the absolute value is below 1.00), then `.`, then exactly two digits. For example -525 is `-5.25`, -5 is `-0.05`, 0 is `0.00` (never `-0.00`) and 142000 is `1420.00`. No floating point is involved anywhere.
- UTF-8 (in practice ASCII), LF after every line including the last, no BOM, no trailing spaces.

#### Worked example

In the notation the site uses, ⟨BOM⟩ is the bytes `EF BB BF`, ␍␊ is CRLF and ␊ is a bare LF. The first five lines of this `records.csv` are the site's preview, byte for byte. The same bytes are `WORKED_RECORDS` in `tests/test_c_join.py`.

`records.csv`:

```
⟨BOM⟩id,name,note,region_id,amount␍␊
1,"Kaphar, Titus","said ""no"" twice",7,1420.00␍␊
2,"Betts","a note that␊spans two lines",7,88.50␊
3,,"",12,0.00␍␊
4,"trailing space ","comma, quote "" and newline␊together",3,-5.25␊
⟨BOM⟩5,"Betts ","one␊two␊three",12,-0.75␍␊
6,"",,12,10.00␍␊
⟨BOM⟩7,"Ann ""Annie"" Lee","x",5,2.00␊
8,"Nobody","",4000000,-1.00␍␊
9,"Neg","n",20,-0.05␍␊
```

`regions.csv`:

```
region_id,country␊
3,BBB␊
7,AAA␊
12,AAA␊
20,CCC␊
21,DDD␊
```

`aggregates.txt`, with every line ending in LF:

```
AAA 5 17 1517.75 -0.75 1420.00 4 24 55
BBB 1 4 -5.25 -5.25 -5.25 1 15 35
CCC 1 9 -0.05 -0.05 -0.05 1 3 1
_UNMATCHED 2 15 1.00 -1.00 2.00 2 21 1
```

How the lines come about:

- **AAA** (regions 7 and 12) holds records 1, 2, 3, 5 and 6.
  - The ids sum to 17.
  - The amounts sum to 142000 + 8850 + 0 - 75 + 1000 = 151775 cents.
  - The names are `Kaphar, Titus`, `Betts`, the empty name (unquoted in record 3, quoted in record 6, the same value), and `Betts ` (record 5, with a trailing space). That is 4 distinct values, with 13 + 5 + 0 + 6 + 0 = 24 bytes.
  - The notes are 15 bytes (`said "no" twice`), 27 (one LF), 0, 13 (two LFs) and 0 (unquoted empty), so 55 bytes.
  - The BOM before record 5 follows an LF terminator, and record 5 ends with CRLF.
- **BBB** holds only record 4. Its name has a trailing space (15 bytes), and its note `comma, quote " and newline` LF `together` is 35 bytes.
- **CCC** holds only record 9. Its amount is -0.05, so the sum, the min and the max are all `-0.05`.
- **DDD** (region 21) has no records and produces no line.
- **_UNMATCHED** holds record 7 and record 8:
  - Record 7 has region 5, which is not in the table although it lies between 3 and 21. Its name `Ann "Annie" Lee` is 15 bytes once unescaped.
  - Record 8 has region 4000000, above the table's range.
  - The BOM before record 7 follows a CRLF terminator.

### 5. Sizes

Measured from the generator with seed 1 on an Apple M4 Pro. Record counts are exact for every seed. Byte counts vary by a fraction of a percent between seeds.

| | full | sample |
|---|---|---|
| records | 3,320,000 | 332,000 |
| `records.csv` | 201,935,059 B | 19,846,456 B |
| `regions.csv` | 2,177,799 B (200,000 regions, 240 countries) | 2,177,777 B (200,000 regions, 240 countries) |
| dataset total | 204,112,858 B (204.1 MB) | 22,024,233 B (22.0 MB) |
| mid-file BOMs | 777 | 83 |
| edge-case blocks | 132 | 13 |
| records with an unknown `region_id` | 13,475 | 1,298 |
| records whose note contains an LF | 276,857 | 27,212 |
| `aggregates.txt` | 240 lines, 17,054 B | 240 lines, 15,415 B |
| generation time | 10.5 s | 1.6 s |

Seed-1 sha256 values, as measured when this spec was written. The hashes published with the datasets are authoritative.

| file | sha256 |
|---|---|
| full `records.csv` | `36166f4ac44101a22ebf25b4f949436a681a2698cb0f2ae1f7f6853010259f48` |
| full `regions.csv` | `414c57c4f3ec968f9bce28c59eebfdf57a9425322b468db7ee964c18c4852fe1` |
| full `aggregates.txt` | `d8088b1cd42ba1338b1345b107b55db7b96ef1f5066f3ff64fd26a8912a0f536` |
| sample `records.csv` | `07bc636857b70556921be08d406f887eb07dbdacb8384587ed7fd3b52addfd86` |
| sample `regions.csv` | `1bd3904fd5eaed7a4dc1ae761bf369521ce5f384e4c400342919d6edf9e5ece1` |
| sample `aggregates.txt` | `9035fed36823232dd41ae2d1b9d6d21aa7c73f49d8e95647e953a89d5ebe9a66` |

Reference on `full`, on the same machine: 6.0 s wall and about 400 MB peak RSS. The scoring-box figure, and the timeout of 10× that figure, are published after calibration.

### 6. Banned libraries

Per rule 02, no analytics dataframe library in Track C. Concretely, you may not use any dataframe library or SQL or query engine, in any language, whether or not it ships with your language's standard library. That includes:

- pandas, Polars, DuckDB
- Apache Arrow in any binding, including its CSV reader and its compute kernels: pyarrow, arrow-rs and arrow-csv, Arrow C++, Acero
- Apache DataFusion, Apache Spark and PySpark, Dask, Modin, Vaex, cuDF and the rest of RAPIDS
- ClickHouse and chDB
- SQLite, including Python's `sqlite3` module and SQLite's CSV virtual table
- R's data.table and dplyr
- Julia's DataFrames.jl
- Go's gota and dataframe-go

If a library's job is to perform a relational join or group-by for you, it is banned even if it is not named here.

CSV parsing libraries are allowed: for example Rust's `csv` and `csv-core`, C's `libcsv` or C++'s `rapidcsv`. A non-standard-library CSV parser is a dependency like any other and is charged on the D axis. A CSV module in your language's standard library (Python's `csv`, Go's `encoding/csv`) costs nothing there. Whatever you use, its output must follow section 2.1 exactly. Most CSV libraries have options that change these rules, such as trimming, comment lines, header handling and BOM handling, and getting them wrong fails the hash.

### 7. Reference behaviour

`reference/c_join.py` is the obvious Python version: correct, single-threaded and readable.

- It loads `regions.csv` into a dict mapping `region_id` to `country`.
- It opens `records.csv` as UTF-8 text with `newline=""` and hands it to Python's `csv` module (excel dialect), which returns one list of five `str` per record.
- It removes one leading U+FEFF from the first field of every record, then checks the header.
- It validates `id` and `region_id` with a regular expression and converts them with `int()`, and converts `amount` to integer cents with a regular expression.
- It looks up each record's `region_id` in the dict, a hash join, one record at a time.
- It keeps one Python object per country holding running count, sums, min and max, a `set` of names, and byte counts. It gets the byte counts by encoding every name and note back to UTF-8.
- When the file is done, it sorts the roughly 240 keys and writes the lines.

Where the time goes:

- every byte is decoded into Python `str` objects and then re-encoded to count bytes;
- every record costs a new list, five new strings and a dozen interpreted function calls;
- everything runs on one core.

### 8. Changes from the site

Compared with the Track C entry on https://speeddemon.dev (`src/data/tracks.json`):

1. **`spec.dataset_mb` 204 → 204.1.** That is the measured total of both files (records.csv 201.9 MB, regions.csv 2.2 MB). It is effectively unchanged.
2. **`spec.records` "hostile CSV" → 3,320,000 records.**
3. **`spec.reference_s` is the box-calibrated reference,** with `timeout_s` at ten times it. On an Apple M4 Pro the reference runs in about 6.0 s, below the conventions' 20 to 60 s band: Python's `csv` module parses in C at about 34 MB/s, and a csv-module reference would reach 20 s only at roughly three times the published dataset size, so the size was kept at 204 MB. The scoring box differs.
4. **The lookup table is named and defined.** The site says "join it against a lookup table" without naming one. It is `regions.csv` (section 2.3), and it is deliberately plain.
5. **"Emit sorted aggregates" is defined** (sections 3 and 4). `id_sum` and `name_bytes` are there so that BOM handling and unescaping both change the hash.
6. **"A UTF-8 BOM on some chunks and not others" is made precise.** The file always starts with a BOM. Mid-file BOMs appear only at record boundaries (section 2.2), and the BOM rule is stated exactly in 2.1.
7. **The preview's values are illustrative.** Its five lines are the start of the worked example in section 4, not lines of the generated file. Generated ids reach seven digits, region ids reach seven digits, and names and notes are synthetic. The site's promise that every hostile feature is visible in those five lines holds. The generated file uses no feature beyond them: no repeated header, no CR-only line endings, no CR inside quotes, no quoted numeric fields, no unquoted non-empty text and no non-ASCII text. The one thing the preview does not show is the end of the file, which is defined in 2.1 and fixed in 2.2: the last record is always terminated, by CRLF.

---

# Track spec: specs/d-traverse.md (github.com/hackathon-raptors/speed-demon-2026)

## Track D: Traverse

The algorithms track. Generator `generators/d_traverse.py`, reference `reference/d_traverse.py`, tests `tests/test_d_traverse.py`.

### 1. Summary

The input is a directed graph with non-negative integer weights, built like a road network: 1,740,512 nodes and 5,000,000 edges, followed by 2,000 source-target queries. For each query, print the length of the shortest directed path from source to target, or `-1` if there is none. Preprocessing is explicitly allowed and is counted inside the timed run: the clock covers reading the files, building whatever you build, answering every query and writing the answers.

### 2. Input files

Two files in `/data`, both read-only: `graph.txt` and `queries.txt`.

Both are ASCII (and so valid UTF-8), with LF line endings, a single space between fields, one trailing newline, no blank lines, no BOM and no trailing spaces. Every number is an unsigned decimal integer with no sign and no leading zeros (zero is written `0`).

#### `graph.txt`

```
graph   = header edge*
header  = n SP m LF
edge    = u SP v SP w LF        ; exactly m of these
```

| Field | Range | Meaning |
|---|---|---|
| `n` | 1,740,512 at `full` | Number of nodes. Node ids are dense: every integer in `[0, n)` is a node, including ids that appear on no edge line. |
| `m` | 5,000,000 at `full` | Number of edge lines that follow. |
| `u` | `0 <= u < n` | Tail of a directed edge. |
| `v` | `0 <= v < n` | Head of the edge. |
| `w` | `0 <= w <= 4294967295` | Weight, a uint32. **Zero is a legal weight.** |

Edge lines are in no particular order: not sorted by `u`, `v`, `w` or anything else. Each line is one directed edge from `u` to `v`. These all occur, at every scale (counts per scale in section 5):

- **Reverse edges.** If both `u -> v` and `v -> u` exist they are separate lines, usually with different weights (`0 918442 37` and `918442 0 41`). About 3% of directed node pairs have no reverse edge at all (one-way streets).
- **Parallel edges.** The same `(u, v)` appears on two or more lines with different weights. In some groups the lighter line comes first in the file, in others the heavier one does.
- **Exact duplicate lines.** The same `u v w` appears more than once.
- **Self-loops.** Lines with `u == v`.
- **Weight 0.** Including paths made only of weight-0 edges, so `s != t` can be at distance 0.
- **Very large weights.** Weights of exactly 4,294,967,295 and weights within 2^20 of it.
- **Nodes with no edges at all**, **nodes with no in-edges** and **nodes with no out-edges**.
- **Parts of the graph that nothing else connects to**, and so pairs of nodes with no path between them.

#### `queries.txt`

```
queries = query*                 ; 2,000 lines at full scale, no header
query   = s SP t LF
```

`0 <= s < n` and `0 <= t < n`. The query count is the number of lines. These all occur, at every scale:

- `s == t`, including on a node that has no edges.
- `s != t` with distance 0.
- Unreachable targets, answered `-1`: from a source with no out-edges, from a node with no edges, from a component that cannot reach the target, and to a target with no in-edges.
- Distances of exactly 4,294,967,295 and distances above it.
- Duplicate query lines (the same `s t` more than once).

#### How the data is generated

The generator is deterministic for a given seed and scale (seed 1 is the development seed; the scoring seed is private). Every scale is built from the same distribution; `sample` is not a prefix of `full`. To make your own: `python3.13 generators/d_traverse.py --seed <int> --scale {full,sample,tiny} --out <dir>` (`full` takes under a minute on a laptop).

**Main body: a road network.** All but a few hundred nodes sit on a jittered square lattice. Arterial roads run along every 8th row and column, highway links join every 6th node along every 48th row and column, and local streets and some diagonals fill in the rest. The arterials, the highways and a random spanning forest of local streets are two-way, so **the main body is strongly connected**: every main-body node can reach every other. About 85% of the remaining local streets are two-way, the rest one-way. A weight is a travel time: straight-line length divided by the road's speed (highways about 5x and arterials about 2.5x faster than local streets, 3% of local streets 3 to 12x slower), with independent noise of up to 12% per direction. Main-body weights are between 1 and a few thousand (median about 120), apart from the injected weight-0 edges and parallel copies.

**Special structures**, outside the main body, at fixed rates:

| Structure | Count | Edges |
|---|---|---|
| Isolated nodes | `max(2, n // 100000)` | none |
| Nodes with no in-edges | `max(2, n // 100000)` | two out-edges each, into the main body, weights in `[2^32 - 2^20, 2^32 - 1]` |
| Nodes with no out-edges | `max(2, n // 100000)` | two in-edges each, from the main body, weights 1 to 500 |
| Islands | `max(1, n // 200000)` | 14 nodes and 33 edges each, no edge to or from anything else: a two-way ring of 12, a weight-0 path, two parallel pairs, a self-loop, and a two-edge spur of weights near 2^32 whose last node has no out-edges |

Inside the main body, chosen at random: `max(2, m // 1000)` extra parallel edges (each a copy of an existing edge with a different weight, lighter or heavier), `max(1, m // 5000)` exact duplicate lines, `max(1, m // 10000)` self-loops, and `max(1, m // 2000)` edges whose weight is set to 0.

Node ids are a random relabelling, so an id says nothing about where a node is in the network, and edge lines are shuffled.

**Queries.** Each line is one of the kinds below, in random order. Counts are `max(minimum, q * per_mille // 1000)`; every query that is not injected is normal.

| Kind | Per 1000 | Min | s | t | Answer |
|---|---|---|---|---|---|
| normal | rest | | uniform over the main body | by Dijkstra rank, below | distance |
| same node | 2 | 1 | main body | `= s` | 0 |
| same isolated | 1 | 1 | node with no edges | `= s` | 0 |
| zero path | 1 | 1 | island | island, via weight-0 edges | 0 |
| parallel min | 2 | 2 | island | island, decided by the lighter of two parallel edges (one group listed lighter first, one heavier first) | distance |
| max weight | 1 | 1 | island | spur | 4294967295 |
| over uint32 | 1 | 1 | island | spur end | above 4294967295 |
| from no-in node | 1 | 1 | node with no in-edges | by Dijkstra rank | at least 4293918720 |
| to no-out node | 1 | 1 | main body, a tail of one of t's in-edges | node with no out-edges | distance |
| unreachable, island | 2 | 1 | island | main body | -1 |
| unreachable, no-out source | 1 | 1 | node with no out-edges | main body | -1 |
| unreachable, isolated source | 1 | 1 | node with no edges | main body | -1 |
| unreachable, no-in target | 1 | 1 | main body | node with no in-edges | -1 |
| duplicate | 10 | 1 | a copy of another query line, never of an unreachable no-in-target one | | same as the original |

**Query distance.** A normal query's target is chosen by Dijkstra rank: an octave `o` is drawn uniformly from 1 to 16, then a rank `r` uniformly from `[2^o, 2^(o+1) - 1]`, and `t` is the `r`-th node that a Dijkstra search from `s` settles (`s` itself is settled first; ties are broken arbitrarily). So `t != s`, `t` is always reachable, `log2(r)` is close to uniform between 1 and 17, the median `r` is 512, the mean is about 12,300 and the maximum is 131,071. At `tiny` scale `r` is capped at the main-body size minus one. The distribution is the same at every scale, so a `sample` query is as long as a `full` one; `sample` simply has a tenth as many of them on a tenth of the map.

### 3. Task

For each query `(s, t)`, compute `d(s, t)`: the minimum, over all directed paths from `s` to `t`, of the sum of the path's edge weights.

- A path may have zero edges, so `d(s, s) = 0` for every node, including nodes with no edges and nodes with a self-loop.
- If no directed path from `s` to `t` exists, the answer is `-1`.
- Every edge line is an edge. With parallel edges, a shortest path uses the lightest one; duplicate lines change nothing. A self-loop never shortens a path.
- Weight 0 is an ordinary weight. All weights are non-negative.
- Direction matters: `d(s, t)` and `d(t, s)` are independent.
- A duplicated query line is answered again, with the same answer, on its own line.
- Answers are exact integers and can exceed 2^32 - 1. A shortest path has at most `n - 1` edges, so every answer is at most `(n - 1) * (2^32 - 1) < 2^63` and fits in a signed 64-bit integer.

**Preprocessing is allowed and counted.** The harness times the whole process, from exec to exit, so parsing, any index or ordering you build, the queries and the output are all inside your time. You may answer queries in any order and in parallel; only the output order is fixed. Nothing derived from the input may be computed ahead of the run (site rule 04): the scoring graph is generated at a seed you have not seen.

### 4. Output

One file, `/out/distances.txt`, ASCII, LF line endings, one trailing newline, nothing else.

- Exactly one line per query line, in input order: line `i` answers line `i` of `queries.txt`.
- Each line is `d(s, t)` in decimal with no leading zeros and no sign (`0` for zero), or the two characters `-1` when `t` is unreachable from `s`.
- The answer is fully determined by the input, so there are no ties or rounding to define. The SHA-256 of this file is the correctness gate.

#### Worked example

`graph.txt` (8 nodes, 12 edge lines):

```
8 12
0 1 4
1 2 3
0 2 9
2 0 5
2 1 6
1 3 0
3 4 0
4 4 7
4 5 4294967295
2 1 2
5 6 4294967295
0 1 4
```

`queries.txt`:

```
0 2
2 1
1 4
4 4
7 7
0 6
4 5
6 0
0 7
1 0
2 1
```

`distances.txt`, exactly these 42 bytes (SHA-256 `2e9871bba1d5013144ceff48d806a28da0e3f203b29118a89d98d8f654c3f6a0`):

```
7
2
0
0
0
8589934594
4294967295
-1
-1
8
2
```

| Query | Answer | Why |
|---|---|---|
| `0 2` | 7 | `0->1->2` is 4 + 3, shorter than the direct edge of 9. The duplicate line `0 1 4` changes nothing. |
| `2 1` | 2 | Parallel edges `2 1 6` and `2 1 2`: the lighter one counts, though it is listed second. |
| `1 4` | 0 | `1->3->4` uses two weight-0 edges, so `s != t` at distance 0. |
| `4 4` | 0 | `s == t`. The self-loop `4 4 7` is never part of a shortest path. |
| `7 7` | 0 | Node 7 is on no edge line but is still a node, because `n = 8`. |
| `0 6` | 8589934594 | 4 + 0 + 0 + 4294967295 + 4294967295, above 2^32 - 1. |
| `4 5` | 4294967295 | One edge of the maximum weight. |
| `6 0` | -1 | Node 6 has no out-edges. |
| `0 7` | -1 | Node 7 has no in-edges. |
| `1 0` | 8 | `1->2->0` is 3 + 5; edge `2 0 5` has no reverse, so `0 2` and `2 0` differ. |
| `2 1` | 2 | A duplicate query is answered again. |

### 5. Sizes

Measured from the generator, seed 1.

| Scale | Nodes `n` | Edges `m` | Queries | `graph.txt` | `queries.txt` | `distances.txt` |
|---|---|---|---|---|---|---|
| `full` | 1,740,512 | 5,000,000 | 2,000 | 91,950,247 B | 29,401 B | 9,039 B |
| `sample` | 174,051 | 500,000 | 200 | 8,194,650 B | 2,535 B | 908 B |
| `tiny` | 348 | 1,000 | 40 | 11,056 B | 296 B | 181 B |

`sample` is 10% of `full` in nodes, edges and queries, from the same distribution.

SHA-256, seed 1:

| File | `full` | `sample` |
|---|---|---|
| `graph.txt` | `fe6471f2712d10b23c2ec58a4cbc7db1ea3e742daf6b6db62829e74d6c4a26c7` | `16351d408f1f94a6bcf66de5c5762abede111c07a4a38306253504a6c9c9b7cd` |
| `queries.txt` | `160f1ef48378dcb567a8d9ddd2676ee777b044a5ede3ad4c7b42e812aa4b3a6f` | `fb4065ffe94d2dd51c924f389bd0c6afa58d42534391aa5b0fe5edfaa33988a5` |
| `distances.txt` (expected) | `a23dbdf0a3e49cc8901310a9ee1dc6a9a569a85297b6da75c2a7002cb2ed277e` | `a4c76ed030bc7e365db33191f6cf74708fba7f25eb48d64c8c4279f2e57046a0` |

Structure per scale:

| | `full` | `sample` | `tiny` |
|---|---|---|---|
| Main-body nodes (one strongly connected component) | 1,740,349 | 174,031 | 328 |
| Isolated nodes | 17 | 2 | 2 |
| Nodes with no in-edges | 17 | 2 | 2 |
| Nodes with no out-edges (incl. one per island) | 25 | 3 | 3 |
| Islands | 8 | 1 | 1 |
| Injected parallel groups (main body + 2 per island) | 5,016 | 502 | 4 |
| Injected exact duplicate lines | 1,000 | 100 | 1 |
| Self-loops (main body + 1 per island) | 508 | 51 | 2 |
| Weight-0 edges injected (main body + 2 per island) | 2,516 | 252 | 3 |

A lighter parallel copy can also have weight 0, so the file can hold a few more weight-0 lines than the last row says.

Query mix per scale:

| Kind | `full` | `sample` | `tiny` |
|---|---|---|---|
| normal | 1,950 | 185 | 26 |
| `s == t` (main body, isolated) | 4 + 2 | 1 + 1 | 1 + 1 |
| zero path | 2 | 1 | 1 |
| parallel min | 4 | 2 | 2 |
| max weight, over uint32 | 2 + 2 | 1 + 1 | 1 + 1 |
| from no-in node, to no-out node | 2 + 2 | 1 + 1 | 1 + 1 |
| unreachable (island, no-out source, isolated source, no-in target) | 4 + 2 + 2 + 2 | 1 + 1 + 1 + 1 | 1 + 1 + 1 + 1 |
| duplicate | 20 | 2 | 1 |
| **answers `-1`** | **10** | **4** | **4** |

### 6. Banned libraries

Per site rule 02, a library that solves the whole task is banned. For this track that is any library or module that computes shortest paths, distances or route-planning indexes for you. By name, and not limited to:

- **Python:** NetworkX, python-igraph, graph-tool, rustworkx (retworkx), NetworKit, scipy.sparse.csgraph, cuGraph, Pathfinding (`pathfinding`), OSMnx.
- **C and C++:** Boost Graph Library (BGL), LEMON, igraph, SNAP, NetworKit, RoutingKit, OSRM, Valhalla, the graph module of Google OR-Tools.
- **Rust:** the algorithm modules of petgraph (`petgraph::algo`, including `dijkstra`, `astar`, `bellman_ford`, `floyd_warshall`, `k_shortest_path`), fast_paths, the `pathfinding` crate, rustworkx-core.
- **JVM:** GraphHopper, JGraphT, JUNG, Neo4j Graph Data Science.
- **Go, Julia and others:** gonum `graph/path`, Graphs.jl (and LightGraphs.jl), pgRouting, any database's built-in shortest-path function.

Also banned: any library that builds contraction hierarchies, hub labels, transit-node routing or another shortest-path index for you.

Allowed, and counted on the dependency axis like any other dependency: plain data structures (heaps and priority queues, including radix heaps, hash maps, bitsets, arenas, allocators), parsing helpers, and parallelism runtimes. Graph *containers* are allowed as long as no shortest-path or indexing algorithm from them is called; petgraph's `Graph` type is fine, `petgraph::algo` is not. If you are unsure whether a library is allowed, ask before kickoff.

### 7. Reference behaviour

`reference/d_traverse.py` is single-threaded Python 3.13, standard library only, and slow on purpose.

- It reads `graph.txt` line by line, splits each line and converts the three fields with `int()`.
- It stores the graph as a Python list of per-node lists of `(head, weight)` tuples, appended in file order. Parallel edges, duplicate lines and self-loops are kept as they are.
- For every query it starts a fresh Dijkstra search from `s` with a `heapq` binary heap and a `dict` of tentative distances, skips stale heap entries, and stops when `t` is popped. It builds no index and reuses nothing between queries.
- It searches forward from `s` only, so it settles every node closer to `s` than `t` is, and on an unreachable target it settles everything `s` can reach. Each of the "no-in target" queries settles the whole main body.
- It writes all answers at the end.

Measured at `full`, seed 1, on an Apple M4 Pro with `/usr/bin/time -l`: 46.8 s and 48.2 s wall in two runs (with other jobs on the machine; an earlier run on a quiet machine, same distribution, took 43.0 s), peak RSS 915 MB, identical output both times. About 2 s of that is loading the graph; the rest is queries, of which the two no-in-target queries take about 3 s each. At `sample` it takes 3.3 s and 103 MB. The scoring-box number comes from the calibration run on the box.

### 8. Changes from the site

The site text is in `late-oct-event/src/data/tracks.json`. Differences, with reasons:

1. **Dataset size is 92 MB.** `graph.txt` is 91,950,247 bytes at `full` (about 18.4 bytes per edge line). `n`, `m`, the header and the line format are exactly as published.
2. **Reference time and timeout** are the final scoring-box figures: the box-calibrated `reference_s` and `timeout_s` at ten times it. (On an Apple M4 Pro the reference runs in about 43 to 48 s; the scoring box differs.)
3. **The preview lines** on the site are the first edge lines of the seed-1 `full` file: `1188446 437285 120`, `64107 865467 170`, `472413 359585 122` and `429961 196139 82`.
4. **Weight 0 is legal.** The site says weights are uint32; this spec makes explicit that the range is `[0, 4294967295]`, that 0 occurs, and that sums are not limited to 32 bits.
5. **Things the site does not mention, all defined here:** parallel edges, exact duplicate lines, self-loops, nodes with no edges (they still count in `n`), and unreachable pairs.
6. **Queries and output are defined here.** The site gives only the query count. This spec adds `queries.txt` (2,000 lines `s t`, no header), the query distribution in section 2, and `distances.txt` with `-1` for unreachable.
7. **Scales.** `sample` has 10% of the nodes, edges and queries (200), from the same distribution, so it is a faithful small model of `full` for local checks; `tiny` has 1,000 edges and 40 queries.

---

# Image allowlist (https://speeddemon.dev/allowlist.json)

Every Dockerfile FROM must be one of these, pinned by digest.

- `debian:13-slim@sha256:62bd423aeee912639ca8ee681c2b762cb3e25137e0a92b20be455898a9f034ad`: runtime stage
- `python:3.13-slim@sha256:d98cc29d3266e535dd9da1099688d9cb3ca4753813571e071763d17725460962`: Python (the reference)
- `rust:1-slim-trixie@sha256:1a939748a7cbab172de91a014e3804a97af1394a8c844faa67fce9401c4e40fb`: Rust
- `rust:1-trixie@sha256:931c2c0531a8b143e044f0f3914dfe802f15bb3d1955f0b657e9359327ac2ffc`: Rust, full toolchain
- `halloumee/speed-demon-rust:1.99.1-trixie-llvm-tools@sha256:13e4ee7135d8eac6c56476c7d1aee201f36479f4745fd5d0e6b4529f4e13251a`: Rust, rust:1-trixie plus llvm-tools (llvm-profdata for PGO); source in harness/images/rust-llvm-tools
- `golang:1-trixie@sha256:fbb7ae40b62ff8afe31662df570d39b4f5052ac32b90dae8e60d7657a20a74f4`: Go
- `gcc:15@sha256:eb5e64286ad47187b32bda75c6b6916166ecb41da56559b8b09e0001748d2cce`: C and C++
- `gcc:16@sha256:43978d7710a803d1c3d32f4117c1528ec7285a4ce2c0a51e982ec1a6e3d87d95`: C and C++
- `eclipse-temurin:25-jdk@sha256:2d5d5e9b533342a565907c84f47a2ec298c40cde7b13a3ca85d4950fa456a641`: Java, Kotlin, Scala build
- `eclipse-temurin:25-jre@sha256:dbb43d81099eec91e0800226ed2296d03146ee7857165b95112372ad0d67e87f`: JVM runtime stage
- `ghcr.io/graalvm/native-image-community:25@sha256:0d936f32bb8acb5bc60c41b33e05f064d7a6aaf36b726538296c54949bd4a3c0`: GraalVM native-image
- `node:22-slim@sha256:de170b39439eb0e5a3444ad7d60b015614a381e10fecae2e30ec9e735122f38d`: JavaScript and TypeScript
- `node:24-slim@sha256:55fb5f5a527ea2c5c4a95f963e48ef90aa38c7b346054fdc8ee9957a2b55d205`: JavaScript and TypeScript
- `mcr.microsoft.com/dotnet/sdk:10.0@sha256:747cbade5bfd918a175ac32f8f1d931edec2c7d0d5397a3de72eaef1ac1f66d2`: C# and F# build
- `mcr.microsoft.com/dotnet/runtime:10.0@sha256:0cf09aaccf6a9f68900aed4c73fb28fe25bb59ec85de4dea08ba02240a8635f3`: .NET runtime stage
- `swift:6.2@sha256:d6f8f6112e9a63da0a4a151d6b93acd16699d35c0b82531ac1a245f5af210197`: Swift
- `haskell:9@sha256:15aeb965021bf51853f95a2328291d169aa541c836ca76986b97297df817f293`: Haskell