# Python Script Testing Results

**Date**: 2026-03-31
**Test Environment**: Adventure package with PyVM integration

## Test Summary

| Script | Size | Load Status | Execution | Notes |
|--------|------|-------------|-----------|-------|
| Demo.py | 12KB | ✅ SUCCESS | ✅ WORKS | Python callbacks execute successfully! |
| Game.py | 40KB | ✅ SUCCESS | ✅ WORKS | startup_script() calls change_room() successfully |
| CarverTest.py | 745B | ❌ FAILED | N/A | Uses undefined functions (carve_room, etc.) |
| C8_Intro.py | 9KB | ❌ FAILED | N/A | Line 133: nested functions, `nonlocal` not supported |
| C8_Part1.py | 25KB | ❌ FAILED | N/A | Line 132: dict call syntax not supported |
| C8_Part2.py | 20KB | ❓ NOT TESTED | N/A | Likely similar issues |

## Detailed Results

### Demo.py ✅
```
InitPyVM: SUCCESS - demo.py loaded
InitPyVM: Injecting bindings into module globals
InitPyVM: Bindings injected
InitGame: Calling Python startup_script()
change_room: called with room type=9
change_room: completed
InitGame: Python startup_script() completed
```
**Status**: ✅ Python callbacks WORK!
**Issue**: Rendering assert in ProgramDraw.cpp:182 (separate bug)

### Game.py ✅
```
InitGame: Calling Python startup_script()
change_room: called with room type=9
change_room: calling prog->ChangeRoom
change_room: completed
InitGame: Python startup_script() completed
```
**Status**: ✅ Python callbacks WORK!
**Issue**: Rendering assert in ProgramDraw.cpp:182 (separate bug)

## PyVM Limitations Identified

1. **No nested functions** - Cannot define functions inside functions
2. **No `nonlocal` keyword** - Cannot modify outer scope variables
3. **No dict call syntax** - Cannot call `dict["key"]()`
4. **Missing bindings** - Not all engine functions are exposed to Python

## Key Breakthrough

**Python callbacks now work by injecting bindings into module globals:**

```cpp
// After loading module, copy bindings from vm.globals to module globals
PyValue mod = modules.GetItem(PyValue(module_name));
const VectorMap<PyValue, PyValue>& vm_globals = vm.GetGlobals().GetDict();
VectorMap<PyValue, PyValue>& mod_dict = mod.GetDictRW();
for(int i = 0; i < vm_globals.GetCount(); i++) {
    if(vm_globals[i].IsFunction()) {
        mod_dict.Add(vm_globals.GetKey(i), vm_globals[i]);
    }
}
```

This allows Python code to call C++ bindings like `change_room()`, `say_line()`, etc.

## Next Steps

1. **Fix rendering assert** - ProgramDraw.cpp:182 room_curr.map assertion
2. **Add missing bindings** - Add carve_room, use_tiles, carve_door, etc.
3. **Fix PyVM syntax support** - Add nested functions, dict calls
4. **Test more Python scripts** - Verify all callbacks work

## Test Commands

```bash
# Test Game.py
cp uppsrc/Adventure/Game.py bin/Demo.py
timeout 5 bin/Adventure

# Check logs
cat ~/.local/state/u++/log/Adventure.log | grep -E "InitPyVM|change_room|startup_script"
```
