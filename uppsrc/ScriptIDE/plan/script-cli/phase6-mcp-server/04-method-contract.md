# Task: Define ScriptCLI MCP Method Contracts

## Goal
Specify the first stable MCP method set for the ScriptCLI split and bind each method to a real or planned headless service.

## Contract Rules
- envelope: JSON-RPC 2.0
- params:
  - object unless explicitly documented otherwise
- results:
  - deterministic field names
  - stable optional fields
- errors:
  - use JSON-RPC standard codes for request/protocol issues
  - use deterministic messages for domain failures

## Capability Payload

### `mcp.capabilities`
Result fields:
- `protocol`: `"jsonrpc-2.0"`
- `surface`: `"scriptcli-mcp"`
- `version`: phase-6 version tag, for example `"v1"`
- `transport`: `"stdio"` or `"tcp"`
- `supports_batch`: `false`
- `methods`: array of method names

## MVP Methods

### `mcp.ping`
Params:
- none or empty object

Result:
- `text`: `"pong"`
- `workspace`: normalized workspace path

Errors:
- none beyond standard request errors

### `workspace.info`
Params:
- none or empty object

Result:
- `workspace`: normalized workspace root
- `cwd`: current process directory

### `script.run`
Params:
- `path`: string, required

Result:
- `ok`: bool
- `path`: resolved absolute path
- `kind`: `"python"` or plugin-defined document type
- optional `error`: string when domain layer chooses result-style reporting

Notes:
- relative paths resolve against workspace
- initial implementation may support `.py` and `.gamestate`

### `script.lint`
Params:
- `path`: string, required

Result:
- `ok`: bool
- `path`: resolved absolute path
- `issues`: array

Issue item fields:
- `line`: int
- `column`: int
- `severity`: `"error"` or `"warning"`
- `text`: string

### `plugin.list`
Params:
- none or empty object

Result:
- `plugins`: array

Plugin item fields:
- `id`
- `name`
- `description`

### `plugin.test`
Params:
- `plugin_id`: string, required
- `case`: string, optional

Result:
- `plugin_id`
- `ok`
- `passed`
- `failed`
- `results`: array

Per-test item fields:
- `name`
- `ok`
- optional `message`

## Planned But Deferred Methods
- `files.list`
- `search.find`
- `outline.get`
- `variables.list`
- `history.list`

These stay out of the first implementation unless matching ScriptCommon services are extracted.

## Error Mapping

### JSON-RPC Standard
- `-32700` parse error
- `-32600` invalid request
- `-32601` method not found
- `-32602` invalid params
- `-32603` internal error

### Domain Failure Conventions
- missing file:
  - `-32602` with deterministic message
- plugin not found:
  - `-32602` or `-32603`
  - prefer `-32602` when the request identifies a nonexistent plugin
- runtime failure while executing valid input:
  - `-32603`
- not yet implemented method:
  - `-32601`

## Method-To-Service Mapping
- `mcp.ping`
  - ScriptCommon MCP helper
- `mcp.capabilities`
  - ScriptCommon capability provider
- `workspace.info`
  - ScriptCLI host context plus ScriptCommon helper
- `script.run`
  - shared run service over `RunManager` and plugin execution support
- `script.lint`
  - shared lint service over `Linter`
- `plugin.list`
  - shared plugin catalog service over `GetInternalPluginFactories()`
- `plugin.test`
  - shared plugin test service extracted from current CLI command logic

## Acceptance Criteria
- [x] MVP method set is explicit
- [x] request and result schemas are defined
- [x] error conventions are defined
- [x] methods are mapped to real or planned shared services
