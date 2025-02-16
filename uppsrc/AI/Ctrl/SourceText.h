#ifndef _AI_TextCtrl_SourceText_h_
#define _AI_TextCtrl_SourceText_h_

NAMESPACE_UPP

class SourceTextCtrl;

class SourceDataCtrl : public ParentCtrl {
	ToolAppCtrl& o;
	Splitter vsplit, hsplit;
	ArrayCtrl entities, components; //, active_components;
	DocEdit scripts, analysis;
	int data_type = 0;
	
public:
	typedef SourceDataCtrl CLASSNAME;
	SourceDataCtrl(ToolAppCtrl& o);

	void Data();
	void DataEntity();
	void DataExtension();
	void SetFont(Font fnt);
	void SetDataType(int i) {data_type = i;}
};


// TODO rename
class TokensPage : public ParentCtrl {
	ToolAppCtrl& o;
	Splitter hsplit;
	ArrayCtrl tokens;
	
public:
	typedef TokensPage CLASSNAME;
	TokensPage(ToolAppCtrl& o);
	
	void Data();
};

// TODO rename
class TextDataWords : public ParentCtrl {
	ToolAppCtrl& o;
	Splitter hsplit, vsplit;
	ArrayCtrl colors, words;
	bool disabled = false;
	bool batch = false;
	
public:
	typedef TextDataWords CLASSNAME;
	TextDataWords(ToolAppCtrl& o);
	
	void EnableAll();
	void DisableAll();
	void Data();
	void DataColor();
	void DumpWordGroups();
	void DumpPhoneticChars();
};

// TODO rename
class TokenPhrases : public ParentCtrl {
	ToolAppCtrl& o;
	Splitter hsplit;
	ArrayCtrl texts;
	
public:
	typedef TokenPhrases CLASSNAME;
	TokenPhrases(ToolAppCtrl& o);
	
	void Data();
};

// TODO rename
class AmbiguousWordPairs : public ParentCtrl {
	ToolAppCtrl& o;
	Splitter vsplit, hsplit;
	ArrayCtrl texts;
	
public:
	typedef AmbiguousWordPairs CLASSNAME;
	AmbiguousWordPairs(ToolAppCtrl& o);
	
	void Data();
};

String GetTypePhraseString(const Vector<int>& word_classes, const SrcTextData& da);

// TODO rename
class VirtualPhrases : public ParentCtrl {
	ToolAppCtrl& o;
	Splitter vsplit, hsplit;
	ArrayCtrl texts, parts;
	
public:
	typedef VirtualPhrases CLASSNAME;
	VirtualPhrases(ToolAppCtrl& o);
	
	void Data();
};

// TODO rename
class VirtualPhraseParts : public ParentCtrl {
	ToolAppCtrl& o;
	Splitter vsplit, hsplit;
	ArrayCtrl texts, parts;
	
public:
	typedef VirtualPhraseParts CLASSNAME;
	VirtualPhraseParts(ToolAppCtrl& o);
	
	void Data();
};

// TODO rename
class VirtualPhraseStructs : public ParentCtrl {
	ToolAppCtrl& o;
	Splitter vsplit, hsplit;
	ArrayCtrl texts, parts;
	
public:
	typedef VirtualPhraseStructs CLASSNAME;
	VirtualPhraseStructs(ToolAppCtrl& o);
	
	void Data();
};

struct ScoreDisplay : public Display {
	virtual void Paint(Draw& w, const Rect& r, const Value& q,
	                   Color ink, Color paper, dword style) const;
};

// TODO rename
class PhrasePartAnalysis : public ParentCtrl {
	ToolAppCtrl& o;
	Splitter vsplit, hsplit;
	ArrayCtrl attrs, colors, actions, action_args, parts;
	
	VectorMap<String, VectorMap<String, int>> uniq_acts;
	
public:
	typedef PhrasePartAnalysis CLASSNAME;
	PhrasePartAnalysis(ToolAppCtrl& o);
	
	void Data();
	void DataMain();
	void DataAttribute();
	void DataColor();
	void DataAction();
	void DataActionHeader();
	void UpdateCounts();
};

// TODO rename
class PhrasePartAnalysis2 : public ParentCtrl {
	ToolAppCtrl& o;
	Splitter vsplit, hsplit;
	ArrayCtrl elements, typecasts, contrasts, parts,  colors;
	
public:
	typedef PhrasePartAnalysis2 CLASSNAME;
	PhrasePartAnalysis2(ToolAppCtrl& o);
	
	void Data();
	void DataMain();
	void DataElement();
	void DataTypeclass();
	void DataContrast();
	void DataColor();
	void UpdateCounts();
	void ClearAll();
};

// TODO rename
class ActionAttrsPage : public ParentCtrl {
	ToolAppCtrl& o;
	Splitter vsplit, hsplit;
	ArrayCtrl attrs, colors, actions;
	
public:
	typedef ActionAttrsPage CLASSNAME;
	ActionAttrsPage(ToolAppCtrl& o);
	
	void Data();
	void DataAttribute();
	void DataColor();
	void UpdateFromCache();
};

// TODO rename
class Attributes : public ParentCtrl {
	ToolAppCtrl& o;
	Splitter hsplit, vsplit;
	ArrayCtrl groups, values, pos_values, neg_values;
	
	VectorMap<String,Index<String>> uniq_attrs;
	VectorMap<String,Index<int>> uniq_attrs_i;
	
	void RealizeTemp();
	
public:
	typedef Attributes CLASSNAME;
	Attributes(ToolAppCtrl& o);
	
	void Data();
	void DataGroup();
};

// TODO rename
class TextDataDiagnostics : public ParentCtrl {
	ToolAppCtrl& o;
	Splitter hsplit;
	ArrayCtrl values;
	
public:
	typedef TextDataDiagnostics CLASSNAME;
	TextDataDiagnostics(ToolAppCtrl& o);
	
	void Data();
};

class SourceTextCtrl : public ToolAppCtrl {
	DropList				data_type;
	SourceDataCtrl			src;
	TokensPage				tk;
	AmbiguousWordPairs		awp;
	VirtualPhrases			vp;
	VirtualPhraseParts		vpp;
	VirtualPhraseStructs	vps;
	PhrasePartAnalysis		vpa;
	PhrasePartAnalysis2		vpa2;
	ActionAttrsPage			aap;
	Attributes				att;
	TextDataDiagnostics		diag;
	
	void Load();
public:
	typedef SourceTextCtrl CLASSNAME;
	SourceTextCtrl();
	
	void SetFont(Font fnt);
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void Visit(NodeVisitor& vis) override;
	bool Load(const String& includes, const String& filename, Stream& in, byte charset) override;
	void Do(int fn);
	void SetDataCtrl();
	
	static bool AcceptsExt(String e) { return e == ".db-src"; }
	static String GetID() { return "SourceText"; }
};

END_UPP_NAMESPACE

#endif
