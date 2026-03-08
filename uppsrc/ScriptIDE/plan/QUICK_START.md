# Quick Start Guide for Implementing ScriptIDE

## For AI Agents

1. **Read cookie.txt** to find current task
2. **Follow task markdown** in specified directory
3. **Update cookie.txt** when starting/completing tasks
4. **Build frequently** to catch errors early

## First Steps (Do These First!)

### 1. Read Key Documentation
- `IMPLEMENTATION_SUMMARY.md` - Architecture overview
- `01-foundation/README.md` - Foundation track details
- `cookie.txt` - Current progress

### 2. Start with Track 1 (Foundation)

**Critical:** Do ByteVM debugging FIRST before GUI work!

```bash
# Follow these tasks in order:
cd uppsrc/ScriptIDE/plan/01-foundation

# Phase 1: Package Setup
./phase1-package-setup/task-01-create-package/01-create-upp-file.md
./phase1-package-setup/task-01-create-package/02-create-headers.md
./phase1-package-setup/task-01-create-package/03-create-sources.md
./phase1-package-setup/task-01-create-package/04-create-layout-image-files.md

# Test build
script/build.py -mc 1 -j 12 ScriptIDE

# Phase 3: ByteVM Debugging (DO THIS BEFORE GUI!)
./phase3-bytevm-debug/task-01-breakpoints/01-add-breakpoint-storage.md
./phase3-bytevm-debug/task-01-breakpoints/02-integrate-breakpoint-checking.md
./phase3-bytevm-debug/task-02-stepping/01-implement-stepping.md
./phase3-bytevm-debug/task-03-callstack/01-expose-callstack.md

# Test ByteVM
# Create upptst/ByteVMDebug test
```

### 3. Continue with Track 2 (GUI)

```bash
cd uppsrc/ScriptIDE/plan/02-gui-components

# Phase 1: Main Window
./phase1-main-window/task-01-layout/01-create-main-layout.md

# Phase 2: Editor Area
./phase2-editor-area/task-01-custom-tabs/01-custom-filetabs.md

# Phase 5: StatusBar/Menus
./phase5-status-menu-toolbar/01-implement-menus.md
./phase5-status-menu-toolbar/02-implement-statusbar.md
```

### 4. Track 4: Variable Explorer

```bash
cd uppsrc/ScriptIDE/plan/04-features

./phase1-variable-explorer/01-create-variable-explorer.md
```

## Build and Run

```bash
# Build
script/build.py -mc 1 -j 12 ScriptIDE

# Run
./bin/ScriptIDE

# Or with debug output
./bin/ScriptIDE -v
```

## Common Issues

### 1. Missing Dependencies
**Error**: "Cannot find package ByteVM"
**Fix**: Verify uppsrc/ByteVM exists

### 2. Compilation Errors
**Error**: Undefined symbol
**Fix**: Check #include order in ScriptIDE.h

### 3. Layout Issues
**Error**: Controls not visible
**Fix**: Verify Add() and SizePos() calls

## Validation Checklist

After each phase:
- [ ] Code compiles without warnings
- [ ] Application runs without crashing
- [ ] New features are visible/functional
- [ ] No regressions in existing features

## Key Files Reference

### Package Files
- `uppsrc/ScriptIDE/ScriptIDE.upp` - Package manifest
- `uppsrc/ScriptIDE/ScriptIDE.h` - Main header
- `uppsrc/ScriptIDE/AGENTS.md` - Package documentation

### Main Components
- `PythonIDE.h/cpp` - Main window
- `CustomFileTabs.h/cpp` - Custom tab control
- `VariableExplorer.h/cpp` - Variable inspector
- `PythonConsole.h/cpp` - Console (Track 3)
- `FileTree.h/cpp` - File navigation

### ByteVM Modifications
- `uppsrc/ByteVM/PyVM.h` - Add debugging interface
- `uppsrc/ByteVM/PyVM.cpp` - Implement debugging
- `uppsrc/ByteVM/PyIR.h` - Add source location metadata

## Testing Python Scripts

Create test scripts in `uppsrc/ScriptIDE/examples/`:

```python
# test_basic.py
def factorial(n):
    if n <= 1:
        return 1
    return n * factorial(n - 1)

result = factorial(5)
print(f"5! = {result}")
```

Set breakpoint on line 3, test:
1. Run → pauses at breakpoint
2. Step Over → moves to line 4
3. Step In → enters recursive call
4. Variable Explorer shows `n`, `result`

## Next Steps After Foundation

1. Implement console (Track 3)
2. Add file tree (Track 4)
3. Implement help system
4. Add settings dialog
5. Polish UI/UX

## Getting Help

- Review `IMPLEMENTATION_SUMMARY.md` for architecture
- Check specific task .md files for detailed steps
- Look at TheIDE (`uppsrc/ide/`) for similar patterns
- Study TabBar (`uppsrc/TabBar/`) for custom tabs
- Read ByteVM (`uppsrc/ByteVM/`) for interpreter details

## Important Reminders

1. **ByteVM debugging MUST be done first**
2. **Build frequently** (after each subtask if possible)
3. **Update cookie.txt** to track progress
4. **Follow task order** - dependencies matter
5. **Test each phase** before moving on
6. **Use `script/build.py -mc 1 -j 12 ScriptIDE`** for builds
