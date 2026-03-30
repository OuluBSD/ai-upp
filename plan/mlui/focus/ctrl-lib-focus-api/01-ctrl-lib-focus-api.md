# MLUI Focus Plan - CtrlLib Focus API

## Objective
Introduce production Focus API in `uppsrc/CtrlLib/MLUI` so apps can expose compact, task-oriented focus pages on top of raw GUI snapshot data.

## Current Status
- Implemented in references: `reference/MLUI_FocusPage` and `reference/MLUI_Focus`.
- Production `CtrlLib` Focus API is now implemented in `uppsrc/CtrlLib/Mlui.{h,cpp}`.
- `MLUI::` wrappers and macro+ref model are available in production (`MLUI_USE_VAR/CTRL/STATE/ACTION`).
- Snapshot path now includes optional focus summary (`include_focus`).

## Progress Update (2026-03-30)
Completed:
1. Added production focus registry API:
   - `RegisterMluiFocusPage`, `GetMluiFocusPage`, `HasMluiFocusPage`
   - runtime helpers (`ClearRuntime`, `AddValue`, `AddCtrl`, `AddState`, `AddAction`, `ActionHandler`)
2. Added production convenience namespace API:
   - `MLUI::RegisterFocusPage`, `MLUI::GetFocusPage`, `MLUI::HasFocusPage`, etc.
3. Added macro helpers for low-friction runtime exposure:
   - `MLUI_USE_VAR`, `MLUI_USE_CTRL`, `MLUI_USE_STATE`, `MLUI_USE_ACTION`
4. Added deterministic ordering for focus payload output:
   - sorted keys for runtime/context maps
   - sorted focus list order by page id

Remaining:
1. Remove/align outdated reference package APIs (`reference/MLUI_FocusPage`) to avoid symbol collisions with production `MLUI::` API.

## Next Tasks
1. Add production Focus registry API in `CtrlLib` (`RegisterFocusPage`, `GetFocusPage`, runtime clear/add helpers).
2. Add stable data model for page metadata + runtime fields/actions with deterministic output order.
3. Add macro helpers for low-friction usage in `Ctrl::Access(Visitor&)`.
4. Wire Focus emission into MLUI snapshot path without breaking existing snapshot clients.

## Risks
- Overly generic API may produce noisy output if no conventions exist.
- Non-deterministic ordering would hurt MCP automation reproducibility.
- Runtime field growth can inflate payloads unless pages stay intentionally compact.
