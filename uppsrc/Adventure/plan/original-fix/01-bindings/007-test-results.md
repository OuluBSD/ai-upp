# Task 007: Test Results - Python Bindings Runtime Verification

## Test Environment
- **Build**: bin/Adventure (56,056,840 bytes)
- **Date**: 2026-03-29
- **Test script**: uppsrc/Adventure/test_bindings.py
- **Bindings implementation**: uppsrc/Adventure/AdventureBindings.cpp/h

## Executive Summary

**Status**: ⚠️ **PARTIAL** - Bindings implemented but NOT integrated with runtime

All 52 Python bindings have been implemented in C++ (Tasks 001-006 ✓). However, **runtime testing revealed that the bindings are not integrated** with the Adventure engine's execution model.

### Key Finding

The Adventure engine uses **ESC (Escape VM)** for scripting, not **PyVM (ByteVM)**. The bindings in `AdventureBindings.cpp` target PyVM but are never registered or called during engine execution.

## Results by Category

### Core Functions (12)
| Function | Status | Notes |
|----------|--------|-------|
| change_room | ✗ NOT INTEGRATED | Binding implemented, not registered |
| camera_follow | ✗ NOT INTEGRATED | Binding implemented, not registered |
| camera_at | ✗ NOT INTEGRATED | Binding implemented, not registered |
| camera_pan_to | ✗ NOT INTEGRATED | Binding implemented, not registered |
| put_at | ✗ NOT INTEGRATED | Binding implemented, not registered |
| walk_to | ✗ NOT INTEGRATED | Binding implemented, not registered |
| do_anim | ✗ NOT INTEGRATED | Binding implemented, not registered |
| get_room | ✗ NOT INTEGRATED | Binding implemented, not registered |
| get_actor | ✗ NOT INTEGRATED | Binding implemented, not registered |
| is_in_room | ✗ NOT INTEGRATED | Binding implemented, not registered |
| get_distance | ✗ NOT INTEGRATED | Binding implemented, not registered |
| face_direction | ✗ NOT INTEGRATED | Binding implemented, not registered |

### UI Functions (10)
| Function | Status | Notes |
|----------|--------|-------|
| say_line | ✗ NOT INTEGRATED | Binding implemented, not registered |
| print_line | ✗ NOT INTEGRATED | Binding implemented, not registered |
| dialog_set | ✗ NOT INTEGRATED | Binding implemented, not registered |
| dialog_start | ✗ NOT INTEGRATED | Binding implemented, not registered |
| dialog_clear | ✗ NOT INTEGRATED | Binding implemented, not registered |
| wait_for_camera | ✗ NOT INTEGRATED | Binding implemented, not registered |
| fades | ✗ NOT INTEGRATED | Binding implemented, not registered |
| clear_dialog | ✗ NOT INTEGRATED | Binding implemented, not registered |
| say_get | ✗ NOT INTEGRATED | Binding implemented, not registered |
| stop_talking | ✗ NOT INTEGRATED | Binding implemented, not registered |

### Game Logic Functions (15)
| Function | Status | Notes |
|----------|--------|-------|
| break_time | ✗ NOT INTEGRATED | Binding implemented, not registered |
| cutscene | ✗ NOT INTEGRATED | Binding implemented, not registered |
| pickup_obj | ✗ NOT INTEGRATED | Binding implemented, not registered |
| start_script | ✗ NOT INTEGRATED | Binding implemented, not registered |
| stop_script | ✗ NOT INTEGRATED | Binding implemented, not registered |
| is_script_running | ✗ NOT INTEGRATED | Binding implemented, not registered |
| verb_set | ✗ NOT INTEGRATED | Binding implemented, not registered |
| verb_get | ✗ NOT INTEGRATED | Binding implemented, not registered |
| inventory_get | ✗ NOT INTEGRATED | Binding implemented, not registered |
| has_obj | ✗ NOT INTEGRATED | Binding implemented, not registered |
| use_obj | ✗ NOT INTEGRATED | Binding implemented, not registered |
| set_selected_actor | ✗ NOT INTEGRATED | Binding implemented, not registered |
| get_selected_actor | ✗ NOT INTEGRATED | Binding implemented, not registered |
| open_door | ✗ NOT INTEGRATED | Binding implemented, not registered |
| close_door | ✗ NOT INTEGRATED | Binding implemented, not registered |

### Drawing Functions (9)
| Function | Status | Notes |
|----------|--------|-------|
| set_trans_col | ✗ NOT INTEGRATED | Binding implemented, not registered |
| map_draw | ✗ NOT INTEGRATED | Binding implemented, not registered |
| spr | ✗ NOT INTEGRATED | Binding implemented, not registered |
| sspr | ✗ NOT INTEGRATED | Binding implemented, not registered |
| rect | ✗ NOT INTEGRATED | Binding implemented, not registered |
| circle | ✗ NOT INTEGRATED | Binding implemented, not registered |
| line | ✗ NOT INTEGRATED | Binding implemented, not registered |
| pal | ✗ NOT INTEGRATED | Binding implemented, not registered |
| rectfill | ✗ NOT INTEGRATED | Binding implemented, not registered |

