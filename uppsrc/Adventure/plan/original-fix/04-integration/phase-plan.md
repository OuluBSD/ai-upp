# Phase 04: C++ Integration

## Overview

Update C++ code to use Python VM instead of ESC VM.

## Scope

- Modify `Program.cpp` to load Python files instead of ESC
- Replace `EscVM` calls with `PyVM` calls
- Update `Adventure.upp` to include Python files

## Tasks

1. [ ] **001-update-program-init** - Change `ctx.AddCodePath("Game.esc")` to `vm.LoadModule("game", src, path)`
2. [ ] **002-replace-esc-vm-calls** - Replace EscVM with PyVM throughout
3. [ ] **003-update-adventure-upp** - Add .py files to package manifest
4. [ ] **004-remove-esc-dependencies** - Remove EscAnim dependency if no longer needed
5. [ ] **005-update-build-script** - Ensure Python files are copied to build output

## Code Changes

### Program.cpp

**Before:**
```cpp
if (!ctx.AddCodePath(GetDataFile("Game.esc")))
    return false;
if (!ctx.Init(false))
    return false;
if (!AddEscFunctions())
    return false;
```

**After:**
```cpp
String game_py = LoadFile(GetDataFile("Game.py"));
if (!vm.LoadModule("game", game_py, GetDataFile("Game.py")))
    return false;
if (!AddPythonBindings(vm))
    return false;
```

### Adventure.upp

**Add:**
```
file
    ...
    Game.py,
    CarverTest.py,
    Demo.py,
    ...
```

## Expected Output

- Updated C++ files using PyVM
- Updated .upp manifest
- Working Python integration

## Acceptance Criteria

- [ ] C++ code compiles without errors
- [ ] Python files load correctly at runtime
- [ ] Python code can call C++ bindings
- [ ] C++ code can call Python functions

## Dependencies

- Phase 01 (bindings) complete
- Phase 02-03 (ESC conversion) complete

## Time Estimate

4-8 hours
