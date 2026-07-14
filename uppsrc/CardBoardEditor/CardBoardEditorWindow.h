#ifndef _CardBoardEditor_CardBoardEditorWindow_h_
#define _CardBoardEditor_CardBoardEditorWindow_h_

class CardBoardEditorWindow : public DockWindow {
public:
	typedef CardBoardEditorWindow CLASSNAME;

	CardBoardEditorWindow();
	void DockInit() override;
	void Close() override;

private:
	MenuBar menu_;
	ToolBar toolbar_;
	CardBoardCanvas canvas_;
	CardBoardTreePanel tree_;
	CardBoardPropertiesPanel properties_;
	CardBoardDiagnosticsPanel diagnostics_;
	String default_layout_data_;
	bool loaded_ = false;

	void MainMenu(Bar& bar);
	void MenuFile(Bar& bar);
	void MenuView(Bar& bar);
	void MenuHelp(Bar& bar);
	void ToolBar(Bar& bar);
	void ResetLayout();
	void RefreshPanels();
	void CacheDefaultLayout();
};

bool RunCardBoardEditorCli(const Vector<String>& args);

#endif
