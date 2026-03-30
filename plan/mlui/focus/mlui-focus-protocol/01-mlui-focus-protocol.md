# MLUI Focus Plan - Protocol Endpoints

## Objective
Extend MLUI JSON protocol with Focus endpoints so MCP/agents can query compact pages instead of traversing full raw control trees.

## Current Status
- Core MLUI runtime and request/response parser exist in `Core`.
- CtrlLib MLUI handler currently supports: `ping`, `snapshot`, `click`, `set`, `key`, `mouse`.
- No focus-specific protocol methods yet.

## Next Tasks
1. Define request/response schema for:
   - `focus.list`
   - `focus.get`
   - `focus.tree`
   - `focus.search`
   - `focus.action`
2. Implement schema validation and consistent error responses for missing/invalid params.
3. Ensure backward compatibility for existing non-focus methods.
4. Add version marker in result payload to allow future protocol evolution.

## Risks
- Endpoint overlap with raw snapshot methods may create ambiguity in client behavior.
- Weak action contracts can make `focus.action` unsafe or nondeterministic.
- Search semantics can become expensive without bounded scope defaults.
