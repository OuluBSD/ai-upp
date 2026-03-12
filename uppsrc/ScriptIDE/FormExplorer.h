#ifndef _ScriptIDE_FormExplorer_h_
#define _ScriptIDE_FormExplorer_h_

struct FormExplorerEntry : Moveable<FormExplorerEntry> {
	String path;
	String type;
	Rect   rect;
	String details;
};

class FormExplorer : public DockableCtrl {
public:
	typedef FormExplorer CLASSNAME;

	FormExplorer();

	void SetScene(const Size& canvas_size, const Vector<FormExplorerEntry>& entries);
	void Clear();

private:
	ToolBar toolbar;
	TreeArrayCtrl tree;

	void LayoutToolbar(Bar& bar);
	String FormatRect(const Rect& r) const;
};

#endif
