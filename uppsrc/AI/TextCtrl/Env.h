#ifndef _AI_TextCtrl_Env_h_
#define _AI_TextCtrl_Env_h_

NAMESPACE_UPP


class EnvEditorCtrl : public ToolAppCtrl {
	Splitter hsplit;
	ArrayCtrl ctxlist;
	WithEnvEditor<Ctrl> edit;
	
protected:
	Ptr<MetaNode> file_root;
	Vector<Vector<MetaNode*>> ctx_dbs;
	Vector<MetaNode*> ctxs;
	Vector<Ptr<MetaNode>> dbs
	;
	
	
public:
	typedef EnvEditorCtrl CLASSNAME;
	EnvEditorCtrl();
	
	void Data() override;
	void DataItem();
	void RefreshDatabases();
	void ToolMenu(Bar& bar) override;
	void SetFont(Font fnt);
	MetaSrcFile& RealizeFileRoot();
	void AddContext();
	void RemoveContext();
	void OnLoad(const String& data, const String& filepath) override;
	void OnSave(String& data, const String& filepath) override;
	void OnValueChange();
	void OnOption(Option* opt, MetaNode* db);
	
	static bool AcceptsExt(String e) { return e == ".env"; }
	static String GetID() { return "Environment Editor"; }
	
	static String MakeIdString(const Vector<MetaNode*>& v);
};


END_UPP_NAMESPACE

#endif
