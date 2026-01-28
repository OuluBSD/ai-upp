Task 0001 - Feature pipeline rewrite plan

Checklist
- Evaluate OrbSystem + DescriptorImage data flow.
- Define where to store per-feature metadata (score, scale, orientation, stereo pair id).
- Decide matching strategy: brute-force Hamming, grid-indexed Hamming, or FLANN-like.
- Define a consistent descriptor format between ComputerVision and Geometry.

Deliverables
- A concrete feature pipeline spec and module boundaries.
