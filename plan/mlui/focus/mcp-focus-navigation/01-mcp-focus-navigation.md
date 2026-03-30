# MLUI Focus Plan - MCP Navigation UX

## Objective
Provide practical MCP tooling (`pysrc/bin/mcp_mlui.py`) for browsing large MLUI trees through focus pages using shell-like navigation and fast filtering.

## Current Status
- MCP MLUI tooling now includes focus forwarding and compact detail helpers.
- Path navigation (`cd/ls/tree/find`) is available and working with snapshot index.

## Progress Update (2026-03-30)
Completed:
1. Added focus method forwarding in `pysrc/bin/mcp_mlui.py`:
   - `mlui.focus.list`, `mlui.focus.get`, `mlui.focus.tree`, `mlui.focus.search`, `mlui.focus.action`
2. Added compact detail endpoint:
   - `mlui.focus.detail` (page summary + optional item detail for value/ctrl/action)
3. Added helper command:
   - `mcp.help` with command flow examples
4. Updated README method list/examples for focus methods.

Remaining:
1. Add explicit arg schemas to `mcp.help` output for each focus method (machine-readable).
2. Add one “session macro” helper flow (`select -> set_priority -> verify`) for common Overviewer workflows.

## Next Tasks
1. Add path-oriented focus navigation commands:
   - `cd`, `ls`, `tree` (depth-limited)
2. Add focused wildcard search (`*text`) with predictable matching rules.
3. Add compact detail view command for one selected node/page/action.
4. Add command help and examples tuned for agent usage.

## Notes
- Default outputs should prefer compact summaries over full dumps.
- Depth and result limits must have conservative defaults.

## Risks
- If defaults are too broad, tools remain noisy and costly.
- If defaults are too strict, important context may be hidden.
