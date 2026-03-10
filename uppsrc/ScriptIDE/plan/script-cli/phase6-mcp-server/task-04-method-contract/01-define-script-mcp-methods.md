# Task: Define Script MCP Method Contracts

## Goal
Specify first MCP method set for ScriptCLI/ScriptCommon and align it with pane-service/runtime needs.

## Background / Rationale
MCP clients need stable method contracts and deterministic payloads for automation.

## Scope
- Method names, params, result schemas, error codes.
- Capability advertisement entries.
- Pagination/filtering conventions for list-heavy methods.

## Non-goals
- Full parity with TheIDE MCP method list.
- Tool-specific client UX docs.

## Dependencies
- ScriptCommon service interfaces.
- ScriptCLI MCP host design.

## Concrete Investigation Steps
1. Define MVP methods:
   - `mcp.ping`, `mcp.capabilities`
   - `script.run`, `script.lint`
   - `plugin.list`, `plugin.test`
   - `files.list`, `search.find`, `outline.get`, `variables.list`, `history.list`
2. Define request/response JSON schema per method.
3. Define standard error mapping and partial-result semantics.
4. Define extension policy for future TUI/IDE parity methods.

## Affected Subsystems
- ScriptCommon MCP protocol/handlers
- ScriptCLI MCP host
- Headless plugin test harness

## Implementation Direction
Contract style should mirror existing MCP core conventions:
- JSON-RPC 2.0 request/response envelope
- `mcp.capabilities` declares methods + protocol flags
- deterministic field names and stable optional fields

## Risks
- Method naming drift across CLI and MCP surfaces.
- Inconsistent schema evolution.

## Acceptance Criteria
- [ ] MVP method table with schemas is documented.
- [ ] Error code and capability conventions are documented.
- [ ] Mapping from methods to ScriptCommon services is complete.
