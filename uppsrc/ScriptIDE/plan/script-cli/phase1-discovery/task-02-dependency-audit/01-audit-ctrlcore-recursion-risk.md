# Task: Audit and Enforce No-CtrlCore Boundary for ScriptCommon/ScriptCLI

## Goal
Define a strict dependency boundary proving `ScriptCommon` and `ScriptCLI` remain free of `CtrlCore`/GUI recursion.

## Background / Rationale
Headless plugin development/testing breaks when GUI stack is pulled in transitively. This boundary is a hard requirement.

## Scope
- Define allowed dependencies for `ScriptCommon` and `ScriptCLI`.
- Define verification checks.
- Define remediation workflow when violations appear.

## Non-goals
- Implementing CI scripts yet.
- Deciding final TUI library.

## Dependencies
- `ScriptIDE.upp` current dependency graph.
- U++ package conventions.

## Concrete Investigation Steps
1. Build a dependency matrix for candidate extracted files.
2. Define allowed package list for `ScriptCommon` (e.g., `Core`, `ByteVM`, other non-GUI packages only).
3. Define allowed package list for `ScriptCLI` (headless runtime + `ScriptCommon`).
4. Define static checks:
   - No `#include <CtrlCore/...>`
   - No `#include <CtrlLib/...>`
   - `.upp` uses section excludes GUI packages
5. Define break-glass handling for unavoidable mixed classes (split into core + adapter).

## Affected Subsystems
- Future `uppsrc/ScriptCommon/ScriptCommon.upp`
- Future `uppsrc/ScriptCLI/ScriptCLI.upp`
- Existing `uppsrc/ScriptIDE/ScriptIDE.upp`

## Implementation Direction
Boundary policy document should include:
- Allowed/forbidden package list.
- Header include examples (allowed vs forbidden).
- Review checklist used in each migration task.

## Risks
- Hidden GUI usage via shared helper headers.
- Macro leakage from GUI headers into common code.

## Acceptance Criteria
- [ ] Allowed and forbidden dependencies are explicitly documented.
- [ ] Verification checklist is defined and actionable.
- [ ] Split strategy exists for mixed classes.