### Audio Functions (2)
| Function | Status | Notes |
|----------|--------|-------|
| sfx | ✗ NOT INTEGRATED | Binding implemented, not registered |
| music | ✗ NOT INTEGRATED | Binding implemented, not registered |

### Object Property Access (4)
| Function | Status | Notes |
|----------|--------|-------|
| obj_get_prop | ✗ NOT INTEGRATED | Binding implemented, not registered |
| obj_set_prop | ✗ NOT INTEGRATED | Binding implemented, not registered |
| obj_get_x | ✗ NOT INTEGRATED | Binding implemented, not registered |
| obj_set_x | ✗ NOT INTEGRATED | Binding implemented, not registered |

## Summary

- **Implemented**: 52/52 functions (100%) ✓
- **Integrated**: 0/52 functions (0%) ✗
- **Runtime tested**: 0/52 functions (0%) ✗

## Architecture Issue

### Current State

The Adventure engine uses **ESC (Escape VM)** for scripting:

```cpp
// In Program.cpp - ESC functions are registered
bool Program::AddEscFunctions() {
    auto& global = ctx.global;  // ESC context
    Escape(global, "change_room(new_room, fade)", THISBACK(EscChangeRoom));
    Escape(global, "camera_follow(actor)", THISBACK(EscCameraFollow));
    // ... more ESC bindings
}
```

### Implemented But Unused

The PyVM bindings exist but are never called:

```cpp
// In AdventureBindings.cpp - PyVM bindings (implemented but not registered)
void AdventureBindings::RegisterAll(PyVM& vm, Program& prog) {
    PyValue globals = vm.GetGlobals();
    globals.SetItem(PyValue("change_room"), PyValue::Function("change_room", change_room, &prog));
    // ... never called!
}
```

## Issues Found

### Issue 1: Wrong VM Target
- **Problem**: Bindings target PyVM, but engine uses ESC
- **Impact**: 100% of bindings are unreachable
- **Fix required**: Either:
  1. Convert engine to use PyVM instead of ESC, OR
  2. Create ESC bindings instead of PyVM bindings

### Issue 2: No Registration Point
- **Problem**: `AdventureBindings::RegisterAll()` is never called
- **Location**: `AdventureBindings.cpp:936`
- **Impact**: Even if PyVM were used, bindings wouldn't be registered

### Issue 3: Test Harness Build Failure
- **Problem**: Attempted to create test harness but encountered build issues
- **Details**: GUI vs CONSOLE app conflicts, missing includes
- **Files**: `TestBindings.cpp` created but not buildable as standalone

## Recommendations

### Option 1: Convert to PyVM (Recommended for Python-first approach)

**Steps**:
1. Replace ESC VM with PyVM in `Program::Init()`
2. Call `AdventureBindings::RegisterAll()` during initialization
3. Update script loading to use PyVM compiler instead of ESC
4. Migrate existing .esc scripts to .py or create Python equivalents

**Effort**: 8-16 hours
**Risk**: High - breaks existing ESC scripts

### Option 2: Create ESC Bindings (Recommended for backward compatibility)

**Steps**:
1. Convert each `AdventureBindings::function()` to `Program::EscFunction(EscEscape&)`
2. Register with `Escape(global, "name", THISBACK(EscFunction))`
3. Test with existing ESC scripts that call Python-like syntax

**Effort**: 16-24 hours (52 functions × 20-30 min each)
**Risk**: Medium - requires careful type conversion

### Option 3: Hybrid Approach (Recommended for gradual migration)

**Steps**:
1. Keep ESC for existing scripts
2. Add PyVM support as optional feature
3. Allow Python scripts to coexist with ESC
4. Migrate scripts gradually

**Effort**: 24-40 hours
**Risk**: Low - maintains compatibility

## Files Created/Modified

### Created
- `uppsrc/Adventure/TestBindings.cpp` - Test harness (not buildable)
- `uppsrc/Adventure/test_bindings.py` - Python test script (52 tests)

### Modified
- `uppsrc/Adventure/Adventure.upp` - Added TestBindings.cpp (not used)

### Not Used
- `AdventureBindings::RegisterAll()` - Implemented but never called

## Next Steps

To complete runtime testing:

1. **Decide on VM strategy** (ESC vs PyVM vs hybrid)
2. **Implement integration** based on chosen strategy
3. **Build test harness** that can run Python scripts
4. **Execute test script** and verify all 52 functions
5. **Update this document** with actual runtime results

## Conclusion

**Task 007 is BLOCKED** by architectural mismatch between implemented bindings (PyVM) and engine VM (ESC).

**What works**:
- ✓ All 52 binding functions are implemented in C++
- ✓ Type conversions (PyValue ↔ EscValue) are defined
- ✓ Test script exists with 52 test cases

**What doesn't work**:
- ✗ Bindings are never registered with any VM
- ✗ No way to call bindings from Python at runtime
- ✗ Test harness cannot be built due to GUI/CONSOLE conflicts

**To unblock**: Choose Option 1, 2, or 3 from Recommendations above.

---

**Phase 01 Status**: 6/7 tasks complete (86%)
- Task 007: BLOCKED - requires architectural decision
