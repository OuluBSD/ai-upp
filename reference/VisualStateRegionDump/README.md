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

See `Manager/2-plan/ai-upp/root/VisualStateModel/docs/ARCHITECTURE.md` for the full "Region identity" concept.
The key principle is:

> "Stable identity must not be equivalent to screen position."

This tool verifies that principle empirically: a region's identity persists across movement.

Run:

```sh
bin/build.exe -m 7 -j12 VisualStateRegionDump
bin\VisualStateRegionDump.exe
```

## M01/M02 Session Mode (Milestone 03)

Passing a session directory runs the same `VsmDetectChanges`/`VsmRegionMemory`
pipeline over a real M01/M02 TexasHoldem session (`metadata.json` +
`groundtruth.jsonl` + `frames/%08d.png`, see
`Manager/2-plan/ai-upp/root/VisualStateModel/docs/TEXAS_HOLDEM_SOURCE_CONTRACT.md`), decoding frames via
`VsmLoadM01M02SessionFrame` (the PNG bridge added in task 0103):

```sh
bin\VisualStateRegionDump.exe var\vsm_fixtures\texas_ps6p_sample
bin\VisualStateRegionDump.exe var\vsm_fixtures\texas_ps6p_sample --frame-start 1 --frame-end 1
bin\VisualStateRegionDump.exe var\vsm_fixtures\texas_ps6p_sample --jsonl-out out.jsonl
bin\VisualStateRegionDump.exe var\vsm_fixtures\texas_ps6p_sample --overlay-out out\overlay --crop-report-out out\crop
```

- `--frame-start N` / `--frame-end M` restrict processing to transitions ending
  in `[N..M]` (MILESTONE_03's "focused reruns" on a frame range). Frame 0 has
  no predecessor, so the earliest possible transition target is frame 1.
- `--jsonl-out <path>` writes one compact JSON object per changed region per
  frame transition (`frame_prev`, `frame`, `x`, `y`, `w`, `h`, `score`,
  `region_id`) to `<path>`, one record per line — deterministic and
  regression-diffable for a fixed input session. Without `--jsonl-out`, the
  same JSONL lines print to stdout instead.
- `--crop-report-out <dir>` (task 0110) writes, for each frame transition
  with >=1 changed region: one small cropped PNG per changed region
  (`crop_<prev4>_<curr4>_<idx2>.png`, e.g. `crop_0000_0001_00.png` — just the
  region's rect plus a fixed 12px padding margin, via `Draw/ImageOp.h`'s
  `Crop()` — NOT the whole frame; distinct from `--overlay-out`'s full-frame
  images) plus one markdown file (`report_<prev4>_<curr4>.md`) embedding the
  crop(s) and a data table with the same fields as the `--jsonl-out` record.
  Independent of and composable with `--jsonl-out`/`--overlay-out` — request
  any combination. Example `report_0000_0001.md` shape:

  ```markdown
  # Frame transition 0000 -> 0001

  7 changed region(s) detected.

  ## Region rgn-0001

  ![rgn-0001](crop_0000_0001_00.png)

  ...

  ## Region Data

  | region_id | x | y | w | h | score | frame_prev | frame |
  | --- | --- | --- | --- | --- | --- | --- | --- |
  | rgn-0001 | 416 | 120 | 32 | 16 | 0.236328125 | 0 | 1 |
  ...
  ```

This mode **supersedes** the OLD real-session path that used to read
`VsmSessionStoreSource`/`.vsm` binary sessions (task 0104) — that format is
incompatible with M01/M02's PNG-based contract and is no longer reachable from
this tool. The synthetic path above is unchanged and continues to serve as the
reuse smoke test for `ChangeDetect`/`RegionMemory` themselves.
