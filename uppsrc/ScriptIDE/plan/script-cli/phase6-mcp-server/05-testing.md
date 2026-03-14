# Task: Define MCP Smoke and Regression Plan for ScriptCLI

## Goal
Define a repeatable test strategy for the new ScriptCLI MCP surface so protocol, transport, and handler regressions are caught independently of the GUI.

## Test Layers

### 1. Handler-Level Tests
Focus:
- request validation
- path resolution
- deterministic response payloads
- domain error mapping

Targets:
- `mcp.ping`
- `mcp.capabilities`
- `workspace.info`
- `script.run`
- `script.lint`
- `plugin.list`
- `plugin.test`

### 2. Host Integration Tests
Focus:
- stdio framing
- EOF shutdown
- unknown method behavior
- parse and invalid-request handling

### 3. End-To-End CLI Tests
Focus:
- `ScriptCLI mcp serve`
- request fixture files
- response stability in a real workspace

## Smoke Sequence

### Minimal Happy Path
1. start `ScriptCLI mcp serve`
2. send `mcp.ping`
3. send `mcp.capabilities`
4. send `workspace.info`
5. send `script.lint` for a known-valid file
6. send `script.run` for a known-valid file
7. send `plugin.list`

Expected:
- every response is valid JSON
- every response echoes `jsonrpc: "2.0"`
- ids are preserved
- capability list contains only implemented methods

## Negative Tests

### Protocol Errors
- malformed JSON
- non-object request
- missing `method`
- wrong `jsonrpc` version
- notification request with no `id`

Expected:
- malformed JSON returns parse error
- structurally invalid request returns invalid request
- notification produces no response

### Param Errors
- `script.run` without `path`
- `script.run` for missing file
- `script.lint` without `path`
- `plugin.test` without `plugin_id`
- `plugin.test` with unknown plugin id

Expected:
- deterministic `invalid params` or explicit documented domain error

### Method Errors
- unknown method name

Expected:
- `method not found`

## Fixture Workspace
Create or reuse a deterministic test workspace containing:
- one valid Python file
- one invalid Python file
- one valid `.gamestate` file if plugin execution is in the MVP
- a plugin test fixture directory for at least one internal plugin

## Suggested Fixture Assertions
- `script.lint` valid file:
  - `ok == true`
  - empty `issues`
- `script.lint` invalid file:
  - `ok == false`
  - at least one issue
- `script.run` valid file:
  - `ok == true`
- `plugin.list`:
  - known internal plugin ids present
- `plugin.test` known plugin:
  - result shape stable even if failures occur

## CI Strategy
- keep handler-level tests fast and deterministic
- run stdio integration tests in CI
- add TCP tests only after TCP is actually implemented
- avoid brittle timing assumptions

## Regression Risks To Watch
- CLI text leaking into JSON responses
- capability list advertising methods not implemented by handlers
- stdio framing breaking on multi-line or partial input
- divergence between CLI command logic and MCP handler logic
- plugin test behavior depending on host-specific paths

## Exit Criteria
- handler tests cover all implemented methods
- negative tests cover parse, invalid request, invalid params, and unknown method
- stdio smoke test passes in CI
- MCP responses remain stable across repeated runs

## Acceptance Criteria
- [x] Smoke checklist is defined
- [x] negative tests are defined
- [x] fixture workspace requirements are defined
- [x] CI execution strategy is defined
