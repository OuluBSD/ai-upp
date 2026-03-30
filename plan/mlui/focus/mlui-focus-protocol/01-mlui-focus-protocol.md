# MLUI Focus Plan - Protocol Endpoints

## Objective
Extend MLUI JSON protocol with Focus endpoints so MCP/agents can query compact pages instead of traversing full raw control trees.

## Current Status
- Core MLUI runtime and request/response parser exist in `Core`.
- CtrlLib MLUI handler now supports focus methods:
  - `focus.list`, `focus.get`, `focus.tree`, `focus.search`, `focus.action`
- Core responses now include `protocol_version`.

## Progress Update (2026-03-30)
Completed:
1. Implemented `focus.*` request handlers in `uppsrc/CtrlLib/Mlui.cpp`.
2. Added param normalization for aliases (e.g. `page_id/page/id`, `action_id/action/id`).
3. Added validation/error responses for missing parameters.
4. Added bounded validation:
   - `focus.tree.depth` in `[1..16]`
   - `focus.search.limit` in `[1..1024]`
5. Added protocol version marker in `MluiMakeJsonResponse` (`uppsrc/Core/Mlui.cpp`).
6. Added focus runtime refresh before `focus.*` evaluation to ensure action handlers/state are available.

Remaining:
1. Optional schema strictness pass (type validation for all known params, not only presence/ranges).
2. Optional protocol documentation block for each `focus.*` method in Topic++ docs.

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
