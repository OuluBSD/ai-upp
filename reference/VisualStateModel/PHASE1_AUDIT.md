# VisualStateModel Phase 1 Audit

Audit date: 2026-06-29  
Audited by: Claude Code (automated + manual review)

## 1. Build Status

All Phase 1 targets build clean with **zero errors** using:

```
bin/build.exe -m 7 -j12 <target>
```

| Target | Packages | Binary size | Result |
|--------|----------|-------------|--------|
| VisualStateModelTest | 4 | 2 759 168 B | **OK** |
| VisualStateReplayReport | 4 | 2 783 232 B | **OK** |
| VisualStateWorkbench | 19 | 10 877 440 B | **OK** |

Linker warning `LNK4224: /INCREMENTAL:YES is no longer supported` appears in all MSVC builds — this is a toolchain noise warning, not a code issue.

## 2. Headless Constraint — uppsrc/VisualStateModel

```
rg "CtrlLib|Docking|TopWindow|TabCtrl|ArrayCtrl|DockWindow" uppsrc/VisualStateModel
```

**Result: no matches.** The package imports only `<Core/Core.h>` and is fully headless.

Files:
```
uppsrc/VisualStateModel/VisualStateModel.h  (umbrella + PCH)
uppsrc/VisualStateModel/VisualStateModel.upp
uppsrc/VisualStateModel/Types.h / Types.cpp
uppsrc/VisualStateModel/RegionMemory.h / RegionMemory.cpp
uppsrc/VisualStateModel/ChangeDetect.h / ChangeDetect.cpp
uppsrc/VisualStateModel/GroundTruth.h / GroundTruth.cpp
uppsrc/VisualStateModel/Replay.h / Replay.cpp
```

## 3. Sample JSON vs GROUND_TRUTH.md

`VsmMakeSampleJson()` (in GroundTruth.cpp) uses:
- schema version 1 ✓
- event types: `frame`, `change`, `region`, `state_snapshot`, `divergence` ✓
- `rect` as nested object for region events ✓
- `severity` field on divergence ✓
- `expected` / `observed` as JSON objects ✓

**Minor inconsistency:** sample uses `"fingerprint":"sha1:a3f8c1d2e"` (legacy placeholder string)
while `VsmRegionMemory` computes `md5:...` hashes at runtime. The sample is synthetic
and not meant to be bit-exact — this is acceptable. GROUND_TRUTH.md does not mandate the
hash algorithm prefix.

**Not a blocking issue.** No fix required.

## 4. Replay Diagnostics

`VisualStateModelTest` output confirms:

```
All VisualStateModel checks passed.
AppLog records: 9
```

`VisualStateReplayReport` output confirms all three log levels appear:
- INFO: frame events, region events, load complete
- WARN: divergence at frame 3 (surfaced twice — once from loader, once from replay)
- ERROR: none triggered (correct for clean sample)

Divergence is picked up by `VsmReplaySession::Step()` and forwarded through `CoreLog → AppLog`.

## 5. VisualStateReplayReport Output

Report written to `$TEMP/vsm_report/`:
```
index.md          — session header, divergences, stats, event index, regions table
events/000001.md  — change event page (frame 1, 1 region)
events/000002.md  — divergence page (frame 3, severity=warning)
```

**Bug found and fixed:** `Format("... %dx%d ...", w, h)` — U++ Format() was interpreting
`%dx` as a combined format specifier, producing `<N/A 'dx' for type 1>` instead of `80x40`.
Fixed by building the `WxH` string with `IntStr()` concatenation before passing to Format.

## 6. VisualStateWorkbench — Logging and Persistence

Confirmed usage pattern in `MainWindow::DockInit()`:

```cpp
log_.WhenRecord = [=](const AppLogRecord& r) { debug_tab_.AddRecord(r); };
```

`AppLog` instance owned by `MainWindow`, wired to `DebugLog` display tab.
`VsmGroundTruthLoader` and `VsmReplaySession` both call `SetLog(&log_)`.

`AppRegistry` persistence:
- `InitRegistry()` — creates registry with app/version key
- `LoadUserLayout()` / `SaveUserLayout()` — dock layout round-trip via AppRegistry blobs
- `LoadAppState()` / `SaveAppState()` — tab index and last session path via Jsonize

Uses `DockWindow` pattern correctly: `DockInit → InitDockers → OnResetDockLayout →
CacheDefaultLayout → InitRegistry → LoadAppState → LoadUserLayout`.

No new logging framework introduced. ✓

## 7. Change Detection and Region Memory Coverage

`VisualStateModelTest` exercises:

| Test | Result |
|------|--------|
| Types round-trip (Jsonize → JSON → back) | OK |
| RegionMemory: Add + FindNearest | matched `rgn-0001` distance=0.004 |
| RegionMemory: Distance self = 0 | OK |
| VsmDetectChanges on synthetic frames | 1 region at (48,56) 104×88 score=0.87 |
| VsmCompareFrames | OK |
| Identical frames → no changes | OK |
| Fingerprint hash format `md5:...` | OK |
| GroundTruthLoader: schema/frames/changes/regions/divergences | all correct |
| VsmReplaySession: 6 events, RunAll, CanStep=false after RunAll | OK |

Coverage is minimal but hits all public API paths.

## 8. Issues Found

### Fixed in this audit

| # | File | Issue | Fix |
|---|------|-------|-----|
| F1 | `reference/VisualStateReplayReport/ReportWriter.cpp` | `Format("...%dx%d...")` — U++ misinterprets `%dx` as format code | Build `WxH` with `IntStr(w) + "x" + IntStr(h)` before Format call |

### Known / Acceptable

| # | Item | Notes |
|---|------|-------|
| K1 | Sample JSON fingerprint uses `sha1:` prefix | Synthetic placeholder; implementation uses `md5:`. Not a protocol requirement. |
| K2 | `VsmFrameImage` uses raw `Buffer<byte>` RGBA | Cannot use U++ `Image` (Draw dep). Intentional design choice. Phase 2 OCR/template layers will need image conversion adapters at the GUI boundary. |
| K3 | `VsmReplaySession : NoCopy` | Session reset works via `.Clear()` on each Vector. Adequate for current scope. |

## 9. Phase 2 Readiness

**No blocking issues.** Foundation is stable.

Recommended task order for Phase 2:

1. **0008 Session storage and frame assets** — on-disk session management, frame file tracking. Extends headless package cleanly.
2. **0009 Region annotation editor** — GUI layer only; depends on session storage API from 0008.
3. **0010 Preprocessing pipeline model** — headless; defines per-region image processing chain.
4. **0011 Template match rule layer** — headless rule engine; depends on 0010.
5. **0012 OCR observation layer** — headless; depends on 0010. GPU/OCR library integration point.
6. **0013 Application model runtime** — ties observations to state; depends on 0011 and 0012.

The headless→GUI boundary is well-established: headless packages own all meaningful data
and logic; GUI packages (`VisualStateWorkbench`) only observe and edit. This pattern should
be maintained in Phase 2.

## Appendix: Exact Build Commands Used

```sh
bin/build.exe -m 7 -j12 VisualStateModelTest
bin/build.exe -m 7 -j12 VisualStateReplayReport
bin/build.exe -m 7 -j12 VisualStateWorkbench

bin/VisualStateModelTest.exe
bin/VisualStateReplayReport.exe

git diff --check -- uppsrc/VisualStateModel reference/VisualStateModel \
    reference/VisualStateModelTest reference/VisualStateWorkbench \
    reference/VisualStateReplayReport
```

All commands returned exit code 0.
