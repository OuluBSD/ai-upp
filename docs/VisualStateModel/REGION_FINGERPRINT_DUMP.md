# Region Fingerprint Dump Tool

## Overview

`reference/VisualStateRegionDump/` is a headless CLI tool that demonstrates and validates
the stable region identity property of the Phase 1 Region Memory layer (`RegionMemory.h`/`.cpp`).

## What It Does

The tool creates a synthetic multi-frame session in memory and verifies that regions retain
their assigned stable IDs across frames even when their screen position changes.

**Input:** Three synthetic frames (frame 0 = baseline, frame 1 = region at position A,
frame 2 = same region at position A + offset).

**Processing:**
1. Detect pixel changes between consecutive frames
2. Extract 32×32 fingerprints from each changed region
3. Use `VsmRegionMemory` to query for matching fingerprints
4. Assign stable region IDs: new regions get new IDs, matching regions get existing IDs
5. Print frame-by-frame region information

**Output:** Frame index, region ID, bounding rect, fingerprint hash (short form), and
a pass/fail assertion on whether the shifted region kept the same ID.

## Fingerprint Format

- **Fingerprint:** 32×32 grayscale (1024 bytes), extracted from a region via
  `VsmRegionMemory::ExtractFingerprint()`
- **Hash:** MD5 of the 1024-byte fingerprint data, printed as "md5:..." in the tool output
- **Matching:** `VsmRegionMemory::Distance()` computes normalized MAD (0..1) between
  fingerprints; a distance ≤ 0.3 is considered a match by default

## Stable Identity Definition

From `ARCHITECTURE.md`:

> "Region identity — a stable logical identifier assigned to a visual region across
> movement and resizing. Identity is tracked by fingerprint similarity, not by pixel
> coordinates alone."

This tool validates that property empirically:
- A region's visual content (fingerprint) should be the primary signal for identity
- Position changes alone should not create a new region ID
- If the fingerprint distance is low enough (≤ threshold), the region is "the same"

## Expected Result

When the tool runs:
1. Frame 1 detects a white rectangle; assigns ID `rgn-0001`; stores its fingerprint
2. Frame 2 detects the same rectangle at a shifted position
3. Frame 2's fingerprint is queried against Frame 1's stored fingerprint
4. Distance is well below threshold (≤ 0.3) → match found
5. Frame 2 gets the same ID: `rgn-0001`
6. Output: **"Region identity stability: OK"**

If matching fails (different IDs assigned), the tool prints **"FAIL"** with the actual
observed IDs, indicating a design property issue that requires investigation.

## Usage

```sh
bin/build.exe -m 7 -j12 VisualStateRegionDump
bin\VisualStateRegionDump.exe
```

## See Also

- `uppsrc/VisualStateModel/RegionMemory.h` — API reference
- `docs/VisualStateModel/ARCHITECTURE.md` — Full region memory design and context
