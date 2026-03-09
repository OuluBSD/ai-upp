# Task: Implement Run and Debug Menu Logic

## Goal
Wire the **Run** and **Debug** menus to the `RunManager` and `PyVM` services, replacing all stubs.

## Strategy
1.  **Run Logic**: Connect to `RunManager` for executing cells, selections, and full files.
2.  **Debug Logic**: Map toolbar/menu actions to `PyVM` stepping methods and breakpoint management.
3.  **Visual Feedback**: Ensure the editor and debugger pane refresh correctly upon state changes.

## Success Criteria
- [ ] F5 runs the entire file.
- [ ] F9 runs the current selection.
- [ ] Breakpoints can be toggled via F12.
- [ ] Stepping (In/Over/Out) works correctly when paused.
