# Task: Define ScriptCommon Package and File Structure

## Goal
Define the target `ScriptCommon` package file layout and module boundaries before moving any code.

## Background / Rationale
A stable package layout avoids ad-hoc extraction and keeps future CLI/TUI frontends aligned.

## Scope
- New package directory layout (flat files, no subdirectories).
- Main header and sub-header organization.
- Separation into runtime, project/session, plugin-core, and diagnostics domains.

## Non-goals
- Implementing the files yet.
- Finalizing plugin ABI.

## Dependencies
- Results from phase1 inventory and boundary audit.
- U++ header/include policy from repository AGENTS instructions.

## Concrete Investigation Steps
1. Define high-level modules under `ScriptCommon`.
2. Map current ScriptIDE non-GUI classes into those modules.
3. Define placeholder files for future growth (CLI/TUI reuse).
4. Define migration order file-by-file.

## Affected Subsystems
- New `uppsrc/ScriptCommon/*`
- Existing `uppsrc/ScriptIDE/*`

## Implementation Direction
Planned initial structure (flat package):

```text
uppsrc/ScriptCommon/
  AGENTS.md
  ScriptCommon.upp
  ScriptCommon.h
  RunManager.h
  RunManager.cpp
  Linter.h
  Linter.cpp
  PathManager.h
  PathManager.cpp
  IDESettings.h
  PluginCoreInterfaces.h
  PluginRegistry.h
  PluginRegistry.cpp
  DocumentModel.h
  DocumentModel.cpp
  WorkspaceModel.h
  WorkspaceModel.cpp
```

Logical grouping is maintained in `ScriptCommon.upp` via `file` separators/comments, not subdirectories:
- Runtime: `RunManager*`, `Linter*`
- Environment: `PathManager*`
- Config: `IDESettings.h`
- Plugin: `PluginCoreInterfaces*`, `PluginRegistry*`
- Session: `DocumentModel*`, `WorkspaceModel*`

## Risks
- Premature over-modularization.
- Settings schema still implicitly tied to GUI page naming.

## Acceptance Criteria
- [ ] `ScriptCommon` flat package file layout is documented.
- [ ] Each logical module has clear responsibility and initial file list.
- [ ] Mapping from current ScriptIDE files to target files is explicit.
