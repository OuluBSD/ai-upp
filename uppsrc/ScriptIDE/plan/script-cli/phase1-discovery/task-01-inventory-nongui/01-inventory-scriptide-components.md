# Task: Inventory ScriptIDE Components for Non-GUI Extraction

## Goal
Produce a precise inventory of `ScriptIDE` classes/functions grouped into: GUI-only, non-GUI candidate, and mixed (needs split).

## Background / Rationale
Without an explicit inventory, extraction work will drift and GUI coupling can leak into `ScriptCommon`.

## Scope
- Audit current files in `uppsrc/ScriptIDE`.
- Classify responsibilities and dependency type for each class.
- Identify first extraction candidates.

## Non-goals
- Moving files yet.
- Refactoring runtime behavior.

## Dependencies
- `uppsrc/ScriptIDE/ScriptIDE.upp`
- Existing ScriptIDE plan conventions.

## Concrete Investigation Steps
1. Enumerate all ScriptIDE source/header files and classify by primary role.
2. Mark files with `CtrlCore/CtrlLib/Docking/CodeEditor/RichEdit` usage as GUI-coupled.
3. Mark runtime-only files (`RunManager`, `Linter`, `PathManager`, `IDESettings`) as initial extraction candidates.
4. Mark mixed files (e.g., plugin interfaces/manager) requiring API split.

## Affected Subsystems
- `uppsrc/ScriptIDE/*`
- Future `uppsrc/ScriptCommon/*`
- Future `uppsrc/ScriptCLI/*`

## Implementation Direction
Create an inventory table with columns:
- File/Class
- Current package
- GUI dependency level (`none`, `indirect`, `direct`)
- Destination (`ScriptCommon`, `ScriptIDE`, `split`)
- Notes

## Risks
- Misclassification of transitive GUI dependencies.
- Overlooking helper functions embedded in GUI files.

## Acceptance Criteria
- [ ] Every ScriptIDE file is classified.
- [ ] Initial extraction set is explicitly listed.
- [ ] Mixed components that require interface split are identified.
