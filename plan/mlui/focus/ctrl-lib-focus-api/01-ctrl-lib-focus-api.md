# MLUI Focus Plan - CtrlLib Focus API

## Objective
Introduce production Focus API in `uppsrc/CtrlLib/MLUI` so apps can expose compact, task-oriented focus pages on top of raw GUI snapshot data.

## Current Status
- Implemented in references: `reference/MLUI_FocusPage` and `reference/MLUI_Focus`.
- Not yet implemented in production `CtrlLib` API surface.
- Macro+ref usage model validated in references (`MLUI_USE_VAR/CTRL/STATE/ACTION`).

## Next Tasks
1. Add production Focus registry API in `CtrlLib` (`RegisterFocusPage`, `GetFocusPage`, runtime clear/add helpers).
2. Add stable data model for page metadata + runtime fields/actions with deterministic output order.
3. Add macro helpers for low-friction usage in `Ctrl::Access(Visitor&)`.
4. Wire Focus emission into MLUI snapshot path without breaking existing snapshot clients.

## Risks
- Overly generic API may produce noisy output if no conventions exist.
- Non-deterministic ordering would hurt MCP automation reproducibility.
- Runtime field growth can inflate payloads unless pages stay intentionally compact.
