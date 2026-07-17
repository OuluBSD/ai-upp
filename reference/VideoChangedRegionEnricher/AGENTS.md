# VideoChangedRegionEnricher Agent Notes

Reads a changed-region manifest and each referenced `tracking_summary.json`,
then writes semantic overlap and diagnostic crop enrichment.

OCR integration (Task 0277): `OcrEvidence()` reports real
`VideoSemanticOcrProbe` results, not just unavailable/not-run placeholders.
By default it still guesses `<occurrence's tracker_dir>/ocr_probe.json` (the
original per-occurrence lookup, kept for back-compat). Pass
`--ocr-probe-json <path>` to read one exact `VideoSemanticOcrProbe --out`
JSON file instead, applied to every occurrence in the manifest regardless of
that occurrence's own `tracker_dir` -- real probe runs almost never land at
the guessed default path (this session alone used a dozen different `--out`
paths across tasks), so the flag is the normal way to get real OCR evidence
into an enrichment run without manually copying a probe JSON into place
first (Task 0271's old workaround).

Build with `bin\\build.exe -m MSVS22x64 .\\reference\\VideoChangedRegionEnricher\\VideoChangedRegionEnricher.upp`.

