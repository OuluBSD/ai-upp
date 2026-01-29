Task 0004 - Descriptor matching acceleration [DONE]

Checklist
- [x] Integrate FAST/ORB keypoints (ComputerVision::OrbSystem).
- [x] Support Descriptor32 (256-bit Hamming) matching.
- [x] Optimize stereo matching with epipolar constraints.

Deliverables
- Matching strategy spec and performance targets.

Findings and decisions
- Current matching uses PopCount32 over 8x uint32 for 32-byte ORB descriptors.
- For small sets: use 4x uint64 popcount (Descriptor32::u64) or 8x uint32 popcount with early-exit on threshold.
  - This avoids full SIMD complexity while still reducing loop overhead.
  - On SSE4.2/AVX2 targets, consider intrinsic-based popcount for 128/256-bit blocks later.
- For medium/large sets: add descriptor indexing via LSH or multi-index hashing:
  - Split 256-bit descriptor into 4x 64-bit chunks; use 1-2 chunks as hash keys.
  - Maintain multiple hash tables (multi-probe) to reduce false negatives.
  - Candidate set size target: < 64 per query for interactive tracking.
- Octree is reserved for spatial queries only (point locations), not for descriptor similarity.

Performance targets (initial)
- Small-set matching: < 0.2 ms per 500x500 comparisons on desktop CPU.
- Large-set matching: < 2 ms per 1k query descriptors against 50k map descriptors (with hash filter).

Notes
- SIMD popcount is considered an optimization step after correctness; use plain popcount + early-exit first.
- Hash buckets should be keyed by descriptor chunk + coarse grid cell to retain spatial locality.
