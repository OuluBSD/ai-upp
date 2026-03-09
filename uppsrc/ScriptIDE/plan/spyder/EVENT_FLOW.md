# Event Flow

## Running Code

User presses Run button

    Toolbar.Run
          ↓
    RunManager
          ↓
    ByteVM.ExecuteScript()
          ↓
    Execution finished
          ↓
    VariableExplorer refresh
    PlotsPane update

------------------------------------------------------------------------

## Debugging

    Breakpoint hit
          ↓
    DebuggerPane update
          ↓
    User presses Step
          ↓
    ByteVM.Step()
          ↓
    UI refresh

------------------------------------------------------------------------

## Variable Explorer

    Runtime variable change
          ↓
    VariableInspector API
          ↓
    VariableExplorerPane.Update()

------------------------------------------------------------------------

## File Search

    User search query
          ↓
    SearchEngine
          ↓
    Filesystem scan
          ↓
    Results tree update
