#ifndef _ScriptIDE_FilesPane_h_
#define _ScriptIDE_FilesPane_h_

class FilesPane : public DockableCtrl {
public:
	typedef FilesPane CLASSNAME;
	FilesPane();

	void SetRoot(const String& path);
	String GetRoot() const { return root_path; }
	void ShowHidden(bool b = true) { show_hidden = b; Refresh(); }
	void Refresh();

	Event<const String&> WhenOpen;
	Event<> WhenPathManager;
	Event<> WhenBrowse;
	Event<> WhenParent;

private:
	ToolBar location_bar;
	ToolBar pane_toolbar;
	TreeCtrl tree;
	String root_path;
	bool   show_hidden = false;

	void Populate(int id);
	void OnOpen();
	void LayoutLocationBar(Bar& bar);
	void LayoutPaneToolbar(Bar& bar);
	void LayoutPaneMenu(Bar& bar);
};

#endif
