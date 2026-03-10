# ScriptCLI / ScriptCommon Plan Track

## Goal
Create a clean split where non-GUI scripting/runtime/plugin core is moved from `ScriptIDE` into a new `ScriptCommon` package, and a separate headless `ScriptCLI` package is introduced for plugin development, MCP tooling, and automated testing.

## Why This Track Exists
`ScriptIDE` currently mixes GUI and non-GUI responsibilities. For headless plugin development/testing, MCP integrations, and future TUI IDE work, we need a shared non-GUI core with no `CtrlCore` dependency.

## High-Level Constraints
- `ScriptCommon` and `ScriptCLI` must not depend on `CtrlCore` or `CtrlLib`.
- GUI concerns stay in `ScriptIDE` (and later TUI package).
- APIs in `ScriptCommon` should be presentation-agnostic and reusable from GUI, CLI, MCP, and future TUI frontends.

## Planned Phases
1. `phase1-discovery`: inventory and dependency audit.
2. `phase2-scriptcommon-architecture`: define package/file structure, API boundaries, and pane-service interfaces.
3. `phase3-migration-execution`: move non-GUI classes and add GUI adapters in ScriptIDE.
4. `phase4-scriptcli-bootstrap`: define and bootstrap headless CLI package.
5. `phase5-test-headless`: test strategy and acceptance gates.
6. `phase6-mcp-server`: add MCP server architecture (ScriptCommon core + ScriptCLI host), referencing `uppsrc/ide/MCP` + `uppsrc/MCP` patterns.

## Deliverables
- Phase/task markdowns with concrete file structure, migration order, risks, and acceptance criteria.
- A review-ready extraction + MCP plan before any implementation edits.
