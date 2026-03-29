# Task 002: Python Loading Test Results

## Test Setup
- Build: `script/build.py -mc 1 Adventure`
- Run: `timeout 2 bin/Adventure`

## Results

### InitPyVM() Logging
- InitPyVM started: ✓
- Bindings registered: ✓
- Game.py found: ✓ (at `/common/active/sblo/Dev/ai-upp/bin/Game.py`)
- Game.py loaded: ✗ (37492 bytes loaded, but compilation failed)

### Python Syntax
- py_compile: Exit code 0 (standard Python accepts the syntax)
- PyVM compilation: **FAILED** - PyVM has limited operator support

### Log Output
```
InitPyVM: Starting initialization
InitPyVM: Registered bindings
InitPyVM: Looking for Game.py at: /common/active/sblo/Dev/ai-upp/bin/Game.py
InitPyVM: Loaded 37492 bytes
InitPyVM: FAILED - LoadModule returned error
```

Runtime error message:
```
Compilation error in block: Line 783: Expected statement end after expression, found assign-subtracted
Compilation error [/common/active/sblo/Dev/ai-upp/bin/Game.py]: Line 783: Expected statement end after expression, found assign-subtracted
```

## Issues Found

### Critical Issue: PyVM Operator Support

**Location**: `Game.py:783`

**Problem**: The line `obj_spinning_top["x"] -= dir` uses the `-=` (assign-subtracted) operator, which the PyVM does not support.

**Error**: `Expected statement end after expression, found assign-subtracted`

**Root Cause**: The custom PyVM parser has limited operator support. While standard Python accepts compound assignment operators (`-=`, `+=`, etc.), the PyVM expects simple assignment only.

**Affected Code Pattern**:
```python
# Line 783 - NOT supported by PyVM
obj_spinning_top["x"] -= dir

# Line 785 - NOT supported by PyVM  
dir *= -1
```

**Required Fix**: Convert compound assignments to simple assignments:
```python
# PyVM-compatible version
obj_spinning_top["x"] = obj_spinning_top["x"] - dir
dir = dir * -1
```

### Secondary Issue: File Deployment

**Problem**: Game.py is not automatically copied to the `bin/` directory during build.

**Workaround**: Manual copy required:
```bash
cp uppsrc/Adventure/Game.py bin/Game.py
```

**Note**: The file is listed in `Adventure.upp`, but the U++ build system doesn't automatically deploy Python files to the executable directory.

## Status

- [ ] Game.py loads successfully - **PARTIAL**: File loads but has syntax errors
- [x] No Python syntax errors (standard Python) - **VERIFIED**: py_compile passes
- [x] PyVM initialized correctly - **VERIFIED**: InitPyVM() executes, bindings register
- [ ] PyVM compilation succeeds - **FAILED**: Operator support limitations

## Next Steps

1. **Fix PyVM syntax errors**: Convert all compound assignment operators (`-=`, `*=`, etc.) to simple assignments
2. **Investigate file deployment**: Determine why `.upp` file listing doesn't copy Game.py to bin/
3. **Full PyVM compatibility audit**: Scan Game.py for other PyVM-incompatible syntax

## Additional Notes

### Build Fix Required

During compilation, encountered X11 macro conflict with `PyValue::None()`. Fixed by undefining X11 macros in `PyValue.h`:

```cpp
// Undefine X11 macros that conflict with Python-style naming
#ifdef None
#undef None
#endif
#ifdef True
#undef True
#endif
#ifdef False
#undef False
#endif
```

**Files Modified**:
- `uppsrc/ByteVM/PyValue.h` - Added X11 macro guards
- `uppsrc/Adventure/Program.cpp` - Added InitPyVM() debug logging

### Test Execution Summary

| Step | Status | Details |
|------|--------|---------|
| Build Adventure | ✓ | Fixed X11 macro conflict |
| InitPyVM() logging | ✓ | All 4 log points hit |
| Game.py file found | ✓ | After manual copy to bin/ |
| Game.py bytes loaded | ✓ | 37492 bytes |
| PyVM LoadModule | ✗ | Syntax error at line 783 |
| Standard Python syntax | ✓ | py_compile exit code 0 |
