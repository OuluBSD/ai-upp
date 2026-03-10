#include "ScriptIDE.h"

namespace Upp {

HelpPane::HelpPane()
{
	Title("Help");
	Icon(CtrlImg::help());
	
	source_selector.Add("Console");
	source_selector.Add("Editor");

	Add(header.TopPos(0, 24).HSizePos());
	Add(viewer.VSizePos(24, 0).HSizePos());
	
	header.Set([=](Bar& bar) { LayoutHeader(bar); });
}

void HelpPane::LayoutHeader(Bar& bar)
{
	bar.Add(source_selector);
	bar.Add(object_input);
	bar.Add(CtrlImg::Home(), [=] { OnHome(); }).Help("Home");
	bar.Add(CtrlImg::plus(), [=] { OnLock(); }).Help("Lock"); // Using plus as lock icon placeholder
	bar.Gap(2000);
	bar.Sub("Options", CtrlImg::plus(), [=](Bar& b) { LayoutPaneMenu(b); });
}

void HelpPane::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Rich Text", [=] { Todo("Rich Text mode"); }).Check(true);
	bar.Add("Plain Text", [=] { Todo("Plain Text mode"); }).Check(false);
	bar.Add("Show Source", [=] { Todo("Show Source"); }).Check(false);
	bar.Separator();
	bar.Add("Automatic import", [=] { Todo("Auto import"); }).Check(false);
	bar.Separator();
	bar.Add("Move", [=] { Todo("Move pane"); });
	bar.Add("Undock", [=] { Todo("Undock pane"); });
	bar.Add("Close", [=] { Todo("Close pane"); });
}

void HelpPane::SetQTF(const String& qtf)
{
	viewer.SetQTF(qtf);
}

void HelpPane::Clear()
{
	viewer.Clear();
}

void HelpPane::OnHome() { Todo("Home"); }
void HelpPane::OnLock() { Todo("Lock"); }

}
