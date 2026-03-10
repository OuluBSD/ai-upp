# Task: Define ScriptCLI Package Layout and Build Surface

## Goal
Define the initial `ScriptCLI` package structure and dependencies for headless execution/testing.

## Background / Rationale
A dedicated package is needed for non-GUI plugin workflows and automation without ScriptIDE interference.

## Scope
- New package directory and files (flat package, no subdirectories).
- `ScriptCLI.upp` dependency plan.
- Entry-point and command dispatch architecture.

## Non-goals
- Implementing all CLI commands.
- Packaging/distribution strategy.

## Dependencies
- `ScriptCommon` package finalized enough for consumption.

## Concrete Investigation Steps
1. Define minimal file set for bootstrap CLI.
2. Define command dispatch pattern.
3. Define stderr/stdout contract for automation.
4. Define exit-code semantics.

## Affected Subsystems
- New `uppsrc/ScriptCLI/*`
- New `uppsrc/ScriptCLI/ScriptCLI.upp`

## Implementation Direction
Planned structure (flat package):

```text
uppsrc/ScriptCLI/
  AGENTS.md
  ScriptCLI.upp
  ScriptCLI.h
  Main.cpp
  CommandRegistry.h
  CommandRegistry.cpp
  RunCommand.h
  RunCommand.cpp
  LintCommand.h
  LintCommand.cpp
  PluginCommand.h
  PluginCommand.cpp
  CliSession.h
  CliSession.cpp
  Reporter.h
  Reporter.cpp
```

Logical grouping is maintained in `ScriptCLI.upp` via `file` separators/comments:
- Command layer: `CommandRegistry*`, `RunCommand*`, `LintCommand*`, `PluginCommand*`
- Runtime/session: `CliSession*`
- Output/reporting: `Reporter*`

Dependency rule:
- `ScriptCLI` uses `Core`, `ByteVM`, `ScriptCommon` only.

## Risks
- Command design too tightly coupled to initial use cases.
- Missing structured output makes CI integration brittle.

## Acceptance Criteria
- [ ] ScriptCLI flat package file layout and responsibilities are documented.
- [ ] `.upp` dependency policy excludes GUI packages.
- [ ] Command dispatch and exit-code strategy is defined.
