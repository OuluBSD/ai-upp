# Task: Plan ScriptIDE GUI Adapters on Top of ScriptCommon

## Goal
Define how ScriptIDE consumes ScriptCommon through GUI adapters instead of shared GUI-bound core classes.

## Background / Rationale
After extraction, ScriptIDE still needs menus, docking panes, dialogs, and document controls while core logic stays headless.

## Scope
- Adapter classes in ScriptIDE that bridge GUI events to ScriptCommon services.
- Ownership/lifecycle boundaries between UI and core models.
- Error/log/reporting flow from core to GUI and MCP responses.

## Non-goals
- TUI implementation.
- Rebuilding entire UI architecture.

## Dependencies
- ScriptCommon core interfaces.
- Existing ScriptIDE main window/pane architecture.

## Concrete Investigation Steps
1. Define adapter classes and responsibilities.
2. Define data flow for run/lint/path/plugin/pane-state commands (CLI+MCP).
3. Define callback/event strategy (core emits events, UI renders state).
4. Define where GUI-only prompts/menus remain.

## Affected Subsystems
- `uppsrc/ScriptIDE/PythonIDE.*`
- `uppsrc/ScriptIDE/PluginManager.*`
- `uppsrc/ScriptIDE/*Pane*`

## Implementation Direction
Planned ScriptIDE adapter layer:
- `ScriptIdeRuntimeAdapter` (Run/Lint/Stop actions)
- `ScriptIdePluginUiAdapter` (dock panes/menu contributions)
- `ScriptIdeSettingsAdapter` (load/apply settings to UI controls)

Core rule:
- Adapters depend on ScriptCommon.
- ScriptCommon must not depend on adapters.

## Risks
- Event duplication between old ScriptIDE callbacks and new core events.
- Adapter layer becoming a new monolith if scope is not controlled.

## Acceptance Criteria
- [ ] Adapter classes and ownership are documented.
- [ ] UI/core event boundaries are explicit.
- [ ] No reverse dependency from ScriptCommon to ScriptIDE.
