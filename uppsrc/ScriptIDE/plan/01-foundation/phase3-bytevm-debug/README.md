# Phase 3: ByteVM Debugging Support

## Goal
Enhance ByteVM to support IDE debugging features: breakpoints, stepping, call stack inspection, and variable access.

## Tasks

### Task 1: Breakpoint Support
Add breakpoint infrastructure to ByteVM:
- Breakpoint storage (file:line -> enabled/disabled)
- Breakpoint hit detection during execution
- Callback when breakpoint is hit

### Task 2: Stepping Support
Implement step over/in/out operations:
- Step over: Execute next instruction, skip function calls
- Step in: Enter function calls
- Step out: Run until current function returns
- Continue: Run until next breakpoint

### Task 3: Call Stack Tracking
Track execution frames for debugging:
- Expose current call stack
- Frame inspection (locals, line number, function name)
- Frame navigation for variable inspection

## Files Modified
- `uppsrc/ByteVM/PyVM.h` - Add debugging interface
- `uppsrc/ByteVM/PyVM.cpp` - Implement debugging logic
- `uppsrc/ByteVM/PyValue.h` - Expose value inspection
- `uppsrc/ByteVM/ByteVM.h` - Update package exports

## Dependencies
- Completed Phase 1 (package setup)
- Understanding of PyVM::Step() and frame management

## Testing
Create test in `upptst/ByteVMDebug`:
- Set breakpoints
- Step through code
- Inspect variables
- Verify call stack

## Success Criteria
- [ ] Can set/clear breakpoints by file:line
- [ ] Execution stops at breakpoints
- [ ] Step over/in/out works correctly
- [ ] Call stack is accessible
- [ ] Variables can be inspected at any frame
