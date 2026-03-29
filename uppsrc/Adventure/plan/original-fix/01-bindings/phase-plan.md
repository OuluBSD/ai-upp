# Phase 01: Python Bindings for Engine Functions

## Overview

Create Python bindings for all engine functions that the ESC scripts use. These bindings allow Python code to call C++ engine functions.

## Scope

- Analyze ESC files to identify all engine function calls
- Create Python binding declarations in C++
- Test bindings with simple Python scripts

## Tasks

1. [ ] **001-analyze-esc-usage** - Catalog all engine function calls in ESC files
2. [ ] **002-create-binding-infrastructure** - Set up PyVM binding infrastructure
3. [ ] **003-bind-core-functions** - Bind core game functions (camera, room, actor)
4. [ ] **004-bind-ui-functions** - Bind UI functions (say_line, print_line, dialogs)
5. [ ] **005-bind-drawing-functions** - Bind drawing functions (map, sprites, effects)
6. [ ] **006-bind-game-logic-functions** - Bind game logic (cutscene, inventory, verbs)
7. [ ] **007-test-bindings** - Test all bindings with simple Python scripts

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
