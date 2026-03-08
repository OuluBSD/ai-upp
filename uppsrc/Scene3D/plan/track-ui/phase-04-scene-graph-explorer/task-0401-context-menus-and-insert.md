# Task 0401 - Context Menus + Insert Submenu

## Goal
Implement context menus defined by scene_graph_explorer.xml without restricting existing commands.

## Scope
- Root context menu with Insert submenu, create folder node, focus selected.
- Node context menu with clone/delete, modify selection submenu, insert submenu, focus.
- Insert submenu items map to creation actions.
- Menu availability respects enabled_state (e.g., create path node).

## Acceptance
- Context menus appear on right-click for root and non-root nodes.
- Insert menu items trigger correct creation flow.
- Existing tree context options remain available.
