# Task: Audit MCP Reference in uppsrc/ide and uppsrc/MCP

## Goal
Capture reusable MCP architecture patterns from `uppsrc/MCP` (headless core) and `uppsrc/ide/MCP` (IDE integration wrapper) for ScriptCLI/ScriptCommon design.

## Background / Rationale
The existing TheIDE MCP stack already demonstrates a proven split between transport core and frontend-specific handlers.

## Scope
- Audit key classes and extension points in reference packages.
- Identify what can be reused directly vs copied as pattern.
- Define constraints for ScriptCLI adaptation.

## Non-goals
- Modifying `uppsrc/ide/MCP`.
- Full protocol redesign.

## Dependencies
- `uppsrc/MCP/Server.*`
- `uppsrc/ide/MCP/Server.*`
- `uppsrc/ide/MCP/AGENTS.md`

## Concrete Investigation Steps
1. Document `McpServerCore` responsibilities in `uppsrc/MCP`.
2. Document wrapper pattern (`McpServer : McpServerCore`) in `uppsrc/ide/MCP`.
3. Document method routing pattern (`HandleExtended`).
4. Document framing, logging, and capability discovery patterns.

## Affected Subsystems
- New ScriptCommon MCP core plan
- New ScriptCLI MCP host plan

## Implementation Direction
Use the same architecture style:
- Headless core package owns protocol + transport + base methods.
- Frontend package owns method handlers bound to its service graph.

## Risks
- Coupling ScriptCLI MCP too tightly to TheIDE-specific assumptions.

## Acceptance Criteria
- [ ] Reference pattern document exists with concrete file/class mapping.
- [ ] Reuse vs rewrite decisions are explicit.
- [ ] ScriptCLI compatibility constraints are documented.
