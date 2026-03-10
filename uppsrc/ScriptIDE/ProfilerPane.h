#ifndef _ScriptIDE_ProfilerPane_h_
#define _ScriptIDE_ProfilerPane_h_

class ProfilerPane : public DockableCtrl {
public:
	typedef ProfilerPane CLASSNAME;
	ProfilerPane();

	void LayoutToolbar(Bar& bar);
	void LayoutPaneMenu(Bar& bar);
	void SetData(const VectorMap<String, Value>& data);
	void Clear();

private:
	ToolBar toolbar;
	ArrayCtrl list;
};

#endif
