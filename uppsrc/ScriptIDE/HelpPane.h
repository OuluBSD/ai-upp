#ifndef _ScriptIDE_HelpPane_h_
#define _ScriptIDE_HelpPane_h_

class HelpPane : public DockableCtrl {
public:
	typedef HelpPane CLASSNAME;
	HelpPane();

	void SetQTF(const String& qtf);
	void Clear();

	void LayoutHeader(Bar& bar);
	void LayoutPaneMenu(Bar& bar);

private:
	ToolBar header;
	RichTextCtrl viewer;
	DropList source_selector;
	EditString object_input;

	void OnHome();
	void OnLock();
};

#endif
