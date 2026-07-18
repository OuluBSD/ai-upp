# VideoLiveRecognitionLoop Agent Notes

Scope: this file applies to `reference/VideoLiveRecognitionLoop`.

## Purpose

Task 0280 (MILESTONE_13): the continuous per-frame recognition + resolution
loop. For each new frame from `VsmVideoServerFrameSource` (Task 0279) it:
change-detects (`VsmDetectChanges`), classifies each changed region
(`VsmLiveRegionClassifier` -- the one genuinely-new piece, in
`uppsrc/VisualStateModel/LiveRegionClassifier.{h,cpp}`), OCRs the
OCR-relevant regions (`VsmTesseractOcrEngine`, Task 0277), resolves
structural street events + OCR value-changes, and forces the resolved board
onto a real headless `game/TexasHoldem` `Game` (Task 0270 API).

It reuses already-proven pieces; it does NOT reinvent recognition logic. The
one new algorithm is the live classifier's position-anchored rgb8x8 nearest
match, adapted from `reference/VideoConfidenceTieredCandidates`' template
tier (Task 0267).

## Modes

```
bin\VideoLiveRecognitionLoop.exe --classify-selftest
bin\VideoLiveRecognitionLoop.exe --offline-frames tmp\real_recording_0263_frames [--max-frames N] [--verbose]
# live (needs a running VideoServer, Task 0278):
bin\VideoServer.exe --source video --video bin\video_record_25min_20260716_203356.mp4 --fps 8 &
bin\VideoLiveRecognitionLoop.exe --live --port 8082 --seconds 240 [--no-ocr | --ocr-cap N]
```

- `--classify-selftest`: rigorous leave-one-out classification accuracy over
  the 267-candidate labeled dataset (deterministic; the real accuracy number).
- `--offline-frames`: full stages-1..5 pipeline over the dataset's own source
  frames (deterministic per-stage timing, no server).
- `--live`: the real continuous loop against VideoServer.
- `--no-ocr` / `--ocr-cap N`: throttle OCR. OCR (tesseract, ~2s/crop) is far
  over the ~300ms real-time budget -- keep it off/capped to measure core
  keep-up. See the task file for the full timing finding.

## Rules

- Build with `bin\build.exe -m MSVS22x64 VideoLiveRecognitionLoop`. Never
  `script/build.py`.
- GUI_APP_MAIN + AttachConsole (mirrors `VideoGameEngineReplayValidator`) so
  it can drive the real `TexasHoldem` engine yet stream to a console.
- U++ `Format()` gotcha this tool hit repeatedly: do NOT put a letter
  immediately after a `%d`/`%f`/`%s` conversion (`%dx`, `%.1fms` are
  mis-parsed) and there is no `%lld` -- cast to int or split the string.
