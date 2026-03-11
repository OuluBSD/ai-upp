#include "ScriptIDE.h"

namespace Upp {

PythonConsole::PythonConsole()
{
    Title("IPython Console");
    Icon(CtrlImg::help()); // Placeholder
    
    Add(toolbar.TopPos(0, 24).HSizePos());
    Add(output.VSizePos(24, 24).HSizePos());
    Add(input.BottomPos(0, 24).HSizePos());
    
    toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
    
    output.SetReadOnly();
    
    input.WhenAction = [=] { OnInput(); };
}

void PythonConsole::LayoutToolbar(Bar& bar)
{
	bar.Add("Console 1/A", [=] {}).Enable(false); // Display current console name
	bar.Gap(2000);
	bar.Add(CtrlImg::remove(), [=] { Clear(); }).Help("Clear console");
	bar.Add(ScriptIDEImg::IconStop(), WhenInterrupt).Help("Interrupt kernel");
	bar.Sub("Options", CtrlImg::plus(), [=](Bar& b) { LayoutPaneMenu(b); });
}

void PythonConsole::LayoutPaneMenu(Bar& bar)
{
	bar.Sub("Completion", [=](Bar& b) {
		b.Add("Show completion on-the-fly", [=] {}).Check(true);
		b.Add("Show source code introspection", [=] {}).Check(true);
	});
	bar.Separator();
	bar.Add("Show timing", [=] {}).Check(false);
	bar.Add("Show calltips", [=] {}).Check(true);
	bar.Add("Buffer: 500 lines", [=] { Todo("Buffer size"); });
	bar.Separator();
	bar.Add("Restart kernel", WhenRestart).Key(K_CTRL_PERIOD);
	bar.Add("Remove all variables", WhenRemoveVariables).Key(K_CTRL|K_ALT|K_R);
	bar.Add("Rename tab", [=] { Todo("Rename tab"); });
	bar.Separator();
	bar.Add("Move", [=] { Todo("Move pane"); });
	bar.Add("Undock", [=] { Todo("Undock pane"); });
	bar.Add("Close", [=] { Todo("Close pane"); });
}

void PythonConsole::Write(const String& s)
{
    output.Append(s);
    output.SetCursor(output.GetLength());
}

void PythonConsole::WriteError(const String& s)
{
    output.Append(s); // TODO: Red color
    output.SetCursor(output.GetLength());
}

void PythonConsole::Clear()
{
    output.Clear();
}

void PythonConsole::OnInput()
{
    last_input = ~input;
    input <<= String();
    
    if(!last_input.IsEmpty()) {
        history.Add(last_input);
        history_index = history.GetCount();
        Write(">>> " + last_input + "\n");
        WhenInput();
    }
}

bool PythonConsole::Key(dword key, int count)
{
    if(key == K_UP) {
        if(history_index > 0) {
            history_index--;
            input <<= history[history_index];
        }
        return true;
    }
    if(key == K_DOWN) {
        if(history_index < history.GetCount() - 1) {
            history_index++;
            input <<= history[history_index];
        }
        else {
            history_index = history.GetCount();
            input <<= String();
        }
        return true;
    }
    return DockableCtrl::Key(key, count);
}

}
