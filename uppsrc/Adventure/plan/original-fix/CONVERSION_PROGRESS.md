# Esc → PyVM Conversion Progress

## Completed

### Phase 1: PyVM Core Fixes ✅
- Tokenizer: Comment handling produces TK_NEWLINE tokens
- Parser: Chained subscripts work (`obj["a"]["b"] = value`)
- Game.py loads successfully (40325 bytes, 1569 lines)

### Phase 2: Python Dict Conversion ✅
- PyToEscValue converts Python dicts to EscValue maps
- Room objects accessible from C++
- Basic data types convert correctly (str, int, float)

## Remaining Work

### Python Function Callbacks
**Problem**: Room enter/exit callbacks are Python functions, but Esc system expects EscValue lambdas.

**Current Status**: PyToEscValue returns empty EscValue for functions

**Solution Options**:

#### Option A: Python Function Wrapper (Recommended)
Create a wrapper that stores Python function references and calls them via PyVM:

```cpp
// In AdventureBindings.h
struct PyFunctionWrapper {
    PyValue func;
    PyValue module;  // Python module where function lives
    
    EscValue Call(const Vector<EscValue>& args);
};

// In PyToEscValue
if(pv.IsFunction()) {
    PyFunctionWrapper* wrapper = new PyFunctionWrapper();
    wrapper->func = pv;
    // Store in EscValue userdata
    return EscValue::UserData(wrapper);
}
```

#### Option B: Function Name Lookup
Store function names as strings, Esc looks up by name:
```python
# Python
room = {
    "enter": "game.enter_room",  # String reference
    "exit": "game.exit_room"
}
```

#### Option C: Hybrid Scripts
Keep room enter/exit as Esc lambdas, use Python for game logic only.

## Next Steps

1. Implement Option A (Python function wrapper)
2. Add StartScriptPyVM to call Python functions
3. Update Room.cpp to use Python callbacks
4. Test full room transition with Python enter/exit

## Files Modified

- `uppsrc/Core/TextParsing/Tokenizer.cpp` - Comment/newline handling
- `uppsrc/ByteVM/PyCompiler.cpp` - Chained subscripts
- `uppsrc/Adventure/AdventureBindings.cpp` - Dict conversion

## Build Status

```
script/build.py Adventure  # ✓ Success
bin/Adventure  # Loads Game.py, fails on room callbacks
```
