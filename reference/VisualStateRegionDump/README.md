# VisualStateRegionDump

Console reference package demonstrating `RegionMemory` stable region identity tracking
across frame-to-frame changes, specifically when regions move or are resized but retain
the same visual content (fingerprint).

## Purpose

Phase 1 of VisualStateModel implements region identity stability via fingerprint matching:
regions with similar visual content (even if moved) should be assigned the same stable ID
across frames. This tool makes that property inspectable without debugger stepping.

## Demo

Creates a synthetic 3-frame session:

1. **Frame 0** — baseline gray (reference)
2. **Frame 1** — gray with a white rectangle at position (100, 80), size 60×50
3. **Frame 2** — gray with the same white rectangle at position (105, 85), size 60×50

The tool:
- Detects pixel changes between consecutive frames
- Extracts 32×32 grayscale fingerprints from each changed region
- Uses `VsmRegionMemory` to query for matching fingerprints
- Assigns stable region IDs based on fingerprint similarity
- Prints frame-by-frame region info (frame, region ID, bounding rect, fingerprint hash)
- **Asserts and reports** whether the shifted rectangle kept the same stable ID

Expected result: The white rectangle in Frame 1 gets assigned `rgn-0001`. When the
same rectangle appears at a different position in Frame 2, its fingerprint should match
the stored fingerprint for `rgn-0001`, so it should get the same ID. If not, a FAIL is
reported along with the actual IDs observed.

## Output Format

```
Frame N regions:
  Frame N: region_id=rgn-XXXX rect=(x,y,w×h) hash=md5:... [matched, distance=0.XX]
```

## Design Context

See `docs/VisualStateModel/ARCHITECTURE.md` for the full "Region identity" concept.
The key principle is:

> "Stable identity must not be equivalent to screen position."

This tool verifies that principle empirically: a region's identity persists across movement.

Run:

```sh
bin/build.exe -m 7 -j12 VisualStateRegionDump
bin\VisualStateRegionDump.exe
```
