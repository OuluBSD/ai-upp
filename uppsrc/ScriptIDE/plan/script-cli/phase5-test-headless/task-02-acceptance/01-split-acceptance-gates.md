# Task: Define Acceptance Gates for ScriptCommon + ScriptCLI Split

## Goal
Define objective completion gates for accepting the split from ScriptIDE to ScriptCommon/ScriptCLI.

## Background / Rationale
The migration spans package boundaries and multiple frontends; completion criteria must be explicit.

## Scope
- Package-level dependency gates.
- Functional parity gates for core workflows.
- Regression checklist for ScriptIDE continuity.

## Non-goals
- Release planning.
- Feature expansion beyond split objectives.

## Dependencies
- Prior phases (architecture, migration, CLI bootstrap, headless testing).

## Concrete Investigation Steps
1. Define dependency gates (no `CtrlCore`/`CtrlLib` in `ScriptCommon` and `ScriptCLI`).
2. Define core workflow gates (`run`, `lint`, plugin lifecycle).
3. Define ScriptIDE regression gates after extraction.
4. Define future TUI-readiness checks (API shape, no GUI assumptions in core).

## Affected Subsystems
- `uppsrc/ScriptCommon/*`
- `uppsrc/ScriptCLI/*`
- `uppsrc/ScriptIDE/*`

## Implementation Direction
Acceptance gate checklist:
- Dependency:
  - `ScriptCommon.upp` and `ScriptCLI.upp` exclude GUI packages.
  - Source files under both packages compile without GUI includes.
- Functional:
  - ScriptCLI can run and lint scripts via ScriptCommon.
  - Headless plugin tests pass.
- Integration:
  - ScriptIDE still builds and uses ScriptCommon APIs through adapters.
- Forward-compatibility:
  - Core APIs are frontend-agnostic and suitable for future TUI layer.

## Risks
- Partial migration accepted too early.
- Hidden transitive dependency violations.

## Acceptance Criteria
- [ ] Dependency gates are measurable.
- [ ] Functional and integration gates are measurable.
- [ ] Future TUI-readiness criteria are documented.
