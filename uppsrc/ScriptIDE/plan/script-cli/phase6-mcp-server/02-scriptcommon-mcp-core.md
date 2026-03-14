# Task: Design ScriptCommon MCP Core Layer

## Goal
Define the headless MCP domain layer in `ScriptCommon` so CLI, MCP, and future frontends all call the same script-runtime services.

## Design Decision
`ScriptCommon` will own request validation, service dispatch, and JSON-value response construction for Script-domain methods. It will not own sockets, threads, stdio loops, or process lifecycle.

## Why This Is Needed
- current MCP handling in `uppsrc/ScriptCLI/McpCommand.cpp` duplicates runtime logic
- `script.run` and `script.lint` are implemented directly in the CLI MCP path instead of through shared services
- `plugin.list` and `plugin.test` have no reusable headless service layer yet

## Planned File Set
- `uppsrc/ScriptCommon/ScriptMcpProtocol.h`
- `uppsrc/ScriptCommon/ScriptMcpHandlers.h`
- `uppsrc/ScriptCommon/ScriptMcpHandlers.cpp`
- `uppsrc/ScriptCommon/ScriptMcpCapabilities.h`
- `uppsrc/ScriptCommon/ScriptMcpCapabilities.cpp`
- optional shared service files if command logic must be extracted first:
  - `ScriptServices.h/.cpp`
  - or narrower service files for run, lint, and plugin operations

## Responsibilities

### ScriptMcpProtocol
- declare Script-domain method names
- declare version string for the ScriptCLI MCP surface
- define helper conventions for:
  - params object validation
  - error payload construction
  - deterministic result field names

### ScriptMcpHandlers
- accept parsed request data plus a service context
- validate params
- dispatch to ScriptCommon services
- return JSON-compatible `Value` or JSON-RPC error payloads

### ScriptMcpCapabilities
- provide the ScriptCLI capability payload
- expose:
  - protocol version
  - transport flags
  - supported methods
  - optional method metadata/version tag

## Required Shared Service Context
The handler layer needs a headless service graph. Minimum context:
- workspace root
- `PyVM` creation strategy or runtime factory
- run service
- lint service
- plugin catalog service
- plugin test service

Planned context shape:
- `ScriptMcpContext`
  - `String workspace`
  - service references or function objects for run/lint/plugin operations

## Initial Method Set Bound To Services

### Core
- `mcp.ping`
- `mcp.capabilities`
- `workspace.info`

### Script Runtime
- `script.run`
- `script.lint`

### Plugin
- `plugin.list`
- `plugin.test`

## Deferred Until Services Exist
- `files.list`
- `search.find`
- `outline.get`
- `variables.list`
- `history.list`

These remain phase-6 contract placeholders only until matching ScriptCommon services are extracted.

## Service Extraction Required Before Or During MCP Work

### Run
Current state:
- command logic split between `RunCommand.cpp` and `RunManager.cpp`

Needed:
- a shared service returning structured results such as:
  - `ok`
  - `path`
  - `runtime_error`
  - optional captured stdout/stderr metadata if later added

### Lint
Current state:
- `Linter` is reusable
- CLI command still formats plain text itself

Needed:
- a structured lint result service:
  - `path`
  - `issues[]`
  - `ok`

### Plugin List
Current state:
- logic only in `PluginCommand.cpp`

Needed:
- shared enumeration service returning:
  - plugin id
  - name
  - description

### Plugin Test
Current state:
- logic only in `PluginCommand.cpp`

Needed:
- shared test service returning:
  - plugin id
  - passed count
  - failed count
  - per-test results
  - overall status

## Error Conventions
- use JSON-RPC standard envelope codes for transport/request errors
- map domain failures to deterministic internal error payloads
- separate:
  - invalid params
  - infrastructure failure
  - runtime failure
  - method not implemented

## Non-Goals
- TCP socket ownership
- stdio read/write loop
- GUI bridge logic
- TheIDE workspace or debug integration

## Acceptance Criteria
- [x] Headless ScriptCommon MCP layer boundary is defined
- [x] Required file set is identified
- [x] Method-to-service mapping is defined for the initial realistic MVP
- [x] No GUI or transport-specific dependencies are introduced into the design
