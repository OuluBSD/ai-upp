# Task: Design ScriptCommon MCP Core Layer

## Goal
Define a headless MCP core in `ScriptCommon` that exposes script-runtime and pane-service capabilities without GUI dependencies.

## Background / Rationale
MCP handlers should share the same runtime logic as CLI/GUI adapters and must remain frontend-neutral.

## Scope
- MCP method handlers bound to ScriptCommon services.
- Capability advertisement for Script domain methods.
- Error and response conventions.

## Non-goals
- Implementing IDE workspace/build/debug MCP methods from TheIDE.
- HTTP/SSE transport migration.

## Dependencies
- Pane service interfaces from phase2.
- Runtime/plugin services in ScriptCommon.
- MCP reference audit from phase6/task-01.

## Concrete Investigation Steps
1. Define Script-domain MCP namespaces (e.g., `script.*`, `plugin.*`, `analysis.*`).
2. Map each MCP method to a ScriptCommon service call.
3. Define method-level request/response schemas.
4. Define capability payload and versioning.

## Affected Subsystems
- New `uppsrc/ScriptCommon/*` MCP handler files
- Future `uppsrc/ScriptCLI/*` MCP host wiring

## Implementation Direction
Planned flat files in `ScriptCommon`:
- `ScriptMcpProtocol.h`
- `ScriptMcpHandlers.h/.cpp`
- `ScriptMcpCapabilities.h/.cpp`

Core MCP responsibilities:
- pure request validation and service dispatch
- JSON-RPC result/error mapping
- no socket/thread ownership in this layer

## Risks
- API duplication between CLI commands and MCP methods.
- Unstable method contracts without explicit versioning.

## Acceptance Criteria
- [ ] ScriptCommon MCP handler layer is defined.
- [ ] Method-to-service mapping is complete.
- [ ] No GUI or transport-specific dependencies are introduced.
