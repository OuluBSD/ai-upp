# Task: Main Layout Refactor (WinXP Frame)
# Status: DONE

## Objective
Rebuild the MaestroHub main window structure to support the three-pane horizontal layout and bottom panel.

## Requirements
- Replace the root `TabCtrl` with a horizontal `Splitter`.
- Pane 1 (Left): `TabCtrl` for "Workspace" and "Pipeline".
- Pane 2 (Center): `TabCtrl` for primary functional hubs (Fleet, Intelligence, Designer, Execution, Issues).
- Pane 3 (Right): `ParentCtrl` for the "Global AI Assistant".
- Bottom: `Splitter` (Vertical) to house the "Output/Trace" tabbed pane.
- Add standard `ToolBar` below the `MenuBar`.
- Add multi-pane `StatusBar`.
