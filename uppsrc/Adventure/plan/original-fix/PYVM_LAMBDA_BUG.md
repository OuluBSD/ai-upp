# PyVM Lambda Compilation Bug

## Issue

PyVM (ByteVM Python implementation) fails to compile lambda expressions with error:
```
Compilation error [Game.py]: Line N: Expected statement end after expression, found assign
```

## Investigation Findings

### Lambda Parsing Code Location
- File: `uppsrc/ByteVM/PyCompiler.cpp`
- Lines: 1099-1118

### How Lambda Parsing Works

```cpp
else if(IsId("lambda")) {
    Next(); // consume 'lambda'
    Vector<String> largs;
    while(IsId() && !IsId("in") && !IsId("for") && !IsId("if")) {
        largs.Add(Peek().str_value);
        Next();
        if(IsToken(TK_COMMA)) Next(); else break;
    }
    this->Expect(TK_COLON);
    PyCompiler sub(tokens, file);
    sub.pos = pos;
    Vector<PyIR> body;
    sub.CompileLambdaBody(body);  // Compiles lambda body expression
    pos = sub.pos;  // Update position after lambda body
    // ... create function object
}
```

### CompileLambdaBody

```cpp
void PyCompiler::CompileLambdaBody(Vector<PyIR>& out)
{
    ir.Clear();
    Expression();        // Parse the lambda body expression
    Emit(PY_RETURN_VALUE);
    out = pick(ir);
}
```

### Root Cause Analysis

The issue appears to be in how `Expression()` terminates when parsing lambda bodies inside complex structures (dictionaries, multi-line conditions).

**Symptoms:**
1. Error reported at wrong line number (e.g., error at line 967 but actual issue earlier)
2. Error message: "Expected statement end after expression, found assign"
3. Segfault during compilation
4. Parser position gets out of sync after lambda body

**Suspected Issues:**

1. **Expression() doesn't stop at statement boundaries**: When parsing `lambda me: start_script(fn, True)`, the `Expression()` might consume tokens beyond the lambda body (like the comma after the lambda in a dictionary).

2. **Position tracking bug**: After `sub.CompileLambdaBody(body)` returns, `pos = sub.pos` should update the position, but the position might be pointing to the wrong token.

3. **Dictionary parsing interaction**: When a lambda appears as a dictionary value, the dictionary parsing code (lines 1276-1355) might not properly handle the lambda's end.

4. **Multi-line expression handling**: Complex multi-line conditions (like `if (a and b and c):`) might confuse the tokenizer or parser.

### Test Cases

**Fails:**
```python
# Lambda in dictionary
d = {
    "enter": lambda me: start_script(fn, True),
}

# Function after while loop with multi-line condition
def kitchen_tentacle_guard():
    while True:
        if (proximity(actor, door) < 40
            and not obj["alerting"]):
            do_something()
        break_time(10)

def next_function():  # Error reported here
    pass
```

**Works (standard Python):**
```python
# Same code compiles fine with python3 -m py_compile
```

### Workaround

Convert all lambda expressions to named functions:

**Before:**
```python
"enter": lambda me: start_script(fn, True),
```

**After:**
```python
def _enter_fn(me):
    start_script(fn, True)

# ... later:
"enter": _enter_fn,
```

This workaround was applied to `Game.py` (81 lambdas converted).

### Files Modified

- `uppsrc/ByteVM/PyCompiler.cpp` - Lambda parsing code (lines 1099-1118)
- `uppsrc/Adventure/Game.py` - Workaround: all lambdas converted to named functions

### Recommended Fix

1. **Add debug logging** to lambda parsing to trace position changes
2. **Create minimal test case** that reproduces the issue
3. **Check Expression() termination** - ensure it stops at appropriate boundaries
4. **Verify position tracking** - ensure `sub.pos` is correctly updated
5. **Test dictionary parsing** - verify lambdas in dict values work correctly

### Impact

- **Severity**: High - PyVM cannot compile valid Python lambda expressions
- **Scope**: Affects all Python code using lambdas
- **Workaround**: Convert lambdas to named functions (verbose but functional)

### Next Steps

1. Create comprehensive PyVM lambda test suite
2. Debug position tracking in lambda parsing
3. Fix Expression() termination logic
4. Add regression tests for lambda in various contexts

## References

- Python lambda semantics: `lambda args: expression` returns a function object
- PyVM should support: simple lambdas, lambdas in dicts, lambdas calling functions
- Token types: `TK_COLON`, `TK_COMMA`, `TK_BRACKET_BEGIN/END` (for `{}`)
