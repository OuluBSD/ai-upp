#ifndef _AI_TextCtrl_SourceText_h_
#define _AI_TextCtrl_SourceText_h_

NAMESPACE_UPP

class SourceTextCtrl;

class SourceDataCtrl : public ParentCtrl {
	SourceTextCtrl& o;
	Splitter vsplit, hsplit;
	ArrayCtrl entities, components; //, active_components;
	DocEdit scripts, analysis;
	int data_type = 0;
	
public:
	typedef SourceDataCtrl CLASSNAME;
	SourceDataCtrl(SourceTextCtrl& o);

	void Data();
	void ToolMenu(Bar& bar);
	void DataEntity();
	void DataComponent();
	void SetFont(Font fnt);
	void SetDataType(int i) {data_type = i;}
};


// TODO rename
class TokensPage : public ParentCtrl {
	SourceTextCtrl& o;
	Splitter hsplit;
	ArrayCtrl tokens;
	
public:
	typedef TokensPage CLASSNAME;
	TokensPage(SourceTextCtrl& o);
	
	void Data();
	void ToolMenu(Bar& bar);
	
	
};


class SourceTextCtrl : public ToolAppCtrl {
	DropList		data_type;
	SourceDataCtrl	src;
	TokensPage		tk;
	
public:
	typedef SourceTextCtrl CLASSNAME;
	SourceTextCtrl();
	
	void SetFont(Font fnt);
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void OnLoad(const String& data, const String& filepath) override;
	void OnSave(String& data, const String& filepath) override;
	void Do(int fn);
	void SetDataCtrl();
	
	static DbContent GetDbType() { return DBCONTENT_SRCTEXT; }
	static String GetExt() { return ".db-src"; }
	static String GetID() { return "SourceText"; }
	
};

END_UPP_NAMESPACE

#endif
