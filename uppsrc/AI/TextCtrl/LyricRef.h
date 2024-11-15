#ifndef _AI_TextCtrl_LyricRef_h_
#define _AI_TextCtrl_LyricRef_h_

NAMESPACE_UPP


struct NavigatorState {
	PartLineCtrl* line = 0;
	int depth = -1;
	LineElement* el = 0;
	hash_t sorter = 0;
	String element;
	AttrHeader attr;
	int clr_i = -1;
	ActionHeader act;
	int typeclass_i = -1;
	int con_i = -1;
	void Clear();
	void RemoveDuplicate(const NavigatorState& s);
};



class ScriptPhrasePartsGroups : public Ctrl {
	ToolAppCtrl& o;
	Splitter vsplit, hsplit;
	bool set_cursor = true;
	
protected:
	friend class ScriptReferenceMakerCtrl;
	DropList mode;
	ArrayCtrl attr_groups, attr_values, colors, actions, action_args, parts;
	ArrayCtrl elements, typeclasses, contents;
	
	VectorMap<String, VectorMap<String, int>> uniq_acts;
	Vector<int> sort;
	
public:
	typedef ScriptPhrasePartsGroups CLASSNAME;
	ScriptPhrasePartsGroups(ToolAppCtrl& o);
	
	void UpdateNavigator();
	void Data();
	void DataList();
	void UpdateCounts();
	void SetIndexCursor(int idx, ArrayCtrl& arr);
	void InitArray(ArrayCtrl& arr, String title, DatabaseBrowser::ColumnType t);
	void FillArrayCtrlColor(DatabaseBrowser::ColumnType t, ArrayCtrl& arr);
	void FillArrayCtrl(DatabaseBrowser::ColumnType t, ArrayCtrl& arr);
	hash_t GetModeHash() const;
	void SetModeCursor(int mode);
	
	Event<> WhenBrowserCursor;
	
};

void ReadNavigatorState(Script& s, int part_i, int sub_i, int line_i, NavigatorState& state, int depth_limit);

class ScriptReferenceMakerCtrl : public ToolAppCtrl {
	
protected:
	friend class PartContentCtrl;
	friend class PartLineCtrl;
	
	Splitter hsplit, lsplit, split;
	ArrayCtrl parts;
	WithPartInfoForm<Ctrl> form;
	PartContentCtrl content;
	ScriptPhrasePartsGroups db0;
	
public:
	typedef ScriptReferenceMakerCtrl CLASSNAME;
	ScriptReferenceMakerCtrl();
	
	void Data() override;
	void DataPart();
	void DataLine();
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	void OnValueChange();
	void MakeLines();
	void OnBrowserCursor();
	void SetLineText();
	void UpdateMode();
	int GetActiveMode();
	int GetInheritedMode();
	void SetFont(Font fnt);
	void Save(Stream& s, byte charset);
	void ReadNavigatorState(NavigatorState& state, int depth_limit=INT_MAX);
	
	static String GetID() {return "lyric-ref";}
	static String GetExt() {return ".lyref";}
};

END_UPP_NAMESPACE

#endif
