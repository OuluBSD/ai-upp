# MLUI Focus Plan - Smoke Tests

## Objective
Add focused smoke coverage for MLUI + Focus stack so regressions in threading, protocol behavior, and action routing are caught early.

## Current Status
- Dedicated smoke script now exists: `script/test_mlui_focus_smoke.sh`.
- Baseline focus/baseline MLUI endpoint sanity is automated for Bombs.

## Progress Update (2026-03-30)
Completed:
1. Added smoke script:
   - `script/test_mlui_focus_smoke.sh`
2. Smoke covers baseline methods:
   - `ping`, `snapshot`, `click`, `set`, `key`, `mouse`
3. Smoke covers focus methods:
   - `focus.list`, `focus.get`, `focus.tree`, `focus.search`, `focus.action`
4. Added checks that reject unexpected `Unknown method` regressions.

Remaining:
1. Add Overviewer-specific smoke step for positive focus action contract (`file_tree.select`, `active_file.set_priority`).
2. Add timeout/non-blocking regression check for large trees (select should not hang).
3. Hook smoke script into CI/test launcher when available.

## Next Tasks
1. Add smoke script coverage for baseline MLUI methods:
   - `ping`, `snapshot`, `click`, `set`, `key`
2. Add smoke script coverage for focus endpoints:
   - `focus.list`, `focus.get`, `focus.tree`, `focus.search`, `focus.action`
3. Add thread-safety checks around GUI dispatch (`MluiGuiCall`) and non-blocking behavior.
4. Add one demo target (Bombs or Overviewer) as stable CI smoke fixture.

## Risks
- UI timing instability can cause flaky click/action tests.
- Tests tied to volatile widget paths may break on harmless UI refactors.
