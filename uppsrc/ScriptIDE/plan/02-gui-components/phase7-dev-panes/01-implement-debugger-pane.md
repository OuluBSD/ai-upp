# Task: Implement Debugger Pane

## Goal
Implement the `DebuggerPane` to provide visual control and inspection of the Python debugging session, fulfilling the specification in `spyder/GUI.md`.

## U++ Widget Mapping
- **Main Container**: `DockableCtrl`.
- **Controls**: `ToolBar` for debugger actions.
- **Stack Trace**: `TreeCtrl` to display call stack frames.

## Interface Definition (DebuggerPane.h)

```cpp
class DebuggerPane : public DockableCtrl {
public:
	typedef DebuggerPane CLASSNAME;
	DebuggerPane();

	void SetStack(const Vector<PyVM::StackFrame>& stack);
	void Clear();

	Event<> WhenStepOver;
	Event<> WhenStepInto;
	Event<> WhenStepOut;
	Event<> WhenContinue;
	Event<> WhenStop;
	Event<int> WhenFrameSelected;

private:
	ToolBar toolbar;
	TreeCtrl stack_tree;
	
	void LayoutToolbar(Bar& bar);
	void OnTreeCursor();
};
```

## Implementation Steps

### 1. UI Layout
- Initialize `Title("Debugger")` and `Icon(CtrlImg::exclamation())`.
- Add `toolbar` at the top (`TopPos(0, 24)`).
- Add `stack_tree` below (`VSizePos(24, 0)`).

### 2. Toolbar Actions
- Add buttons: Continue, Step Over, Step Into, Step Out, Stop.
- Connect buttons to the corresponding `When...` events.

### 3. Stack Display
- `SetStack` should clear the tree and add nodes for each `StackFrame`.
- Label format: `function_name (file:line)`.
- Selecting a frame should trigger `WhenFrameSelected` with the frame index.

### 4. Integration with PythonIDE
- Register `DebuggerPane` in `PythonIDE::DockInit`.
- Update `PythonIDE::OnBreakpointHit` to refresh the stack tree.
- Connect debugger toolbar events to `PythonIDE` debugging methods.

## Success Criteria
- [ ] Toolbar shows all debugging controls.
- [ ] Call stack is visible when paused.
- [ ] Clicking a frame in the tree notifies the IDE.
- [ ] No stubs: all buttons must trigger actual VM actions.
