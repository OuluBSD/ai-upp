# FormEditor Adaptation & Integration Strategy

## Overview
We will adapt U++'s native `FormEditor` to serve as a WYSIWYG designer for `.form` files (formerly `.xlay`). Integration must go beyond simple `Ctrl` embedding; the editor's menus and toolbars must merge with ScriptIDE's main interface, and its property panes must integrate with the docking system.

## Reusable Subsystems
- **Property Grid**: `FormProperties.cpp` logic for attribute editing.
- **Manipulation Logic**: Selection, dragging, resizing, and snapping in `FormView`.
- **Undo/Redo**: Core stack in `FormEdit.hpp`.

## ScriptIDE Integration Requirements

### 1. Main Menu Integration
When a `.form` editor is active:
- The ScriptIDE main menu must gain a "Layout" top-level menu (or merge into "Search/Source").
- Specific actions like "Align", "Distribute", and "Bring to Front" must be added to the IDE's main toolbar or a dedicated context-sensitive toolbar.

### 2. Dynamic Dockables
The `FormEditor`'s "Properties" and "Toolbox" panes must be registered as dockable panes in `ScriptIDE`.
- **Lifecycle**: These panes should be added to the `DockWindow` only when a `.form` editor tab is active.
- **Auto-Hide**: When switching to a `.py` or `.gamestate` tab, these editor-specific panes must be hidden or removed from the layout to avoid cluttering the workspace.

### 3. Placeholder Panes
ScriptIDE will provide "Contextual Panes" (e.g., `ContextualPane1`, `ContextualPane2`) which are essentially empty containers.
- The active `IDocumentHost` can "claim" these placeholders to display its specific secondary UI.
- Example: The `.form` editor uses `ContextualPane1` for the Property Grid and `ContextualPane2` for the Widget Toolbox.

## Adaptations for Card Games
- **Custom Serializer**: Use JSON/YAML instead of `.lay` format.
- **Game Toolbox**: Provide `Zone`, `CardSlot`, and `Sprite` primitives.
- **Asset Integration**: Toolbox items should be able to browse the project's `assets/` folder.

## Integration Workflow
1. `PythonIDE` detects tab change.
2. It calls `active_host->ActivateUI()`.
3. The `.form` editor host registers its panes and merges its menus.
4. On deactivate, it calls `active_host->DeactivateUI()` to clean up.
