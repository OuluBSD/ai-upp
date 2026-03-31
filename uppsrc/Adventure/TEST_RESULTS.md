# Python Script Testing Results

**Date**: 2026-03-31
**Test Environment**: Adventure package with PyVM integration

## Test Summary

| Script | Size | Load Status | Execution | Notes |
|--------|------|-------------|-----------|-------|
| Demo.py | 12KB | ✅ SUCCESS | ⚠️ Partial | Loads successfully, "empty script" error |
| Game.py | 40KB | ✅ SUCCESS | ❌ Crash | Loads but segfaults (exit 139) |
| CarverTest.py | 745B | ❌ FAILED | N/A | Uses undefined functions (carve_room, etc.) |
| C8_Intro.py | 9KB | ❌ FAILED | N/A | Line 133: nested functions, `nonlocal` not supported |
| C8_Part1.py | 25KB | ❌ FAILED | N/A | Line 132: dict call syntax not supported |
| C8_Part2.py | 20KB | ❓ NOT TESTED | N/A | Likely similar issues |

## Detailed Results

### Demo.py ✅
```
InitPyVM: SUCCESS - demo.py loaded
error: empty script
```
**Status**: Loads successfully but shows "empty script" error
**Issue**: Script initialization may not be calling startup functions
**Exit Code**: 124 (timeout - ran for 5 seconds)

### Game.py ⚠️
```
InitPyVM: SUCCESS - demo.py loaded
error: empty script
[segfault]
```
**Status**: Loads but crashes during execution
**Issue**: Segfault (exit code 139) - likely in room initialization or callback execution
**Exit Code**: 139 (segmentation fault)

### CarverTest.py ❌
```
InitPyVM: FAILED - LoadModule returned error
```
**Status**: Failed to load
**Issue**: Uses undefined functions:
- `carve_room()`
- `split`
- `use_tiles()`
- `carve_door()`

These functions are not registered as Python bindings.

### C8_Intro.py ❌
```
Compilation error in block: Line 133: Expected statement end after expression, found id
```
**Status**: Failed to compile
**Issue**: Line 133 uses Python features not supported by PyVM:
- Nested function definitions (`def getval(bits):`)
- `nonlocal` keyword
- Complex closure patterns

### C8_Part1.py ❌
```
Compilation error in block: Line 132: Expected statement end after expression, found parenthesis-begin
```
**Status**: Failed to compile
**Issue**: Line 132 uses unsupported syntax:
```python
rm_ship_engine["scripts"]["check_engine"]()
```
PyVM doesn't support calling nested dictionary values as functions.

## PyVM Limitations Identified

1. **No nested functions** - Cannot define functions inside functions
2. **No `nonlocal` keyword** - Cannot modify outer scope variables
3. **No dict call syntax** - Cannot call `dict["key"]()`
4. **Limited closure support** - Complex closures may not work
5. **Missing bindings** - Not all engine functions are exposed to Python

## Recommendations

### High Priority
1. **Fix Game.py segfault** - Debug crash during execution
2. **Add missing bindings** - Add carve_room, use_tiles, carve_door, etc.
3. **Fix "empty script" error** - Ensure startup functions are called

### Medium Priority
4. **Support nested functions** - Extend PyVM parser
5. **Support dict call syntax** - Add `obj["key"]()` support
6. **Add `nonlocal` support** - Extend variable scoping

### Low Priority
7. **Convert C8_*.py** - Rewrite to avoid unsupported features
8. **Add more test scripts** - Create minimal test cases for each feature

## Next Steps

1. **Debug Game.py crash** - Add more logging to find segfault source
2. **Test individual callbacks** - Verify room enter/exit work
3. **Test verb execution** - Verify verb callbacks work
4. **Add missing bindings** - Add carve_* functions if needed

## Test Commands

```bash
# Test Demo.py
cp uppsrc/Adventure/Demo.py bin/Demo.py
timeout 5 bin/Adventure

# Test Game.py
cp uppsrc/Adventure/Game.py bin/Demo.py
timeout 5 bin/Adventure

# Check logs
cat ~/.local/state/u++/log/Adventure.log | grep -E "InitPyVM|error|SUCCESS"
```
