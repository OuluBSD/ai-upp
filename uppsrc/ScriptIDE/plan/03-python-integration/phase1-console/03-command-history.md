# Task: Implement Command History

## Goal
Implement UP/DOWN arrow key navigation through previous commands in the console.

## Implementation in PythonConsole.h

```cpp
class PythonConsole : public ParentCtrl {
    // ...
    virtual bool Key(dword key, int count) override;

private:
    Vector<String> history;
    int history_index = -1;
};
```

## Implementation in PythonConsole.cpp

```cpp
bool PythonConsole::Key(dword key, int count)
{
	if(input.HasFocus()) {
		if(key == K_UP) {
			if(history_index < history.GetCount() - 1) {
				history_index++;
				input.SetText(history[history.GetCount() - 1 - history_index]);
			}
			return true;
		}
		if(key == K_DOWN) {
			if(history_index > 0) {
				history_index--;
				input.SetText(history[history.GetCount() - 1 - history_index]);
			}
			else if(history_index == 0) {
				history_index = -1;
				input.Clear();
			}
			return true;
		}
	}
	return ParentCtrl::Key(key, count);
}

void PythonConsole::OnInput()
{
	String cmd = input.GetText().ToString();
	if(cmd.IsEmpty()) return;
	
	// Add to history
	if(history.IsEmpty() || history.Top() != cmd)
		history.Add(cmd);
	history_index = -1;
	
	last_input = cmd;
	Write(">>> " + cmd + "
");
	input.Clear();
	
	WhenInput();
}
```

## Success Criteria
- Pressing UP arrow in input field shows previous command
- Pressing DOWN arrow shows more recent commands
- New commands are added to history
- Duplicate consecutive commands are ignored
