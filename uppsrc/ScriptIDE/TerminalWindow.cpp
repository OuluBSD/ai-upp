#include "ScriptIDE.h"
#include "TerminalWindow.h"

namespace Upp {

TerminalWindow::TerminalWindow()
{
	Title("Terminal");
	Add(terminal.SizePos());
	Sizeable().Zoomable();
	SetRect(0, 0, 800, 600);

	terminal.WhenOutput = [this](String s) { pty.Write(s); };
	terminal.WhenResize = [this]()         { pty.SetSize(terminal.GetPageSize()); };
	
	BackPaint();
}

void TerminalWindow::Execute(const String& cmdline, const String& dir)
{
	pty.Start(cmdline, Environment(), dir);
	Open();
	SetTimeCallback(-10, [this] { Sync(); });
}

void TerminalWindow::Sync()
{
	if(!IsOpen()) {
		delete this;
		return;
	}
	
	terminal.WriteUtf8(pty.Get());
	
	if(pty.IsRunning()) {
		SetTimeCallback(-10, [this] { Sync(); });
	} else {
		terminal.Write("\n[Process finished]\n");
	}
}

}
