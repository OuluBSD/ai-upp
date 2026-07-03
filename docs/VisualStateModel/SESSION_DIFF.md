# VisualStateModel — Session-to-Session Diff

## Overview

`VsmSessionDiff` compares the `divergences.json` files from two session
directories and produces a structured comparison result. This is useful for
detecting regressions: "did today's replay of this recorded scenario produce
the same divergences as last week's replay?"

Each session's divergence list is loaded, and entries are matched by frame
number and message text (exact string match, no fuzzy matching). The result
categorizes each divergence as "only in session A", "only in session B", or
"present in both".

---

## API

### Comparator

```cpp
VsmSessionDiff diff;
diff.SetLog(&log);
VsmSessionDiffResult result = diff.Compare(session_dir_a, session_dir_b);
```

### Result

```cpp
result.only_in_a  // Count of divergences in A but not B
result.only_in_b  // Count of divergences in B but not A
result.in_both    // Count of matching divergences (same frame + message)
result.entries    // Vector<VsmSessionDiffEntry> with detailed breakdown
```

### Per-Entry

Each `VsmSessionDiffEntry` has:

```cpp
entry.status      // "only_in_a", "only_in_b", or "in_both"
entry.frame       // Frame number where divergence occurred
entry.severity    // Severity level (e.g., "warning", "error")
entry.message     // Message text (used for matching)
```

---

## Matching Behavior

### Matching Key

Two divergences are considered identical if:

```cpp
div_a.frame == div_b.frame && div_a.message == div_b.message
```

Matching is **exact string match only**. No fuzzy or semantic matching is
performed. Other fields (`severity`, `region_id`, `expected_json`,
`observed_json`) are not considered for matching.

### Categorization

- **only_in_a**: Divergence in session A with no match in session B.
- **only_in_b**: Divergence in session B with no match in session A.
- **in_both**: Divergence with matching frame and message in both sessions.

---

## Missing Files

Sessions without `divergences.json` are treated as having zero divergences:

```cpp
entry.only_in_a = 0  // No entries from that session
entry.only_in_b = 0  // (or vice versa)
entry.in_both = 0
```

This is **not** an error. A session directory with no divergence data is a
valid scenario (the replay ran cleanly, or the divergence report was not
generated).

---

## Result Interpretation

### Simple Equality Check

```cpp
VsmSessionDiffResult result = diff.Compare(prev_session, curr_session);

if(result.only_in_a == 0 && result.only_in_b == 0) {
    // Sessions have identical divergences
}

if(result.only_in_b > 0) {
    // New divergences appeared in the new session (regression?)
}

if(result.only_in_a > 0) {
    // Divergences that existed before no longer appear (improvement)
}
```

### Detailed Investigation

```cpp
for(const VsmSessionDiffEntry& entry : result.entries) {
    if(entry.status == "only_in_a") {
        Cout() << "Previous session had: " << entry.message << "\n";
    } else if(entry.status == "only_in_b") {
        Cout() << "New regression: " << entry.message << "\n";
    } else {
        Cout() << "Stable divergence: " << entry.message << "\n";
    }
}
```

---

## Non-Goals

- Does **not** compare frame image content (use `VsmFrameComparison` for that).
- Does **not** track history across more than 2 sessions.
- Does **not** validate session structure (that is `VsmSessionValidator`'s
  role).
- Does **not** re-run divergence detection (reads pre-existing
  `divergences.json`).

---

## Reference Tool

`reference/VisualStateSessionDiff/` demonstrates:

1. Creates 2 synthetic session directories in temp.
2. Seeds session A with 2 divergences (one shared, one unique).
3. Seeds session B with 2 divergences (the same shared one, one unique to B).
4. Runs `VsmSessionDiff::Compare()` on both.
5. Prints the comparison result.
6. Asserts `only_in_a == 1`, `only_in_b == 1`, `in_both == 1`.

```sh
bin/build.exe -m 7 -j12 VisualStateSessionDiff
bin\VisualStateSessionDiff.exe
```

Expected output:

```
=== VisualStateModel Session Diff Demo ===

Creating session A: ...
Creating session B: ...

Seeding divergences.json files...
  Saved 2 divergences to session A
  Saved 2 divergences to session B

--- Running Session Diff ---

--- Diff Result Summary ---
Only in A:  1
Only in B:  1
In both:    1
Total entries: 3

Detailed Entries:
  [in_both] Frame 0 (warning): Shared divergence between sessions
  [only_in_a] Frame 0 (error): Unique divergence only in session A
  [only_in_b] Frame 1 (warning): Unique divergence only in session B

--- Acceptance Checks ---
OK: only_in_a == 1
OK: only_in_b == 1
OK: in_both == 1
OK: 3 total entries

All session diff checks passed.
```

---

## Design Notes

- Comparison is **read-only** — no files are modified.
- Logging is optional via `SetLog()`.
- Matching is deterministic and based solely on frame number and message text.
- The order of arguments matters: session A is compared against session B
  (not symmetric, though the counts are).
- All divergences from both sessions are included in the result's `entries`
  vector.
