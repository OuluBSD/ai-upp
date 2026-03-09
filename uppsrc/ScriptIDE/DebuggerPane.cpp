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
	bar.Add("Continue", CtrlImg::right_arrow(), WhenContinue).Help("Execute until next breakpoint");
	bar.Add("Step Over", CtrlImg::plus(), WhenStepOver).Help("Debug current line");
	bar.Add("Step Into", CtrlImg::plus(), WhenStepInto).Help("Step into function or method");
	bar.Add("Step Out", CtrlImg::remove(), WhenStepOut).Help("Execute until function returns");
	bar.Add("Stop", CtrlImg::remove(), WhenStop).Help("Stop debugging");
	bar.Separator();
	bar.Add("Post-mortem", [=] {}).Help("Start debugging after last error");
	bar.Add("Interrupt", [=] {}).Help("Interrupt execution");
	bar.Add("Inspect", [=] {}).Help("Inspect execution");
	bar.Separator();
	bar.Add("Show file", [=] {}).Help("Show file/line in editor");
	bar.Add("Search frames", [=] {});
	bar.Gap(2000);
	bar.Add("Breakpoints", [=] {}).Help("Show breakpoints");
	bar.Sub("Options", CtrlImg::plus(), [=](Bar& b) { LayoutPaneMenu(b); });
}

void DebuggerPane::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Exclude internal frames", [=] {}).Check(true);
}

void DebuggerPane::SetStack(const Vector<PyVM::StackFrame>& stack)
{
	stack_tree.Clear();
	stack_tree.SetRoot(CtrlImg::Dir(), "Call Stack");
	
	for(int i = 0; i < stack.GetCount(); i++) {
		const auto& frame = stack[i];
		String label = Format("%s (%s:%d)", frame.function_name, GetFileName(frame.file), frame.line);
		stack_tree.Add(0, CtrlImg::File(), frame.frame_index, label);
	}
	
	stack_tree.OpenDeep(0);
}

void DebuggerPane::Clear()
{
	stack_tree.Clear();
}

void DebuggerPane::OnTreeCursor()
{
	int id = stack_tree.GetCursor();
	if(id > 0) { // skip root
		Value v = stack_tree.Get(id);
		if(!v.IsNull())
			WhenFrameSelected((int)v);
	}
}

}
