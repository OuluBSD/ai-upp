# Task: Expose Call Stack for Debugging

## Goal
Provide access to call stack frames and local variables for IDE inspection.

## Implementation

### Add StackFrame info struct to PyVM:
```cpp
struct StackFrame {
    String function_name;
    String file;
    int line;
    int frame_index;
    const VectorMap<PyValue, PyValue>* locals;  // Pointer to frame locals
};

// Get current call stack (deepest first)
Vector<StackFrame> GetCallStack() const;

// Get locals at specific frame
const VectorMap<PyValue, PyValue>& GetLocals(int frame_index) const;

// Get current frame index
int GetCurrentFrameIndex() const { return frames.GetCount() - 1; }
```

### Implementation:
```cpp
Vector<PyVM::StackFrame> PyVM::GetCallStack() const
{
    Vector<StackFrame> stack;

    for(int i = frames.GetCount() - 1; i >= 0; i--) {
        const Frame& f = frames[i];
        StackFrame sf;
        sf.frame_index = i;
        sf.locals = &f.locals;

        // Extract function name from PyValue
        if(f.func.GetType() == PY_FUNCTION) {
            // Get function name from PyLambda
            sf.function_name = "<function>";  // TODO: Store name in PyLambda
        }

        // Get current file:line from IR
        if(f.pc < f.ir->GetCount()) {
            sf.file = (*f.ir)[f.pc].file;
            sf.line = (*f.ir)[f.pc].line;
        }

        stack.Add(sf);
    }

    return stack;
}

const VectorMap<PyValue, PyValue>& PyVM::GetLocals(int frame_index) const
{
    ASSERT(frame_index >= 0 && frame_index < frames.GetCount());
    return frames[frame_index].locals;
}
```

### Add function name to PyLambda
In `uppsrc/ByteVM/PyValue.h`:
```cpp
struct PyLambda : PyValue::RefCount {
    String name;  // Function name
    Vector<String> params;
    Vector<PyIR> code;
};
```

## Files Modified
- `uppsrc/ByteVM/PyVM.h`
- `uppsrc/ByteVM/PyVM.cpp`
- `uppsrc/ByteVM/PyValue.h`

## Testing
```cpp
// After pausing at breakpoint
Vector<PyVM::StackFrame> stack = vm.GetCallStack();
ASSERT(stack.GetCount() > 0);

// Check top frame
ASSERT(stack[0].line == expected_line);

// Inspect locals
const auto& locals = vm.GetLocals(stack[0].frame_index);
// Verify variables exist
```

## Success Criteria
- Call stack is accessible
- Each frame shows file:line
- Locals can be accessed at any frame
- Function names are shown (when available)
