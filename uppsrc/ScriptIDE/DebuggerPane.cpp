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
	bar.Add("Continue", CtrlImg::right_arrow(), WhenContinue);
	bar.Add("Step Over", CtrlImg::plus(), WhenStepOver);
	bar.Add("Step Into", CtrlImg::plus(), WhenStepInto); // TODO: need better icons
	bar.Add("Step Out", CtrlImg::remove(), WhenStepOut);
	bar.Separator();
	bar.Add("Stop", CtrlImg::remove(), WhenStop);
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
