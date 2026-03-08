# Task: Create Layout and Image Files

## Goal
Create empty layout and image resource files.

## Files to Create

### 1. `uppsrc/ScriptIDE/ScriptIDE.lay`
Layout file with file tree panel:
```
LAYOUT(FileTreeLayout, 250, 400)
END_LAYOUT

LAYOUT(SettingsLayout, 600, 400)
END_LAYOUT
```

### 2. `uppsrc/ScriptIDE/ScriptIDE.iml`
Image list file with icons:
```
IMAGE_ID(IconPython)
IMAGE_ID(IconFile)
IMAGE_ID(IconFolder)
IMAGE_ID(IconRun)
IMAGE_ID(IconDebug)
IMAGE_ID(IconStop)
IMAGE_ID(IconStepOver)
IMAGE_ID(IconStepIn)
IMAGE_ID(IconStepOut)
IMAGE_ID(IconBreakpoint)
IMAGE_ID(IconNewFile)
IMAGE_ID(IconOpenFile)
IMAGE_ID(IconSave)
IMAGE_ID(IconSaveAs)
```

Note: Images will use CtrlImg stock icons initially, custom icons can be added later.

### 3. `uppsrc/ScriptIDE/AGENTS.md`
Package documentation:
```markdown
# ScriptIDE - Python IDE using ByteVM

## Overview
ScriptIDE is a Spyder-like Python IDE built on Ultimate++ framework using the internal ByteVM Python interpreter.

## Features
- Python code editor with syntax highlighting
- IPython-like console
- Variable explorer
- Integrated debugger (breakpoints, step over/in/out)
- File navigation
- Help browser
- Work area management

## Architecture
- **PythonIDE**: Main window managing layout and coordination
- **PythonConsole**: IPython-style console using ByteVM
- **VariableExplorer**: ArrayCtrl showing stack variables
- **FileTree**: TreeCtrl for file navigation
- **Settings**: Configuration management

## Dependencies
- ByteVM: Python interpreter with debugging support
- CodeEditor: Syntax highlighting for Python
- TabBar/FileTabs: File tab management
- RichEdit: Help system

## ByteVM Integration
Uses internal uppsrc/ByteVM Python interpreter ONLY. No external Python runtime required.
```

## Files Created
- `uppsrc/ScriptIDE/ScriptIDE.lay`
- `uppsrc/ScriptIDE/ScriptIDE.iml`
- `uppsrc/ScriptIDE/AGENTS.md`

## Testing
```bash
# Build should complete successfully
script/build.py -mc 1 -j 12 ScriptIDE
```

## Success Criteria
- Package builds
- No missing resource errors
- Window opens (empty for now)
