#include "ScriptIDE.h"

namespace Upp {

DebuggerPane::DebuggerPane()
{
	Title("Debugger");
	Icon(Icons::Debug());
	
	Add(toolbar.TopPos(0, 36).HSizePos());
	Add(stack_tree.VSizePos(36, 0).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
	
	stack_tree.WhenSel = [=] { OnTreeCursor(); };
}

void DebuggerPane::LayoutToolbar(Bar& bar)
{
	bar.Add(Icons::StepOver(), WhenContinue).Tip("Debug current line").Help("Debug current line (F10)");
	bar.Add(Icons::Run(), WhenContinue).Tip("Execute until next breakpoint").Help("Execute until next breakpoint (F12)");
	bar.Add(Icons::StepIn(), WhenStepInto).Tip("Step into function or method").Help("Step into (F11)");
	bar.Add(Icons::StepOut(), WhenStepOut).Tip("Execute until function returns").Help("Execute until returns (Shift+F11)");
	bar.Add(Icons::Stop(), WhenStop).Tip("Stop debugging").Help("Stop debugging (Shift+F12)");
	bar.Separator();
	bar.Add(Icons::Plus(), [=] { Todo("Debug after error"); }).Tip("Start debugging after last error");
	bar.Add(Icons::Stop(), [=] { Todo("Interrupt and debug"); }).Tip("Interrupt execution and start the debugger");
	bar.Add(Icons::Search(), [=] { Todo("Inspect execution"); }).Tip("Inspect execution");
	bar.Separator();
	bar.Add(Icons::File(), [=] { Todo("Show in editor"); }).Tip("Show file/line in editor");
	bar.Add(Icons::Search(), [=] { Todo("Search frames"); }).Tip("Search frames");
	bar.ToolGapRight();
	bar.Add(Icons::Breakpoint(), [=] { Todo("Show breakpoints"); }).Tip("Show breakpoints");
	bar.Sub("Options", Icons::Settings(), [=](Bar& b) { LayoutPaneMenu(b); }).Tip("Pane menu");
}

void DebuggerPane::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Exclude internal frames when inspecting execution", [=] {}).Check(true);
	bar.Separator();
	bar.Add("Move", [=] { Todo("Move pane"); });
	bar.Add("Undock", [=] { Todo("Undock pane"); });
	bar.Add("Close", [=] { Todo("Close pane"); });
}

void DebuggerPane::SetStack(const Vector<PyVM::StackFrame>& stack)
{
	stack_tree.Clear();
	stack_tree.SetRoot(Icons::Folder(), "Call Stack");
	int root = 0;
	for(int i = 0; i < stack.GetCount(); i++) {
		const auto& f = stack[i];
		String label = Format("%s (%s:%d)", f.function_name, GetFileName(f.file), f.line);
		stack_tree.Add(root, Icons::File(), i, label);
	}
	stack_tree.OpenDeep(root);
}

void DebuggerPane::Clear()
{
	stack_tree.Clear();
}

void DebuggerPane::OnTreeCursor()
{
	int id = stack_tree.GetCursor();
	if(id >= 0) {
		Value v = stack_tree.Get(id);
		if(!v.IsVoid())
			WhenFrameSelected((int)v);
	}
}

}
