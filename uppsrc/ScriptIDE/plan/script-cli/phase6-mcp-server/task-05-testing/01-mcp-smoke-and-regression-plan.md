# Task: Define MCP Smoke and Regression Plan for ScriptCLI

## Goal
Define repeatable tests for ScriptCLI MCP server behavior and ScriptCommon handler correctness.

## Background / Rationale
MCP is a critical integration surface for headless AI workflows and must be validated independently of GUI.

## Scope
- Smoke tests for startup/health/capabilities.
- Functional tests for script/pane/plugin methods.
- Regression tests for framing/protocol errors.

## Non-goals
- Performance benchmarking at scale.
- TheIDE MCP regression coverage.

## Dependencies
- ScriptCLI MCP host design.
- ScriptCommon MCP method contracts.

## Concrete Investigation Steps
1. Define local smoke script sequence (`mcp.ping`, `mcp.capabilities`, sample method calls).
2. Define negative tests (invalid JSON, invalid params, unknown method).
3. Define deterministic fixture workspace and expected responses.
4. Define CI invocation path for MCP tests.

## Affected Subsystems
- Future `upptst/*` MCP tests
- ScriptCLI MCP host
- ScriptCommon MCP handlers

## Implementation Direction
Test layers:
- Unit tests for handler request validation.
- Integration tests over live transport.
- CLI-level end-to-end tests using scripted request files.

## Risks
- Flaky tests due to non-deterministic runtime state.
- Transport-level tests hiding handler-level regressions.

## Acceptance Criteria
- [ ] Smoke test checklist is documented.
- [ ] Negative/protocol test cases are documented.
- [ ] CI execution strategy is documented.
