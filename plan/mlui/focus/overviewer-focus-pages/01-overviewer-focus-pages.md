# MLUI Focus Plan - Overviewer Integration

## Objective
Integrate Focus pages into `uppsrc/Overviewer` so MCP can reliably access high-value context (e.g., file tree + active file) without parsing entire raw UI snapshots.

## Current Status
- Overviewer now has production focus pages integrated.
- End-to-end focus actions are callable over MLUI server.

## Progress Update (2026-03-30)
Completed:
1. Added initial focus pages in `uppsrc/Overviewer/Project.cpp`:
   - `file_tree`
   - `active_file`
2. Wired runtime state emission through `OverviewerWindow::Access(Visitor&)`:
   - project file path
   - root dir
   - selected relative path
   - selected metadata values
3. Added action handlers:
   - `file_tree.select`
   - `file_tree.reload`
   - `active_file.analyze`
   - `active_file.set_priority`
   - `active_file.add_comment`
4. Added targeted tree-path selection helper to avoid expensive full-tree expand in action flow.
5. Verified with MLUI socket smoke calls against `bin/Overviewer --mlui-server__ ...`.

Remaining:
1. Add one more page for project-wide summary (dashboard/session/active scenario) to reduce fallback snapshot usage.
2. Stabilize/normalize returned path semantics further for cross-platform separators in action args.

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
