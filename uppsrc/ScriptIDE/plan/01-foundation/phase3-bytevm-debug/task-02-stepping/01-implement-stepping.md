# Task: Implement Step Over/In/Out

## Goal
Add stepping modes to ByteVM for debugger control.

## Implementation

### Add to PyVM class:
```cpp
void StepOver();    // Execute next line, don't enter calls
void StepIn();      // Execute next line, enter calls
void StepOut();     // Run until current function returns

private:
    int step_frame_depth = 0;  // For step out tracking
```

### StepOver Implementation:
```cpp
void PyVM::StepOver()
{
    if(debug_state == DEBUG_PAUSED) {
        debug_state = DEBUG_STEP_OVER;
        step_frame_depth = frames.GetCount();
    }
}
```

### StepIn Implementation:
```cpp
void PyVM::StepIn()
{
    if(debug_state == DEBUG_PAUSED) {
        debug_state = DEBUG_STEP_IN;
    }
}
```

### StepOut Implementation:
```cpp
void PyVM::StepOut()
{
    if(debug_state == DEBUG_PAUSED) {
        debug_state = DEBUG_STEP_OUT;
        step_frame_depth = frames.GetCount() - 1;
    }
}
```

### Modify Step() logic:
```cpp
bool PyVM::Step()
{
    // ... existing code ...

    // Handle step out
    if(debug_state == DEBUG_STEP_OUT) {
        if(frames.GetCount() <= step_frame_depth) {
            debug_state = DEBUG_PAUSED;
        }
    }

    // Handle step over
    if(debug_state == DEBUG_STEP_OVER) {
        // Pause if we're at same or shallower depth
        if(frames.GetCount() <= step_frame_depth) {
            debug_state = DEBUG_PAUSED;
        }
    }

    // ... execute instruction ...
}
```

## Files Modified
- `uppsrc/ByteVM/PyVM.h`
- `uppsrc/ByteVM/PyVM.cpp`

## Testing
```cpp
// Test step over (should skip function call)
vm.StepOver();
while(vm.Step() && vm.GetDebugState() != PyVM::DEBUG_PAUSED);

// Test step in (should enter function)
vm.StepIn();
vm.Step();
ASSERT(vm.GetFramesCount() > initial_depth);

// Test step out
vm.StepOut();
while(vm.Step() && vm.GetDebugState() != PyVM::DEBUG_PAUSED);
ASSERT(vm.GetFramesCount() == initial_depth);
```

## Success Criteria
- Step over skips function calls
- Step in enters function calls
- Step out returns from current function
- Execution pauses correctly after each step
