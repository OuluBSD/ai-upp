#include "ScriptIDE.h"

namespace Upp {

PythonConsole::PythonConsole()
{
	output.SetReadOnly();
	output.SetFont(Courier(14));
	
	input.SetFont(Courier(14));
	input.WhenAction = [=] { OnInput(); };
	
	Add(output.VSizePos(0, 30));
	Add(input.BottomPos(0, 30).HSizePos());
}

void PythonConsole::Write(const String& s)
{
	output.Append(s.ToWString());
	output.ScrollEnd();
}

void PythonConsole::WriteError(const String& s)
{
	Write(s);
}

void PythonConsole::Clear()
{
	output.Clear();
}

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
	Write(">>> " + cmd + "\n");
	input.Clear();

	WhenInput();
}

}
