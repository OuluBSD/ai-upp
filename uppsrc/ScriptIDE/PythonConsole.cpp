#include "ScriptIDE.h"

namespace Upp {

PythonConsole::PythonConsole()
{
	Title("IPython Console");
	Icon(CtrlImg::exclamation());
	
	Add(toolbar.TopPos(0, 24).HSizePos());
	Add(output.VSizePos(24, 25).HSizePos());
	Add(input.BottomPos(0, 25).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
	
	output.SetReadOnly();
	output.Highlight("python");
	
	input.WhenAction = [=] { OnInput(); };
}

void PythonConsole::LayoutToolbar(Bar& bar)
{
	bar.Add("Console list", CtrlImg::plus(), [=] { Todo("Console list"); }).Help("Console list");
	bar.Add("Console tabs", [=] { Todo("Console tabs"); });
	bar.Gap(2000);
	bar.Add("Clear console", CtrlImg::remove(), [=] { Clear(); }).Help("Clear console (Ctrl+L)");
	bar.Add("Interrupt kernel", CtrlImg::remove(), WhenInterrupt).Help("Interrupt kernel");
	bar.Sub("Options", CtrlImg::plus(), [=](Bar& b) { LayoutPaneMenu(b); });
}

void PythonConsole::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Interrupt kernel", WhenInterrupt);
	bar.Add("Restart kernel", WhenRestart).Key(K_CTRL_PERIOD);
	bar.Add("Remove all variables", WhenRemoveVariables).Key(K_CTRL|K_ALT|K_R);
	bar.Add("Rename tab...", [=] { Todo("Rename tab"); });
	bar.Separator();
	bar.Add("Show environment variables", [=] { Todo("Env vars"); });
	bar.Add("Show sys.path contents", [=] { Todo("sys.path"); });
	bar.Add("Show elapsed time", [=] {}).Check(false);
	bar.Separator();
	bar.Add("Switch to next console", [=] { Todo("Next console"); }).Key(K_ALT|K_SHIFT|K_RIGHT);
	bar.Add("Switch to previous console", [=] { Todo("Prev console"); }).Key(K_ALT|K_SHIFT|K_LEFT);
	bar.Separator();
	bar.Add("Move", [=] { Todo("Move pane"); });
	bar.Add("Undock", [=] { Todo("Undock pane"); });
	bar.Add("Close", [=] { Todo("Close pane"); });
}

void PythonConsole::Write(const String& s)
{
	output.SetCursor(output.GetLength());
	output.Insert(output.GetLength(), s);
	output.SetCursor(output.GetLength());
}

void PythonConsole::WriteError(const String& s)
{
	// For now just plain write
	Write(s);
}

void PythonConsole::Clear()
{
	output.Clear();
}

void PythonConsole::OnInput()
{
	last_input = ~input;
	if(!last_input.IsEmpty()) {
		history.Add(last_input);
		history_index = history.GetCount();
		Write(">>> " + last_input + "\n");
		WhenInput();
		input.Clear();
	}
}

bool PythonConsole::Key(dword key, int count)
{
	if(key == K_UP) {
		if(history_index > 0) {
			history_index--;
			input.SetData(history[history_index]);
		}
		return true;
	}
	if(key == K_DOWN) {
		if(history_index < history.GetCount() - 1) {
			history_index++;
			input.SetData(history[history_index]);
		} else {
			history_index = history.GetCount();
			input.Clear();
		}
		return true;
	}
	if(key == (K_CTRL|K_L)) {
		Clear();
		return true;
	}
	return DockableCtrl::Key(key, count);
}

}
