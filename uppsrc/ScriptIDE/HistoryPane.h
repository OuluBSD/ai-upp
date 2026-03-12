#ifndef _ScriptIDE_HistoryPane_h_
#define _ScriptIDE_HistoryPane_h_

class HistoryPane : public DockableCtrl {
public:
	typedef HistoryPane CLASSNAME;
	HistoryPane();

	void LayoutToolbar(Bar& bar);
	void LayoutPaneMenu(Bar& bar);

private:
	ToolBar    toolbar;
	CodeEditor editor;
	bool wrap_lines = true;
	bool show_line_numbers = false;

	void UpdateEditor();
};

#endif
