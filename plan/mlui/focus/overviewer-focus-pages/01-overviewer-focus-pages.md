# MLUI Focus Plan - Overviewer Integration

## Objective
Integrate Focus pages into `uppsrc/Overviewer` so MCP can reliably access high-value context (e.g., file tree + active file) without parsing entire raw UI snapshots.

## Current Status
- Overviewer already has MLUI usability issues from raw snapshot scale.
- Focus model exists in references but not integrated into production app flow.

## Next Tasks
1. Add initial Focus pages in Overviewer (minimum two):
   - `file_tree`
   - `active_file`
2. Expose page runtime state in `Access(Visitor&)`:
   - root/project path
   - selected node/file
   - open document metadata
3. Add action contracts for common operations (select/open/reload).
4. Validate end-to-end via MLUI protocol + MCP scripts.

## Risks
- If page boundaries are unclear, pages may still become too large.
- Action mapping may drift from real UI behavior if not bound to concrete controls/state.
