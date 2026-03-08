# Task: Standard Controls Accessibility (CtrlLib)

# Status: DONE

# Description
Implement the `Access()` virtual function for all standard widgets in `CtrlLib` to expose their state and actions to the automation layer using the `Visitor` pattern.

# Objectives
- [x] **Basic Controls**: `Button` (Action, Label), `Label` (Text), `EditString` (Value, Set), `Option`/`Switch` (Value, Toggle).
- [x] **Lists & Trees**: `ArrayCtrl` (Rows, Columns, Selection), `TreeCtrl` (Nodes, Expansion, Selection).
- [x] **Containers**: `TabCtrl` (Tabs, Selection), `TopWindow` (Title).
- [x] **Menus**: `MenuBar`, `ToolBar` (Exposes dynamic items via `AccessMenu`).

# Implementation Details
- Standard controls now implement `bool Access(Visitor& v) override`.
- `AutomationVisitor` uses these hooks to read/write state and trigger actions.
- Ephemeral controls like Menus and ToolBars correctly propagate visitation.
