# MLUI Focus Plan - Smoke Tests

## Objective
Add focused smoke coverage for MLUI + Focus stack so regressions in threading, protocol behavior, and action routing are caught early.

## Current Status
- Manual validation is possible but repetitive.
- No dedicated smoke set for focus endpoints and GUI-thread bridge behavior.

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
