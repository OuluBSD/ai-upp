# MLUI Focus Plan - MCP Navigation UX

## Objective
Provide practical MCP tooling (`pysrc/bin/mcp_mlui.py`) for browsing large MLUI trees through focus pages using shell-like navigation and fast filtering.

## Current Status
- MCP MLUI tooling exists but focus-page aware UX is incomplete.
- Large raw row sets are difficult to consume directly.

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
