# Phase 01: Python Bindings for Engine Functions

## Overview

Create Python bindings for all engine functions that the ESC scripts use. These bindings allow Python code to call C++ engine functions.

## Scope

- Analyze ESC files to identify all engine function calls
- Create Python binding declarations in C++
- Test bindings with simple Python scripts

## Tasks

1. [x] **001-analyze-esc-usage** - Catalog all engine function calls in ESC files ✓
2. [x] **002-create-binding-infrastructure** - Set up PyVM binding infrastructure ✓
3. [x] **003-bind-core-functions** - Bind core game functions (camera, room, actor) ✓
4. [x] **004-bind-ui-functions** - Bind UI functions (say_line, print_line, dialogs) ✓
5. [x] **005-bind-drawing-functions** - Bind drawing functions (map, sprites, effects) ✓
6. [x] **006-bind-game-logic-functions** - Bind game logic (cutscene, inventory, verbs) ✓
7. [ ] **007-test-bindings** - Test all bindings with simple Python scripts

## Progress

**Completed**: 6/7 tasks (86%) - **ALL BINDINGS IMPLEMENTED** ✓

### Task 001: Analyze ESC Usage ✓
- Created `function-catalog.md` with 47 unique functions
- Analyzed 6 ESC files (~6000 lines)
- Documented 10 tricky patterns

### Task 002: Create Binding Infrastructure ✓
- Created `AdventureBindings.h/cpp` (354 lines initial)
- Implemented 12 functions (Core: 5, UI: 2, Game Logic: 3, Drawing: 2)
- Created `test_bindings.py` and `AGENTS.md`
- Build succeeds: `bin/Adventure` (55,997,792 bytes)

### Tasks 003-006: Implement Remaining Bindings ✓
- **Total functions**: 52 (exceeded original 47 estimate!)
- **Categories**:
  * Core (12): change_room, camera_follow, walk_to, do_anim, etc.
  * UI (10): say_line, print_line, dialog_*, fades, etc.
  * Game Logic (15): cutscene, pickup_obj, inventory_*, verb_*, etc.
  * Drawing (9): map_draw, spr, sspr, rect, circle, line, etc.
  * Audio (2): sfx, music
  * Object Properties (4): get/set x, y, state, name
- **Build**: ✓ Successful - `bin/Adventure` (56,056,840 bytes)
- **Test script**: 52 test cases covering all functions

### Remaining Task

**Task 007**: Integration testing - Run test_bindings.py with Adventure engine

## Updated Effort

- **Analysis**: 2-4 hours ✓
- **Implementation**: 16-24 hours (52 functions) ✓
- **Testing**: 4-8 hours (pending)
- **Total**: 3-4 days (on track)

## Estimated Effort

- **Analysis**: 2-4 hours
- **Implementation**: 8-16 hours
- **Testing**: 4-8 hours
- **Total**: 2-3 days

## Deliverables

1. `AdventureBindings.cpp/h` - C++ binding implementations
2. `test_bindings.py` - Python test script for bindings
3. Updated `Adventure.upp` - Include binding files

## Acceptance Criteria

- [ ] All engine functions used by ESC scripts have Python bindings
- [ ] Python test script can call all bound functions without crashes
- [ ] Bindings properly handle Python → C++ type conversions
- [ ] Error handling works correctly (Python exceptions from C++ errors)

## Dependencies

- None (this is the first phase)

## Notes

- Reference existing Python bindings in `uppsrc/ScriptCommon/CardGamePlugin.cpp`
- Use `PyVM::GetGlobals().SetItem()` to expose functions to Python
- Consider using a binding generator if manual bindings are too tedious
