#ifndef _ScriptIDE_FilesPane_h_
#define _ScriptIDE_FilesPane_h_

class FilesPane : public DockableCtrl {
public:
	typedef FilesPane CLASSNAME;
	FilesPane();

	void SetRoot(const String& path);
	void Refresh();

	Event<const String&> WhenOpen;

private:
	TreeCtrl tree;
	String root_path;

	void Populate(int id);
	void OnOpen();
};

#endif
