# VisualStateBatchReport

Console reference package demonstrating `VsmBatchDivergenceReport`.

Creates 3 synthetic session directories in temp, seeds 2 of them with
`divergences.json` files (one with 2 divergences, another with 3), runs the
batch aggregator over all 3, and verifies the combined summary counts match
what was seeded.

Run:

```sh
bin/build.exe -m 7 -j12 VisualStateBatchReport
bin\VisualStateBatchReport.exe
```

See `Manager/2-plan/ai-upp/root/VisualStateModel/docs/BATCH_REPORT.md` for design notes and API reference.
