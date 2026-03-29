# Phase 05: Testing and Debugging

## Status: IN PROGRESS (1/5 tasks)

## Tasks

1. [x] **001-fix-graphics-crash** - Fix pre-existing graphics crash ✓
2. [ ] **002-test-python-loading** - Verify Game.py loads
3. [ ] **003-test-bindings** - Test Python bindings work
4. [ ] **004-compare-esc-python** - Verify behavior matches ESC
5. [ ] **005-document-api** - Write Python API docs

## Progress

### Task 001: Fix Graphics Crash ✓

**Issue**: SIGTRAP crash on startup (exit code 133)
**Fix**: Prevent painting before initialization complete
**Result**: ✅ Application runs (exit code 124)

**Files Modified**:
- App.cpp - Init sequence, Hide()/Show()
- Program.h - MarkInitialized() method
- ProgramDraw.cpp - Paint() null check
- Adventure.cpp - Call MarkInitialized()

See `001-fix-graphics-crash.md` for details.

### Remaining Tasks

**Task 002**: Test Python script loading
- Verify Game.py loads via InitPyVM()
- Check for Python syntax errors
- Confirm PyVM initialization works

**Task 003**: Test bindings
- Call each of 52 bindings
- Verify correct behavior
- Document any issues

**Task 004**: Compare ESC vs Python
- Run with ESC (rename Game.py)
- Run with Python
- Verify identical behavior

**Task 005**: Document API
- Write PYTHON_API.md
- Document all 52 functions
- Add usage examples
