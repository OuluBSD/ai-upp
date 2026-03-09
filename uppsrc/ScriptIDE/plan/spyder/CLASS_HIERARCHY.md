# Class Hierarchy (U++ Implementation)

This describes the main object model for the IDE.

    Ctrl
     └── DockableCtrl
          ├── FilesPane
          ├── VariableExplorerPane
          ├── DebuggerPane
          ├── ProfilerPane
          ├── PlotsPane
          ├── HelpPane
          ├── HistoryPane
          └── FindPane

Main window:

    DockWindow
     └── IDEWindow

Editor:

    Ctrl
     └── CodeEditor
          └── EditorTab

Dialogs:

    TopWindow
     ├── PythonPathManager
     ├── EnvironmentVariablesEditor
     └── RemoteConnectionDialog

Runtime integration:

    PythonRuntime
     ├── ByteVM
     ├── Interpreter
     ├── DebugEngine
     └── ProfilerEngine

Communication between UI and runtime is done through event callbacks.
