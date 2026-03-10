# Task: Define Pane Logic Services for GUI, CLI, and MCP Reuse

## Goal
Extract pane/business logic from ScriptIDE controls into `ScriptCommon` services that can be used by ScriptIDE UI adapters, ScriptCLI commands, and MCP handlers.

## Background / Rationale
Panes currently mix state logic and rendering widgets. MCP and CLI need the same logic without UI controls.

## Scope
- Identify pane logic currently embedded in UI classes.
- Define headless service interfaces and data models.
- Define GUI adapter boundaries for ScriptIDE panes.

## Non-goals
- Rebuilding pane visuals.
- Defining final TUI widgets.

## Dependencies
- `uppsrc/ScriptIDE/*Pane*` and related controls.
- ScriptCommon boundary rules.

## Concrete Investigation Steps
1. Audit panes and mark logic that is UI-independent.
2. Define service interfaces for each domain:
   - files tree/state
   - find in files
   - outline/symbol index view model
   - debugger session projection
   - variable inspection
   - plot metadata/history
   - help/history query
3. Define DTO-style response models for CLI/MCP transport.
4. Define ScriptIDE adapter classes that bind services to controls.

## Affected Subsystems
- `uppsrc/ScriptIDE/FilesPane.*`
- `uppsrc/ScriptIDE/FindInFilesPane.*`
- `uppsrc/ScriptIDE/OutlinePane.*`
- `uppsrc/ScriptIDE/DebuggerPane.*`
- `uppsrc/ScriptIDE/VariableExplorer.*`
- `uppsrc/ScriptIDE/PlotsPane.*`
- `uppsrc/ScriptIDE/HelpPane.*`
- `uppsrc/ScriptIDE/HistoryPane.*`
- New `uppsrc/ScriptCommon/*` services

## Implementation Direction
Planned service candidates in `ScriptCommon` (flat package files):
- `FilesService.h/.cpp`
- `SearchService.h/.cpp`
- `OutlineService.h/.cpp`
- `DebugProjectionService.h/.cpp`
- `VariableService.h/.cpp`
- `PlotService.h/.cpp`
- `HelpService.h/.cpp`
- `HistoryService.h/.cpp`

Each service returns plain Core/Value/JSON-friendly models and emits callbacks/events without GUI types.

## Risks
- Over-extraction causing needless complexity.
- Accidental leakage of GUI concerns into service API names.

## Acceptance Criteria
- [ ] Pane-to-service mapping is documented.
- [ ] Service interfaces avoid `CtrlCore`/`CtrlLib` types.
- [ ] ScriptIDE adapter boundary is explicit for each pane.
