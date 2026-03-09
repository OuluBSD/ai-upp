#ifndef _ScriptIDE_OutlinePane_h_
#define _ScriptIDE_OutlinePane_h_

class OutlinePane : public DockableCtrl {
public:
	typedef OutlinePane CLASSNAME;
	OutlinePane();

	void UpdateOutline(const String& code);
	void Clear();
	
	Event<int> WhenSelectLine;

private:
	TreeCtrl tree;
	
	void OnSelect();
};

#endif
