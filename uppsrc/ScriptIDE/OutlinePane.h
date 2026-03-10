#ifndef _ScriptIDE_OutlinePane_h_
#define _ScriptIDE_OutlinePane_h_

class OutlinePane : public DockableCtrl {
public:
	typedef OutlinePane CLASSNAME;
	OutlinePane();

	void UpdateOutline(const String& code);
	void Clear();
	
	Event<int> WhenSelectLine;

	void LayoutToolbar(Bar& bar);
	void LayoutPaneMenu(Bar& bar);

private:
	ToolBar toolbar;
	TreeCtrl tree;
	
	void OnSelect();
};

#endif
