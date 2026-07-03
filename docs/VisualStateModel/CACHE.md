# VisualStateModel — Rule Execution Cache

## Overview

`VsmPipelineCache` (in `uppsrc/VisualStateModel/PipelineCache.h`) provides a
file-backed first-level cache for VisualStateModel pipeline computations.

Cache data is safe to delete. The pipeline recomputes correctly without it.

---

## Cache Key

`VsmCacheKey` uniquely identifies one computation:

| Field | Example | Description |
|---|---|---|
| `asset_id` | `"frames/00000000.vsm"` | Source image path |
| `pipeline_id` | `"pipe-001"` | Preprocessing pipeline |
| `rule_id` | `"rule-001"` | Which rule was evaluated |
| `rule_type` | `"fingerprint"`, `"template"`, `"ocr"`, `"preprocess"` | Category |
| `engine_version` | `"FakeOCR/1.0"` | OCR engine name and version |
| `schema_version` | `1` | Cache schema version |

The hash is: `MD5(field1 + "|" + field2 + … )` — hex string.

---

## Cache Entry

```cpp
struct VsmCacheEntry {
    String key_hash;    // MD5 hex
    String data_json;   // serialized result (JSON string)
    String created_at;  // ISO timestamp
};
```

Large binary results (e.g., preprocessed pixels) are not stored in-cache
directly. Only metadata and compact JSON results are cached.

---

## VsmPipelineCache API

```cpp
VsmPipelineCache cache;
cache.SetLog(&log);
cache.Open(cache_dir);  // creates vsm_cache.json if missing

// Query
String out;
bool hit = cache.Get(key, out);

// Store
cache.Put(key, result_json);

// Stats
int hits   = cache.GetHits();
int misses = cache.GetMisses();
cache.ResetStats();

// Save to disk
cache.Save();

// Clear
cache.Clear();  // removes vsm_cache.json
```

---

## Storage

The cache file is `<cache_dir>/vsm_cache.json`, a JSON array of `VsmCacheEntry`
objects. The workbench uses `<temp>/vsm_wb_cache/vsm_cache.json`.

Session-local cache paths follow:
```
<session_root>/cache/vsm_cache.json
```

---

## Workbench Integration

- `File → Clear Pipeline Cache` clears the current workbench cache.
- After each "Run Pipeline", the debug log shows:
  ```
  pipeline: done — obs=3 transitions=1 divergences=0  cache hits=0 misses=3
  ```
- Re-running the same pipeline (not yet caching in pipeline runner itself)
  allows inspection of miss/hit counts as cache use is added incrementally.

---

## Non-goals

- The cache is not a database.
- Cache is not required for pipeline correctness.
- Background compaction is not implemented.
- GUI layout or UI-only state is not cached here.

---

## Test Coverage

`TestPipelineCache()` in `reference/VisualStateModelTest/main.cpp`:

- Open on new directory
- Key hashing: two keys with different asset_id produce different hashes
- Miss on empty cache
- Put + Get hit
- Stats counters (hits, misses)
- Save and reload: hit still available after reload
- Clear: count returns to 0

All 14 test suites pass as of task 0018.

---

## Stats CLI

`reference/VisualStateCacheStats/` is a console reference package that
demonstrates cache statistics collection and hit/miss ratio inspection.

### Overview

The package runs an observation pipeline twice over an identical session to
show cache behavior:

**Run 1 (Cache Empty):**
- Pipeline processes all computations
- Each cache lookup results in a miss
- Results are stored in cache

**Run 2 (Cache Loaded):**
- Pipeline processes the same computations again
- Lookups should hit cached entries
- Performance characteristics can be inspected

### Counters

Three read-only statistics are available via `VsmPipelineCache`:

```cpp
int GetHits()   const;  // Number of cache lookups that found an entry
int GetMisses() const;  // Number of cache lookups that did not find an entry
int GetCount()  const;  // Number of entries currently in cache
```

### Usage

```sh
bin/build.exe -m 7 -j12 VisualStateCacheStats
bin\VisualStateCacheStats.exe
```

Output shows:
```
--- RUN 1 (Cache Empty) ---
  Frames processed: 2
  Observations made: N
  Cache hits: 0
  Cache misses: 8
  Cache entries: 4

--- RUN 2 (Cache Loaded) ---
  Frames processed: 2
  Observations made: N
  Cache hits: 4
  Cache misses: 12
  Cache entries: 4

--- Verification ---
  Cache entry count: 4 OK
  Cache hits increased: 0 -> 4 OK
  Cache misses recorded: 12 OK
```

### Interpreting Results

- **Hits increase in Run 2:** Demonstrates that identical computations
  are being reused from cache
- **Entry count stable:** Verifies no entries were evicted or duplicated
- **Misses may increase:** Normal — the demo makes new lookups on each run

In a production workbench:
- Multiple runs on the same session should show climbing hit ratio
- Cache efficiency = `hits / (hits + misses)`
- High miss counts on repeat runs may indicate cache key misalignment

See `CACHE.md` (above) for cache architecture and key derivation.
