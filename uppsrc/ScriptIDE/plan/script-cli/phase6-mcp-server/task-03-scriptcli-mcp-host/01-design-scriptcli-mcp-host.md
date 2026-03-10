# Task: Design ScriptCLI MCP Host Runtime

## Goal
Define the `ScriptCLI` MCP server host that owns transport/thread lifecycle and delegates method handling to ScriptCommon MCP handlers.

## Background / Rationale
Following the `uppsrc/MCP` + wrapper pattern, ScriptCLI should host MCP transport while ScriptCommon owns domain logic.

## Scope
- MCP host process lifecycle in ScriptCLI.
- Port/stdin modes and startup flags.
- Handler binding to ScriptCommon service graph.

## Non-goals
- Integrating with TheIDE UI thread.
- Implementing remote auth model.

## Dependencies
- ScriptCommon MCP core design.
- MCP reference audit from phase6/task-01.

## Concrete Investigation Steps
1. Define `scriptcli mcp serve` mode behavior.
2. Define default transport mode (TCP first, optional stdio proxy mode).
3. Define startup config (`--port`, `--workspace`, logging verbosity).
4. Define graceful shutdown and signal handling.

## Affected Subsystems
- Future `uppsrc/ScriptCLI/*` runtime files
- `ScriptCLI` command registry

## Implementation Direction
Planned flat files in `ScriptCLI`:
- `McpHost.h/.cpp`
- `McpHostMain.h/.cpp` (or integrated in Main/CommandRegistry)

Host responsibilities:
- initialize ScriptCommon services
- start MCP server transport
- wire handlers/capabilities
- provide health/log endpoints

## Risks
- Long-running server mode conflicting with one-shot CLI command flow.
- Resource cleanup issues on abrupt termination.

## Acceptance Criteria
- [ ] MCP host lifecycle is documented.
- [ ] CLI startup flags and defaults are documented.
- [ ] Delegation boundary to ScriptCommon handlers is explicit.
