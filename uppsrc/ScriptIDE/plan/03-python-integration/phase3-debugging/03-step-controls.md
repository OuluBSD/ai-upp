# Task: Implement Step Controls

## Goal
Implement Step Over, Step Into, and Step Out functionality in the IDE.

## Implementation Details

We already implemented the `OnStep*` stubs in `PythonIDE.cpp`. Now we need to make sure they work correctly by calling `Step()` repeatedly until the VM pauses again.

### Changes in PythonIDE.cpp

Update stepping actions:
```cpp
void PythonIDE::OnStepOver()
{
	if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
		vm.StepOver();
		while(vm.Step() && vm.GetDebugState() != PyVM::DEBUG_PAUSED);
		if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
			OnBreakpointHit(vm.GetCurrentFile(), vm.GetCurrentLine());
		}
	}
}

void PythonIDE::OnStepIn()
{
	if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
		vm.StepIn();
		while(vm.Step() && vm.GetDebugState() != PyVM::DEBUG_PAUSED);
		if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
			OnBreakpointHit(vm.GetCurrentFile(), vm.GetCurrentLine());
		}
	}
}

void PythonIDE::OnStepOut()
{
	if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
		vm.StepOut();
		while(vm.Step() && vm.GetDebugState() != PyVM::DEBUG_PAUSED);
		if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
			OnBreakpointHit(vm.GetCurrentFile(), vm.GetCurrentLine());
		}
	}
}
```

## Success Criteria
- When paused at a breakpoint, clicking "Step Over" (F10) moves to the next line in the same function
- Clicking "Step Into" (F11) enters a function call
- Clicking "Step Out" (Shift+F11) returns to the caller
- The UI (editor highlight and Variable Explorer) updates after each step
