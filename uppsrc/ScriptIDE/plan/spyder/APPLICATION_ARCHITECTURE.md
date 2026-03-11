# Application Architecture

## Migration Note (ScriptCommon Split, 2026-03-10)

Interpret this architecture with split ownership:
- Execution/analysis/runtime service logic belongs in `uppsrc/ScriptCommon`.
- ScriptIDE keeps GUI composition, controls, docking, and view adapters.
- ScriptCLI and MCP use the same ScriptCommon services without GUI dependencies.

This IDE follows a modular architecture composed of:

GUI Layer Execution Layer Analysis Layer Runtime Layer

    IDE
    ├── GUI Layer
    │   ├── Editor
    │   ├── Panes
    │   ├── Dialogs
    │   └── Toolbars
    │
    ├── Execution Layer
    │   ├── Run Manager
    │   ├── Debug Manager
    │   └── Profiler Manager
    │
    ├── Analysis Layer
    │   ├── Code Analysis
    │   ├── Linting
    │   └── Symbol Indexer
    │
    └── Runtime Layer
        └── ByteVM Python Frontend

------------------------------------------------------------------------

# Runtime

Python code executes inside:

ByteVM

Located in:

    uppsrc/ByteVM/

The runtime exposes APIs:

execute script inspect variables debug step profile execution

------------------------------------------------------------------------

# Communication

GUI communicates with runtime through:

event dispatch command queue

Example:

    Editor Run Button
            ↓
    RunManager
            ↓
    ByteVM.Execute()
            ↓
    VariableExplorer refresh
