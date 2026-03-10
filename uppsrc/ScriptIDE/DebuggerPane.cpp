#include "ScriptIDE.h"

namespace Upp {

DebuggerPane::DebuggerPane()
{
	Title("Debugger");
	Icon(CtrlImg::exclamation());
	
	Add(toolbar.TopPos(0, 24).HSizePos());
	Add(stack_tree.VSizePos(24, 0).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
	
	stack_tree.WhenSel = [=] { OnTreeCursor(); };
}

void DebuggerPane::LayoutToolbar(Bar& bar)
{
	bar.Add("Debug current line", CtrlImg::right_arrow(), WhenContinue).Help("Debug current line (F10)");
	bar.Add("Execute until next breakpoint", CtrlImg::right_arrow(), WhenContinue).Help("Execute until next breakpoint (F12)");
	bar.Add("Step into function or method", CtrlImg::plus(), WhenStepInto).Help("Step into (F11)");
	bar.Add("Execute until function returns", CtrlImg::undo(), WhenStepOut).Help("Execute until returns (Shift+F11)");
	bar.Add("Stop debugging", CtrlImg::remove(), WhenStop).Help("Stop debugging (Shift+F12)");
	bar.Separator();
	bar.Add("Start debugging after last error", [=] { Todo("Debug after error"); });
	bar.Add("Interrupt execution and start the debugger", [=] { Todo("Interrupt and debug"); });
	bar.Add("Inspect execution", [=] { Todo("Inspect execution"); });
	bar.Separator();
	bar.Add("Show file/line in editor", [=] { Todo("Show in editor"); });
	bar.Add("Search frames", [=] { Todo("Search frames"); });
	bar.Gap(2000);
	bar.Add("Show breakpoints", [=] { Todo("Show breakpoints"); });
	bar.Sub("Options", CtrlImg::plus(), [=](Bar& b) { LayoutPaneMenu(b); });
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
	stack_tree.SetRoot(CtrlImg::Dir(), "Call Stack");
	int root = 0;
	for(int i = 0; i < stack.GetCount(); i++) {
		const auto& f = stack[i];
		String label = Format("%s (%s:%d)", f.function_name, GetFileName(f.file), f.line);
		stack_tree.Add(root, CtrlImg::File(), i, label);
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
