# Task: Create ScriptIDE.upp Package File

## Goal
Create the package manifest file defining dependencies and included files.

## Steps

1. **Create `uppsrc/ScriptIDE/ScriptIDE.upp`**
   ```
   description "Python IDE using ByteVM\377";

   uses
       CtrlLib,
       CodeEditor,
       TabBar,
       ByteVM,
       RichEdit,
       Docking;

   file
       AGENTS.md,
       ScriptIDE.h,
       PythonIDE.h,
       PythonIDE.cpp,
       Settings.h,
       Settings.cpp,
       VariableExplorer.h,
       VariableExplorer.cpp,
       PythonConsole.h,
       PythonConsole.cpp,
       FileTree.h,
       FileTree.cpp,
       Main.cpp,
       ScriptIDE.lay,
       ScriptIDE.iml;

   mainconfig
       "" = "GUI";
   ```

2. **Verify dependency chain**:
   - ByteVM depends on Core only
   - CodeEditor depends on CtrlLib, RichEdit
   - TabBar depends on CtrlLib
   - All our dependencies are satisfied

## Files Created
- `uppsrc/ScriptIDE/ScriptIDE.upp`

## Testing
```bash
# List available configs
script/build.py --list-conf ScriptIDE
```

## Success Criteria
- Package file is recognized by build system
- No dependency errors
