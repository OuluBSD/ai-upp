# Task: Integrate Breakpoints with Editor

## Goal
Connect the CodeEditor breakpoints to the ByteVM interpreter.

## Implementation Details

We need to sync the breakpoints set in the `CodeEditor` with the `PyVM` instance.

### Changes in PythonIDE.cpp

Implement `OnToggleBreakpoint`:
```cpp
void PythonIDE::OnToggleBreakpoint()
{
	int line = code_editor.GetCursorLine();
	String filename = "<editor>"; // TODO: Use actual filename
	
	if(code_editor.IsBreakpoint(line)) {
		code_editor.SetBreakpoint(line, false);
		vm.RemoveBreakpoint(filename, line + 1);
	}
	else {
		code_editor.SetBreakpoint(line, true);
		vm.AddBreakpoint(filename, line + 1);
	}
}
```

Also, we need to handle the `WhenBreakpointHit` event from `PyVM`:
```cpp
PythonIDE::PythonIDE()
{
    // ...
    vm.WhenBreakpointHit = [=](const String& file, int line) { OnBreakpointHit(file, line); };
}

void PythonIDE::OnBreakpointHit(const String& file, int line)
{
	python_console.Write("Breakpoint hit at " + file + ":" + AsString(line) + "
");
	// TODO: Highlight current line in editor
	// TODO: Update Variable Explorer
	UpdateVariableExplorer();
}
```

## Success Criteria
- Setting a breakpoint in the editor (Ctrl+F9 or clicking margin) and running the script causes it to pause at that line
- The console shows a "Breakpoint hit" message
- The Variable Explorer updates with current local variables
