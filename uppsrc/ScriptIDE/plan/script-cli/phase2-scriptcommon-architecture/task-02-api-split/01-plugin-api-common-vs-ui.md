# Task: Split Plugin APIs into Common Core and UI Adapters

## Goal
Define a plugin API split where core plugin logic is headless (`ScriptCommon`) and GUI-specific integration remains in `ScriptIDE`.

## Background / Rationale
Current plugin interfaces are tied to `Ctrl`, `Bar`, and `PythonIDE`. This blocks headless execution and future TUI reuse.

## Scope
- Define core plugin interfaces for headless usage.
- Define GUI adapter interfaces for ScriptIDE docking/menu/document host integration.
- Define compatibility transition path, including MCP-friendly headless contracts.

## Non-goals
- Implementing binary plugin loading.
- Final TUI adapter design.

## Dependencies
- Current `PluginInterfaces.h` and `PluginManager.h/cpp`.
- `ScriptCommon` boundary rules.

## Concrete Investigation Steps
1. Split current extension points into headless vs GUI-bound sets.
2. Define core context abstraction independent of `PythonIDE` and `Ctrl`.
3. Define optional adapter extension interfaces for GUI/TUI frontends.
4. Define migration shim so existing plugins can transition incrementally.

## Affected Subsystems
- `uppsrc/ScriptIDE/PluginInterfaces.h`
- `uppsrc/ScriptIDE/PluginManager.*`
- New `uppsrc/ScriptCommon/*`

## Implementation Direction
Planned interface layers:
- Core (`ScriptCommon`):
  - `IScriptPlugin`
  - `IPluginRuntimeContext`
  - `IFileTypeHandlerCore`
  - `IPythonBindingProvider`
  - `ICustomExecuteProvider`
- Frontend adapters (`ScriptIDE`, later TUI):
  - `IGuiPaneProvider`
  - `IGuiDocumentHostFactory`
  - `IGuiCommandContribution`

Adapter model:
- Core plugin can run in CLI and serve MCP without any GUI hooks.
- GUI frontend probes optional GUI adapter interfaces when present.

## Risks
- Existing plugins may rely heavily on direct `PythonIDE` access.
- Dual interface phase can increase complexity temporarily.

## Acceptance Criteria
- [ ] Headless plugin interface set is documented.
- [ ] GUI-specific extension points are isolated.
- [ ] Backward-compatibility transition path is defined.
