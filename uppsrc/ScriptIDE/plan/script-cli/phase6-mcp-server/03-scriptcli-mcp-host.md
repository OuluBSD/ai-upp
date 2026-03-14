# Task: Design ScriptCLI MCP Host Runtime

## Goal
Define `ScriptCLI` as the thin MCP host that owns process lifecycle and transport wiring while delegating all Script-domain request handling to `ScriptCommon`.

## Design Decision
`ScriptCLI` will host the server process, parse CLI flags, construct the ScriptCommon service context, and route requests into ScriptCommon handlers. It should stop embedding domain method logic in `McpCommand.cpp`.

## Current State
- `ScriptCLI mcp serve` already exists
- default transport is stdio
- TCP help text exists, but TCP mode is not implemented
- request handling and runtime logic are mixed together in a single file

## Planned File Set
- `uppsrc/ScriptCLI/McpCommand.cpp`
  - reduced to argument parsing and host startup
- `uppsrc/ScriptCLI/McpHost.h`
- `uppsrc/ScriptCLI/McpHost.cpp`
- optional:
  - `uppsrc/ScriptCLI/McpHostMain.h`
  - `uppsrc/ScriptCLI/McpHostMain.cpp`

## Host Responsibilities
- parse `ScriptCLI mcp serve` flags
- normalize workspace path
- build the ScriptCommon service context
- select transport mode
- start the stdio or TCP host loop
- route parsed requests into ScriptCommon MCP handlers
- exit cleanly on EOF, signal, or transport shutdown

## Proposed Runtime Split

### McpCommand
- CLI-facing usage/help
- argument validation
- startup config assembly
- calls `McpHost::Run(config)`

### McpHost
- owns host lifecycle
- owns transport implementation
- owns protocol loop integration
- does not own domain logic

## Transport Plan

### Default: `stdio`
Reason:
- best fit for AI tool integration
- already aligned with current usage text
- no port management needed

Behavior:
- one JSON-RPC request per newline-delimited JSON object
- one response per line
- notifications produce no response
- EOF stops the host cleanly

### Optional: `tcp`
Reason:
- useful for local debugging and manual integration
- aligns with the existing `uppsrc/MCP` transport shape

Behavior:
- not required for the first code landing if it delays the boundary cleanup
- if implemented, it should reuse the `uppsrc/MCP` framing/client model rather than inventing a second transport pattern

## Startup Flags
- `--workspace <path>`
  - default: current directory
- `--transport stdio|tcp`
  - default: `stdio`
- `--port <n>`
  - default: `7326` when TCP is used
- future optional flags:
  - `--verbose`
  - `--log-file <path>`

## Delegation Boundary
- `ScriptCLI` owns:
  - config
  - transport
  - process lifecycle
- `ScriptCommon` owns:
  - method inventory
  - param validation
  - result payloads
  - service dispatch

## Graceful Shutdown Rules
- stdio mode exits on stdin EOF
- TCP mode exits on signal or explicit stop
- in-flight request must finish before shutdown where practical
- no partially written JSON response

## Implementation Notes
- if phase 6 reuses `uppsrc/MCP`, keep the dependency direction acceptable for the split
- if direct package reuse is awkward, mirror its architecture, not the current monolithic `McpCommand.cpp`
- do not let CLI text formatting leak back into MCP responses

## Acceptance Criteria
- [x] ScriptCLI host lifecycle is documented
- [x] startup flags and defaults are explicit
- [x] stdio-first strategy is explicit
- [x] boundary between ScriptCLI host and ScriptCommon handlers is explicit
