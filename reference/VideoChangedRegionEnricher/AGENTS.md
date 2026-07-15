# VideoChangedRegionEnricher Agent Notes

Reads a changed-region manifest and each referenced `tracking_summary.json`,
then writes semantic overlap and diagnostic crop enrichment. OCR is deliberately
non-invasive: it reports unavailable/not-run unless an existing integration is
added later.

Build with `bin\\build.exe -m MSVS22x64 .\\reference\\VideoChangedRegionEnricher\\VideoChangedRegionEnricher.upp`.

