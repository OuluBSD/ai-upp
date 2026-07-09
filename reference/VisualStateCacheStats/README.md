# VisualStateCacheStats

Console reference package demonstrating `VsmPipelineCache` statistics collection
and hit/miss ratio measurement.

Creates a synthetic session with 2 frames, a preprocessing pipeline, and 
template matching rules. Runs the observation pipeline twice over the same 
session:

1. **Run 1 (cache empty):** Pipeline processes frames, cache records misses.
2. **Run 2 (cache loaded):** Pipeline processes the same data again, demonstrating 
   cache hits on identical computations.

Prints cache statistics after each run and verifies that:
- Entry count remains stable
- Hit count increases in run 2
- Miss count tracks lookups in both runs

Use this to:
- Understand `VsmPipelineCache` API and statistics tracking
- Verify cache efficiency by comparing run 1 vs run 2 metrics
- Inspect hit/miss rates for production pipeline tuning

Run:

```sh
bin/build.exe -m 7 -j12 VisualStateCacheStats
bin\VisualStateCacheStats.exe
```

See `Manager/2-plan/ai-upp/root/VisualStateModel/docs/CACHE.md` for cache design and architecture.
