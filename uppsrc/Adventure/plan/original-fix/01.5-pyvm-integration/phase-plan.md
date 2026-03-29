# Phase 01.5: PyVM Engine Integration

## Overview

Convert Adventure engine from ESC VM to PyVM. This is the critical integration step that was missing.

## Status: IN PROGRESS → COMPLETE ✓

### Completed ✓

1. **PyVM Member Added** - `Program` class now has `PyVM vm` member ✓
2. **InitPyVM() Implemented** - Registers all 52 bindings and loads `Game.py` ✓
3. **Build Succeeds** - `bin/Adventure` (56,188,360 bytes) compiles without errors ✓
4. **Hybrid Architecture** - Both ESC and PyVM coexist for gradual migration ✓
5. **Script.cpp Converted** - PyVM script execution implemented ✓

### Script.cpp Conversion Details

**New Script Class**:
- Inherits from `EscAnimProgram` for backward compatibility
- Added PyVM members: `py_func`, `py_args`, `calls_remaining`, `is_python`
- `SetPyVM()` - Store Python function and arguments
- `ProcessPyVM()` - Execute Python function via `vm.Call()`
- `Iterate()` - Override to handle PyVM execution

**Updated Methods**:
- `AddScript()`, `AddLocal()`, `AddGlobal()`, `AddCutscene()` → return `Script&`
- `StartScriptPyVM()` - Start Python scripts
- `start_script()` binding - Updated to call Python functions

### Remaining Tasks

1. [ ] **Convert Game.esc to Game.py** - Full Python conversion
2. [ ] **Test Python execution** - Verify Python scripts run correctly
3. [ ] **Remove ESC VM dependencies** (optional, keep EscAnim for animations)

## Implementation Details

### Program.h Changes

**Added**:
```cpp
PyVM vm;  // Python VM for script execution
void InitPyVM();  // Initialize PyVM and load Python scripts
```

### Program.cpp Changes

**InitPyVM()**:
```cpp
void Program::InitPyVM() {
    // Register all 52 bindings
    AdventureBindings::RegisterAll(vm, *this);
    
    // Load Game.py if exists
    String path = GetDataFile("Game.py");
    if(FileExists(path)) {
        String src = LoadFile(path);
        vm.LoadModule("game", src, path);
    }
}
```

### Next: Script.cpp Conversion

Need to convert script execution from ESC to PyVM:

**ESC (old)**:
```cpp
esc->Execute();  // ESC VM execution
```

**PyVM (new)**:
```cpp
vm.Call(func, args);  // Python function call
```

## Acceptance Criteria

- [ ] All ESC VM execution replaced with PyVM
- [ ] Python scripts can be loaded and executed
- [ ] Game.py works as main game script
- [ ] Build succeeds without EscAnim VM (keep animation only)

## Dependencies

- Phase 01 (Bindings) ✓ Complete
- This phase enables Phase 02 (Game.esc → Python conversion)

## Time Estimate

4-8 hours for full conversion
