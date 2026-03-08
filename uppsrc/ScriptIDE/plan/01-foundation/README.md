# Track 1: Foundation

Core infrastructure for ScriptIDE.

## Phases

### Phase 1: Package Setup
- Create ScriptIDE package structure
- Set up dependencies (ByteVM, CodeEditor, TabBar, etc.)
- Create initial headers and source files

### Phase 2: UI Foundation
- Create main window class (PythonIDE)
- Set up basic layout with Splitters
- Create placeholder panels

### Phase 3: ByteVM Debugging
- Add breakpoint support to ByteVM
- Add step over/in/out to ByteVM
- Add call stack tracking
- Add variable inspection interface

## Dependencies

- U++ Core, CtrlLib, CtrlCore
- CodeEditor (for Python syntax highlighting)
- TabBar/FileTabs (for tab management)
- ByteVM (Python interpreter)
- RichEdit (for Help system)

## Completion Criteria

- [x] Package compiles successfully
- [x] Main window opens and displays
- [x] ByteVM supports debugging operations
