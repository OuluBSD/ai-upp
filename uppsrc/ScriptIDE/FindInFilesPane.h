#ifndef _ScriptIDE_FindInFilesPane_h_
#define _ScriptIDE_FindInFilesPane_h_

class FindInFilesPane : public DockableCtrl {
public:
	typedef FindInFilesPane CLASSNAME;
	FindInFilesPane();

	void SetRoot(const String& path) { root_path = path; }
	
	Event<const String&, int> WhenOpenMatch;

private:
	EditString search_pattern;
	Button     search_btn;
	ArrayCtrl  results;
	String     root_path;

	void OnSearch();
	void OnResultOpen();
};

#endif
