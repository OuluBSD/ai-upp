# Task: Audit MCP Reference for ScriptCLI and ScriptCommon

## Goal
Capture the reusable architecture from `uppsrc/MCP` and `uppsrc/ide/MCP`, then apply it to the ScriptIDE split so `ScriptCommon` owns domain logic and `ScriptCLI` owns host lifecycle.

## Status Summary
- `uppsrc/MCP` already provides a clean headless MCP core:
  - JSON-RPC parsing and response helpers in `uppsrc/MCP/Protocol.h`
  - transport, framing, client state, and core method routing in `uppsrc/MCP/Server.*`
- `uppsrc/ide/MCP` already demonstrates the frontend wrapper pattern:
  - `McpServer : McpServerCore`
  - frontend-specific methods implemented via `HandleExtended`
- `uppsrc/ScriptCLI` currently has an ad hoc MCP implementation in `uppsrc/ScriptCLI/McpCommand.cpp`:
  - stdio loop only
  - local JSON-RPC helpers duplicated instead of reused
  - method handling mixed directly with CLI entrypoint logic

## Reference Pattern To Reuse

### 1. Headless Core Owns Transport and Core Methods
From `uppsrc/MCP`:
- `McpServerCore` owns:
  - server start/stop
  - framing
  - per-client buffers
  - request dispatch
  - common discovery methods like `mcp.ping` and `mcp.capabilities`
  - logging and request timing
- Extension boundary:
  - `virtual String HandleExtended(const McpRequest& req)`

### 2. Frontend Wrapper Owns Domain Methods
From `uppsrc/ide/MCP`:
- `McpServer` subclasses `McpServerCore`
- wrapper-specific handlers live in `HandleExtended`
- wrapper wires the service graph and frontend lifecycle

### 3. Capabilities Are Declared Centrally
- `mcp.capabilities` is a first-class discovery method
- method inventory is explicit
- protocol flags are carried in a predictable payload

### 4. Protocol Helpers Must Be Shared
- `Protocol.h` already defines request parsing, result generation, and error mapping
- ScriptCLI should stop maintaining its own parallel JSON-RPC helper set

## Reuse vs Rewrite Decisions

### Reuse Directly
- JSON-RPC helper style from `uppsrc/MCP/Protocol.h`
- server/core split from `uppsrc/MCP/Server.*`
- wrapper pattern from `uppsrc/ide/MCP/Server.*`
- capability-first discovery model
- newline-delimited stdio/TCP framing model

### Adapt, Not Copy Blindly
- capability contents:
  - ScriptCLI needs a much smaller method surface than TheIDE
- logging:
  - ScriptCLI can start with lighter logging than TheIDE MCP
- transport defaults:
  - ScriptCLI should default to stdio for AI tooling, even if TCP support is retained as optional

### Do Not Reuse As-Is
- IDE-only handlers in `uppsrc/ide/MCP/Server.cpp`
- GUI-thread assumptions and bridge objects
- workspace/build/debug/layout/editor methods that depend on TheIDE state

## ScriptCLI Compatibility Constraints
- `ScriptCommon` and `ScriptCLI` must remain free of `CtrlCore` and `CtrlLib`
- `ScriptCommon` must not own socket or thread lifecycle
- domain handlers must call headless services, not CLI formatting code
- `ScriptCLI` must remain automation-friendly:
  - deterministic JSON payloads
  - stable exit codes
  - no interactive prompts

## Concrete Implications For Phase 6
- move method dispatch out of `uppsrc/ScriptCLI/McpCommand.cpp`
- add a ScriptCommon handler layer for Script-domain methods
- keep ScriptCLI as a thin host over that handler layer
- align method naming and schemas with actual reusable services, not CLI text output

## Deliverables
- a ScriptCommon MCP handler layer plan
- a ScriptCLI MCP host plan
- a documented MVP method contract
- a transport-level and handler-level test plan

## Acceptance Criteria
- [x] Reference architecture is mapped to concrete files and classes
- [x] Reuse vs rewrite choices are explicit
- [x] ScriptCLI and ScriptCommon boundary constraints are documented
