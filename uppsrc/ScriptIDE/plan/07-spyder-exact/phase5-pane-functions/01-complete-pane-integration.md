# Task: Complete Pane Logic Integration

## Goal
Wire the remaining stubbed actions in all dockable panes to actual functional services in the IDE and VM.

## Strategy
1.  **Define Events**: Add necessary `Event<>` members to each pane class to expose their toolbar/menu actions.
2.  **IDE Wiring**: Connect these events in `PythonIDE` to appropriate methods (e.g., `OnPathManager`, `vm.Reset`).
3.  **Core Implementations**:
    -   **FilesPane**: Implement directory navigation history and system browser integration.
    -   **VariableExplorer**: Implement "Remove all variables" and refresh logic.
    -   **PlotsPane**: Implement Save All and basic Zoom logic in `ImageDisplay`.
    -   **PythonConsole**: Connect Kernel control actions (Interrupt, Restart).

## Implementation Details

### FilesPane
- `WhenPathManager`: Triggers `OnPathManager()`.
- `WhenBrowse`: Opens directory picker and calls `SetRoot()`.
- `WhenParent`: Moves root up one level.

### VariableExplorer
- `WhenRemoveAll`: Clears `vm.GetGlobals()` and refreshes display.
- `WhenRefresh`: Forces an update from current VM state.

### PlotsPane
- `WhenSaveAll`: Iterates through all images and saves them.
- `Zoom`: Update `ImageDisplay` scaling factor.

### PythonConsole
- `WhenRestartKernel`: Calls `vm.Reset()` and clears history.
- `WhenRemoveVariables`: Clears `vm.GetGlobals()`.

## Success Criteria
- [ ] PYTHONPATH button in Files Pane opens the manager dialog.
- [ ] "Remove all variables" in Variable Explorer actually clears the VM state.
- [ ] Browsing directory in Files Pane updates the tree root.
- [ ] Restart kernel in Console performs a full VM reset.
- [ ] No major stubs left in pane toolbars.
