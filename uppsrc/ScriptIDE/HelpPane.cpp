#include "ScriptIDE.h"

namespace Upp {

HelpPane::HelpPane()
{
	Title("Help");
	Icon(Icons::Help());
	
	source_selector.Add("Console");
	source_selector.Add("Editor");

	Add(header.TopPos(0, 36).HSizePos());
	Add(viewer.VSizePos(36, 0).HSizePos());
	
	header.Set([=](Bar& bar) { LayoutHeader(bar); });
}

void HelpPane::LayoutHeader(Bar& bar)
{
	bar.Add(source_selector);
	bar.Add(object_input);
	bar.Add(Icons::Undo(), [=] { OnHome(); }).Tip("Home button").Help("Home");
	bar.Add(Icons::Plus(), [=] { OnLock(); }).Tip("Lock button").Help("Lock"); // Using plus as lock icon placeholder
	bar.ToolGapRight();
	bar.Sub("Options", Icons::Settings(), [=](Bar& b) { LayoutPaneMenu(b); }).Tip("Pane menu");
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
