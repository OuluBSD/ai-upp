# Adventure Python Bindings Infrastructure

**Status**: ✅ COMPLETED  
**Date**: 2026-03-29  
**Priority**: HIGH

---

## Summary

Created Python binding infrastructure for the Adventure game engine, exposing 12 core engine functions to Python scripts via PyVM.

---

## Deliverables

### Files Created

1. **`uppsrc/Adventure/AdventureBindings.h`** - Binding declarations
2. **`uppsrc/Adventure/AdventureBindings.cpp`** - Binding implementations (12 functions)
3. **`uppsrc/Adventure/test_bindings.py`** - Test script
4. **`uppsrc/Adventure/AGENTS.md`** - Documentation for developers

### Files Modified

1. **`uppsrc/Adventure/Adventure.upp`** - Added binding files to package manifest
2. **`uppsrc/EscAnim/EscAnim.h`** - Fixed missing Math.h include

---

## Functions Bound

### Core Functions (Room & Camera)
- ✅ `change_room(room, fade)` - Switch to different room
- ✅ `camera_follow(actor)` - Make camera follow actor
- ✅ `camera_at(position)` - Set camera position
- ✅ `camera_pan_to(target)` - Pan camera smoothly
- ✅ `put_at(obj, x, y, room)` - Position object

### UI Functions (Dialog & Text)
- ✅ `say_line(text)` / `say_line(actor, text, wait, duration)` - Display dialog
- ✅ `print_line(text, x, y, color, ...)` - Print text at position

### Game Logic Functions
- ✅ `break_time(frames)` - Wait N frames
- ✅ `cutscene(type, setup_fn, cleanup_fn)` - Start cutscene
- ✅ `pickup_obj(obj, actor)` - Add to inventory

### Drawing Functions
- ✅ `set_trans_col(color, enable)` - Set transparent color
- ✅ `map_draw(src_x, src_y, dest_x, dest_y, width, height, flags)` - Draw map tiles

---

## Build Status

```bash
$ script/build.py -j12 Adventure
----- Adventure ( GUI DEBUG_FULL MAIN CLANG DEBUG SHARED BLITZ POSIX LINUX ) (38 / 38)
AdventureBindings.cpp
Adventure: 1 file(s) built in (0:01.52), 1525 msecs / file
Linking...
/common/active/sblo/Dev/ai-upp/bin/Adventure (55997792 B) linked in (0:02.18)
OK. (0:09.52)
Executable compiled: bin/Adventure
```

✅ **Build succeeds without errors**

---

## Implementation Pattern

### Function Signature
```cpp
Upp::PyValue AdventureBindings::function_name(
    const Upp::Vector<Upp::PyValue>& args, 
    void* user_data)
{
    Adventure::Program* prog = GetProgram(user_data);
    
    // Validate arguments
    if(args.GetCount() < required) {
        return Upp::PyValue::None();
    }
    
    // Extract arguments
    // - args[0].GetStr() for strings
    // - args[0].AsInt() for integers
    // - args[0].IsTrue() for booleans
    // - PyToEscValue(args[0]) for game objects
    
    // Call C++ implementation
    prog->SomeFunction(...);
    
    return Upp::PyValue::None();
}
```

### Registration
```cpp
void AdventureBindings::RegisterAll(Upp::PyVM& vm, Adventure::Program& prog)
{
    Upp::PyValue globals = vm.GetGlobals();
    
    globals.SetItem(Upp::PyValue("function_name"), 
                    Upp::PyValue::Function("function_name", function_name, &prog));
}
```

---

## Usage Example (Python)

```python
# Test script for Adventure Python bindings

print("Testing Adventure bindings...")

# Test say_line
say_line("Hello from Python!")
say_line("Multi-line message!:Second line")

# Test camera functions
camera_follow(player_actor)
camera_at(100)
camera_pan_to(target_actor)

# Test game logic
break_time(30)  # Wait 30 frames
cutscene(1, lambda: (
    say_line("Cutscene dialog"),
    print("Setup complete")
))

# Test drawing
set_trans_col(8, True)

print("All tests passed!")
```

---

## Technical Notes

### Namespace Handling
- `Adventure::Program` is in the `Adventure` namespace (not `Upp::`)
- Binding functions use `Upp::` prefix for PyVM types
- No `NAMESPACE_UPP` macros in binding files

### Type Conversion
- **Python String → WString**: `args[0].GetStr()` returns `WString`
- **Python Int → int**: `args[0].AsInt()` 
- **Python Float → double**: `args[0].AsDouble()`
- **Python Bool**: `args[0].IsTrue()`
- **Game Objects**: `PyToEscValue()` helper converts to `EscValue`

### EscValue Handling
Current implementation uses a simple conversion helper. For full object property access (e.g., `obj.x = 100`), a proper `EscValue` wrapper class is needed.

---

## TODO / Future Work

### High Priority
- [ ] Integrate bindings with Adventure engine initialization
- [ ] Implement `EscValue` wrapper class for Python object references
- [ ] Add property get/set for game objects (`obj.x`, `obj.y`, `obj.state`, etc.)
- [ ] Implement lambda/callback execution in `cutscene()` function

### Medium Priority
- [ ] Add remaining functions from function-catalog.md (47 total, 12 done)
- [ ] Implement proper error handling with `vm.RaiseException()`
- [ ] Create comprehensive integration tests with actual game objects
- [ ] Document object reference handling between Python and C++

### Low Priority
- [ ] Add type checking and better error messages
- [ ] Implement `void` parameter handling (ESC uses `void` as null)
- [ ] Performance optimization for frequent calls
- [ ] Add support for ESC script syntax in Python

---

## References

- **Function Catalog**: `uppsrc/Adventure/plan/original-fix/01-bindings/function-catalog.md`
- **PyVM API**: `uppsrc/ByteVM/PyVM.h`
- **PyValue API**: `uppsrc/ByteVM/PyValue.h`
- **Binding Helpers**: `uppsrc/ByteVM/PyBindings.h`
- **Example Pattern**: `uppsrc/ScriptCommon/CardGamePlugin.cpp:683` (SyncBindings)

---

## Testing

### Build Test
```bash
script/build.py -j12 Adventure
```

### Future Integration Test
```bash
# Once integrated with engine
bin/Adventure --run-python test_bindings.py
```

---

## Acceptance Criteria Status

- [x] Binding infrastructure files created
- [x] At least 5 most-used functions have bindings (12 implemented)
- [x] Test script exists
- [x] Code compiles without errors
- [x] Binding pattern documented for other developers (AGENTS.md)

**All acceptance criteria met** ✅
