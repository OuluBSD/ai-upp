# VisualStateModel — Batch Divergence Report

## Overview

`VsmBatchDivergenceReport` scans a list of session directories and aggregates
divergence statistics across all of them. Each session's `divergences.json`
(written by `VsmModelRuntime::SaveDivergenceReport()`) is parsed and summarized,
producing a combined report with per-session and aggregate counts.

---

## API

### Reporter

```cpp
VsmBatchDivergenceReport batch_report;
batch_report.SetLog(&log);
VsmBatchReportResult result = batch_report.Run(session_dirs);
```

### Result

```cpp
result.sessions_scanned    // Total sessions provided to Run()
result.sessions_with_data  // Sessions that had divergences.json
result.total_divergences   // Sum of divergence_count across all sessions
result.total_errors        // Sum of error_count across all sessions
result.total_warnings      // Sum of warning_count across all sessions
result.sessions            // Vector<VsmBatchSessionEntry> with per-session data
```

### Per-Session Entry

Each `VsmBatchSessionEntry` has:

```cpp
entry.session_dir          // Directory path provided to Run()
entry.session_id           // Extracted from session's manifest.json (if readable)
entry.divergence_count     // Total divergences in this session
entry.error_count          // Divergences with severity="error"
entry.warning_count        // Divergences with severity="warning"
entry.had_divergence_file  // true if divergences.json existed
```

---

## Aggregation Behavior

### Missing divergences.json

Sessions without `divergences.json` are not errors. The reporter records:

```cpp
entry.had_divergence_file = false
entry.divergence_count = 0
entry.error_count = 0
entry.warning_count = 0
```

These sessions are counted in `sessions_scanned` but **not** in
`sessions_with_data`.

### Severity Counting

Divergences are counted by `severity` field:

- `"error"` → increments `error_count`
- `"warning"` → increments `warning_count`
- Other values → increments `divergence_count` but not error/warning counts

All divergences increment `divergence_count`.

### Session ID Extraction

If `manifest.json` can be quickly parsed and contains a `session_id` field, it
is populated. If the manifest is missing or unparseable, `session_id` is left
empty (not an error).

---

## Result Interpretation

```cpp
VsmBatchReportResult result = batch_report.Run(dirs);

if(result.sessions_scanned == 0) {
    // No sessions to scan
}

if(result.sessions_with_data > 0) {
    // At least one session had divergences
}

for(const VsmBatchSessionEntry& entry : result.sessions) {
    if(entry.had_divergence_file) {
        // This session had a divergence report
        if(entry.error_count > 0) {
            // Errors detected
        }
    }
}

// Aggregate summary
Cout() << "Total divergences: " << result.total_divergences << "\n";
Cout() << "Total errors: " << result.total_errors << "\n";
Cout() << "Total warnings: " << result.total_warnings << "\n";
```

---

## Non-Goals

- Does **not** recursively scan directory trees (caller provides explicit list).
- Does **not** track history across runs (single-shot aggregation).
- Does **not** validate session structure (that is `VsmSessionValidator`'s role).
- Does **not** re-run divergence detection (reads pre-existing `divergences.json`).

---

## Reference Tool

`reference/VisualStateBatchReport/` demonstrates:

1. Creates 3 synthetic session directories in temp.
2. Seeds 2 of them with `divergences.json` (one with 2 divergences, another with 3).
3. Runs `VsmBatchDivergenceReport::Run()` over all 3.
4. Prints the aggregated summary.
5. Asserts `sessions_scanned == 3`, `sessions_with_data == 2`, and
   `total_divergences == 5`.

```sh
bin/build.exe -m 7 -j12 VisualStateBatchReport
bin\VisualStateBatchReport.exe
```

Expected output:

```
=== VisualStateModel Batch Divergence Report Demo ===

Creating session 1: ...
Creating session 2: ...
Creating session 3: ...

Seeding divergences.json files...
  Saved 2 divergences to session 1
  Saved 3 divergences to session 2
  Session 3 has no divergences.json (expected scenario)

--- Running Batch Divergence Report ---

--- Batch Report Summary ---
Sessions scanned:     3
Sessions with data:   2
Total divergences:    5
Total errors:         2
Total warnings:       3

Per-Session Details:
  Session: batch-session-001
    ...
  Session: batch-session-002
    ...
  Session: batch-session-003
    ...

--- Acceptance Checks ---
OK: 3 sessions scanned
OK: 2 sessions with divergence data
OK: 5 total divergences
OK: 2 total errors
OK: 3 total warnings

All batch report checks passed.
```

---

## Design Notes

- Batch scanning is **read-only** — no files are modified.
- Logging is optional via `SetLog()`.
- Severity strings are exactly as stored in `VsmDivergence::severity` (check
  `Types.h` for valid values).
- Caller explicitly provides session directories; no filesystem recursion.
