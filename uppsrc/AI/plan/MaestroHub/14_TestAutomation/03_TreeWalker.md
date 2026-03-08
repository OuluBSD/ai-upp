# Task: Heuristic Tree Walker

# Status: DONE

# Description
Implement the logic to traverse the `Ctrl` hierarchy and produce a sorted, linear representation of the UI for automation scripts.

# Objectives
- [x] Create `Walk(Ctrl& c)` method in `GuiAutomationVisitor`.
- [x] Implement recursive traversal: `Walk` visits children recursively.
- [x] Implement **Geometric Sorting**:
    - Collect visible children of `c`.
    - Sort them by `Top` coordinate, then `Left`.
    - Ensures UI elements are processed in reading order.
- [x] Filter invisible controls: `IsVisible()` check is integrated.
- [x] Update `Read` and `Write` workflows to use the walker for exhaustive UI exploration.

# Implementation Details
- `GuiAutomationVisitor` now performs a deep walk of the Ctrl tree.
- `CtrlGeometry` helper structure handles the sorting logic.
- Path construction correctly handles container transitions.