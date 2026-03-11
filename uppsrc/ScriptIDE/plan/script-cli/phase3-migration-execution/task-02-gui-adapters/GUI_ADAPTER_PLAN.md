# ScriptIDE GUI Adapter Plan over ScriptCommon

## Goal
Define ScriptIDE adapter layer that consumes ScriptCommon services while keeping all Ctrl/Docking/UI behavior local to ScriptIDE.

## Adapter Set
- `ScriptIdeRuntimeAdapter`:
  - wraps `RunManager`, `Linter`, `PathManager` services from ScriptCommon
  - exposes UI-friendly events/messages for PythonIDE
- `ScriptIdePluginUiAdapter`:
  - bridges plugin core registry to dock panes/menu entries/document hosts
- `ScriptIdePaneAdapters`:
  - one adapter per pane view to bind ScriptCommon service models to controls

## Ownership Rules
- ScriptCommon: state, business logic, service APIs, plugin runtime contracts
- ScriptIDE: Ctrl objects, docking/layout, menus/toolbars, dialogs, user prompts

## Data Flow
1. UI triggers command in PythonIDE.
2. PythonIDE calls adapter.
3. Adapter calls ScriptCommon service.
4. Adapter transforms result into UI model/state update.
5. UI renders state and user feedback.

## Error/Log Strategy
- ScriptCommon returns status/result structures.
- Adapters map these to:
  - status bar updates
  - pane state refresh
  - optional prompt/log messages
- No Prompt/GUI calls inside ScriptCommon.

## Initial Files (planned)
- `uppsrc/ScriptIDE/ScriptIdeRuntimeAdapter.h/.cpp`
- `uppsrc/ScriptIDE/ScriptIdePluginUiAdapter.h/.cpp`
- `uppsrc/ScriptIDE/ScriptIdePaneAdapters.h/.cpp`

## Acceptance
- [x] Adapter responsibilities defined.
- [x] One-way dependency (ScriptIDE -> ScriptCommon) preserved.
- [x] Error/log boundary defined.
