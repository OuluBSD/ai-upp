#ifndef _ModelerApp_RibbonCtrl_h_
#define _ModelerApp_RibbonCtrl_h_

class XmlNode;
struct Edit3D;

struct ModelerAppRibbon : RibbonBar {
	Edit3D* owner = nullptr;
	bool loaded = false;
	String spec_path;
	int quick_access_pos = RibbonBar::QAT_TOP;

	struct SpecItem : Moveable<SpecItem> {
		String id;
		String text;
		String type;
		String group_id;
		String tab_id;
	};
	Vector<SpecItem> items;
	VectorMap<String, Ctrl*> control_by_id;
	Vector<One<Ctrl>> owned_ctrls;

	Event<String> WhenAction;

	ModelerAppRibbon();
	void Init(Edit3D* o);
	void Clear();
	bool LoadSpec(const String& path);
	bool LoadSpecXml(const String& xml);
	void BuildFromSpec(const XmlNode& root);
	void OnAction(const String& id);
	Ctrl* FindControl(const String& id) const;
	void SetupQuickAccess();
	void AddContextTabs();
	int GetDisplayMode() const { return RibbonBar::GetDisplayMode(); }
	void SetDisplayMode(int mode) { RibbonBar::SetDisplayMode((RibbonBar::DisplayMode)mode); }
	int GetQuickAccessPos() const { return quick_access_pos; }
	void SetQuickAccessPos(int pos) { quick_access_pos = pos; RibbonBar::SetQuickAccessPos((RibbonBar::QuickAccessPos)pos); }
};

#endif
