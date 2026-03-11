#ifndef _ScriptIDE_FindInFilesPane_h_
#define _ScriptIDE_FindInFilesPane_h_

class FindInFilesPane : public DockableCtrl {
public:
	typedef FindInFilesPane CLASSNAME;
	FindInFilesPane();

	void SetRoot(const String& path) { root_path = path; }
	
	Event<const String&, int> WhenOpenMatch;

private:
	ToolBar    toolbar;
	Label      pattern_lbl;
	EditString search_pattern;
	Label      files_lbl;
	EditString files_pattern;
	Button     search_btn;
	Button     stop_btn;
	Button     browse_btn;
	Option     regex_toggle;
	Option     case_toggle;
	
	ArrayCtrl  results;
	String     root_path;

	void OnSearch();
	void OnResultOpen();
	void LayoutToolbar(Bar& bar);
	void LayoutPaneMenu(Bar& bar);
};

#endif
