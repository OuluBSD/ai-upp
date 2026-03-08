# Task: Integrate Breakpoint Checking into Execution

## Goal
Modify PyVM::Step() to check for breakpoints and pause execution.

## Changes Needed

### 1. Add execution state tracking to PyVM
In `uppsrc/ByteVM/PyVM.h`:
```cpp
class PyVM {
    // ... existing members ...

public:
    enum DebugState {
        DEBUG_RUNNING,      // Normal execution
        DEBUG_PAUSED,       // Paused at breakpoint or step
        DEBUG_STEP_OVER,    // Step over next instruction
        DEBUG_STEP_IN,      // Step into next instruction
        DEBUG_STEP_OUT      // Step out of current function
    };

    void Continue();        // Resume from pause
    void Pause();           // Pause execution
    DebugState GetDebugState() const { return debug_state; }

private:
    DebugState debug_state = DEBUG_RUNNING;
    String current_file;
    int current_line = 0;
};
```

### 2. Modify PyVM::Step()
Add breakpoint checking:
```cpp
bool PyVM::Step()
{
    if(frames.IsEmpty())
        return false;

    Frame& frame = TopFrame();

    // Get current location (file:line) from IR metadata
    if(frame.pc < frame.ir->GetCount()) {
        const PyIR& instr = (*frame.ir)[frame.pc];
        current_file = instr.file;  // Assuming PyIR has file metadata
        current_line = instr.line;  // Assuming PyIR has line metadata

        // Check for breakpoint
        if(debug_state == DEBUG_RUNNING && HasBreakpoint(current_file, current_line)) {
            if(CheckBreakpoint(current_file, current_line)) {
                debug_state = DEBUG_PAUSED;
                return true; // Paused at breakpoint
            }
        }

        // Handle step modes
        if(debug_state == DEBUG_STEP_OVER || debug_state == DEBUG_STEP_IN) {
            debug_state = DEBUG_PAUSED;
            // Step completed, pause
        }
    }

    // ... existing Step() implementation ...
}
```

### 3. Add file/line metadata to PyIR
In `uppsrc/ByteVM/PyIR.h`:
```cpp
struct PyIR : Moveable<PyIR> {
    // ... existing members ...

    String file;   // Source file
    int line;      // Source line number

    PyIR() : line(0) {}
};
```

### 4. Update PyCompiler to track source locations
Modify compiler to record file:line in PyIR during compilation.

## Files Modified
- `uppsrc/ByteVM/PyVM.h`
- `uppsrc/ByteVM/PyVM.cpp`
- `uppsrc/ByteVM/PyIR.h`
- `uppsrc/ByteVM/PyCompiler.cpp`

## Testing
```cpp
PyVM vm;
vm.AddBreakpoint("test.py", 5);

// Compile and run test.py
vm.SetIR(compiled_ir);

while(vm.Step()) {
    if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
        // Check we're at line 5
        ASSERT(vm.current_line == 5);
        break;
    }
}
```

## Success Criteria
- Execution pauses at breakpoints
- WhenBreakpointHit is called
- Current file:line is accessible
- Can continue after pause
