#include "ScriptIDE.h"

namespace Upp {

PythonConsole::PythonConsole()
{
	Title("IPython Console");
	
	Add(toolbar.TopPos(0, 24).HSizePos());
	Add(output.VSizePos(24, 25).HSizePos());
	Add(input.BottomPos(0, 25).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
	
	output.SetReadOnly();
	output.SetFont(Courier(14));
	
	input.SetFont(Courier(14));
	input.WhenAction = [=] { OnInput(); };
}

void PythonConsole::LayoutToolbar(Bar& bar)
{
	bar.Add("Consoles", [=] {}).Help("Console list");
	bar.Gap(2000);
	bar.Add(CtrlImg::remove(), [=] { Clear(); }).Help("Clear console");
	bar.Add(CtrlImg::exclamation(), [=] {}).Help("Interrupt kernel");
	bar.Sub("Options", CtrlImg::plus(), [=](Bar& b) { LayoutPaneMenu(b); });
}

void PythonConsole::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Interrupt kernel", [=] {});
	bar.Add("Restart kernel", [=] {}).Key(K_CTRL|K_PERIOD);
	bar.Add("Remove all variables", [=] {}).Key(K_CTRL|K_ALT|K_R);
	bar.Add("Rename tab", [=] {});
	bar.Separator();
	bar.Add("Show environment variables", [=] {});
	bar.Add("Show sys.path contents", [=] {});
	bar.Add("Show elapsed time", [=] {}).Check(false);
}

void PythonConsole::Write(const String& s)
{
	output.Append(s.ToWString());
	output.ScrollEnd();
}

void PythonConsole::WriteError(const String& s)
{
	// TODO: use color
	output.Append(s.ToWString());
	output.ScrollEnd();
}

void PythonConsole::Clear()
{
	output.Clear();
}

bool PythonConsole::Key(dword key, int count)
{
	if(key == K_UP) {
		if(history_index < history.GetCount() - 1) {
			history_index++;
			input.SetData(history[history.GetCount() - 1 - history_index]);
		}
		return true;
	}
	if(key == K_DOWN) {
		if(history_index > 0) {
			history_index--;
			input.SetData(history[history.GetCount() - 1 - history_index]);
		}
		else if(history_index == 0) {
			history_index = -1;
			input.Clear();
		}
		return true;
	}
	return false;
}

void PythonConsole::OnInput()
{
	String cmd = input.GetData();
	if(cmd.IsEmpty()) return;

	if(history.IsEmpty() || history.Top() != cmd)
		history.Add(cmd);
	history_index = -1;

	last_input = cmd;
	Write(">>> " + cmd + "\n");
	input.Clear();

	WhenInput();
}

}
