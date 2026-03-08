# Task: Update UI on Breakpoint Hit

## Goal
Show the current execution line in the editor and refresh the variable explorer when a breakpoint is hit.

## Implementation Details

### Changes in PythonIDE.cpp

Update `OnBreakpointHit`:
```cpp
void PythonIDE::OnBreakpointHit(const String& file, int line)
{
	python_console.Write("Breakpoint hit at " + file + ":" + AsString(line) + "
");
	
	// 1. Highlight line in editor
	// TODO: Verify if file matches current editor
	code_editor.SetCursor(code_editor.GetPos(line - 1));
	code_editor.SetPtr(line - 1, CtrlImg::right_arrow(), 0);
	
	// 2. Refresh Variable Explorer
	UpdateVariableExplorer();
	
	// 3. Focus editor
	code_editor.SetFocus();
}
```

Also, need to clear the execution pointer when resuming:
```cpp
void PythonIDE::OnRun()
{
    code_editor.HidePtr();
    // ...
}

void PythonIDE::OnConsoleInput()
{
    code_editor.HidePtr();
    // ...
}
```

## Success Criteria
- When a breakpoint is hit, the editor jumps to that line and shows a blue arrow in the margin
- The Variable Explorer shows the local variables of the current frame
- The line/column in the status bar are updated
