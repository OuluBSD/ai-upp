# Task: Define Headless Plugin Testing Strategy Using ScriptCLI + ScriptCommon

## Goal
Define how plugins are tested in headless mode without GUI dependencies.

## Background / Rationale
The primary driver for this split is reliable headless plugin development and testing.

## Scope
- Plugin test harness strategy.
- Fixture/layout for test inputs.
- CI-friendly invocation patterns.

## Non-goals
- GUI integration tests.
- Performance benchmarking.

## Dependencies
- ScriptCLI command surface.
- ScriptCommon plugin registry/runtime API.

## Concrete Investigation Steps
1. Define plugin fixture directory layout.
2. Define plugin lifecycle test cases:
   - load/initialize/shutdown
   - file-type routing
   - custom execute dispatch
   - VM binding sync
3. Define golden output handling for deterministic assertions.
4. Define local and CI execution commands.

## Affected Subsystems
- Future `upptst/*` plugin tests
- `ScriptCLI plugin test`
- `ScriptCommon/*`

## Implementation Direction
Test topology:
- Unit-level tests for registry and routing.
- Integration-level CLI tests for plugin commands.
- Optional contract tests shared across GUI/CLI/TUI frontends.

## Risks
- Plugin tests may still accidentally depend on GUI-only codepaths.
- Non-deterministic VM state across test runs.

## Acceptance Criteria
- [ ] Headless plugin test types are documented.
- [ ] Fixture and invocation conventions are documented.
- [ ] CI-ready deterministic output strategy is defined.
