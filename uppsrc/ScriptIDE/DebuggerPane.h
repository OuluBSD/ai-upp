#ifndef _ScriptIDE_DebuggerPane_h_
#define _ScriptIDE_DebuggerPane_h_

class DebuggerPane : public DockableCtrl {
public:
	typedef DebuggerPane CLASSNAME;
	DebuggerPane();

	void SetStack(const Vector<PyVM::StackFrame>& stack);
	void Clear();

	Event<> WhenStepOver;
	Event<> WhenStepInto;
	Event<> WhenStepOut;
	Event<> WhenContinue;
	Event<> WhenStop;
	Event<int> WhenFrameSelected;

private:
	ToolBar toolbar;
	TreeCtrl stack_tree;
	
	void LayoutToolbar(Bar& bar);
	void LayoutPaneMenu(Bar& bar);
	void OnTreeCursor();
};

#endif
