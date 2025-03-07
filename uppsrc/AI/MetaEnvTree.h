#ifndef _AI_MetaEnvTree_h_
#define _AI_MetaEnvTree_h_

NAMESPACE_UPP


class MetaEnvTree : public ParentCtrl {
	Splitter split, lsplit, ltsplit;
	ArrayCtrl pkgs, files;
	TreeCtrl stmts;
	TreeCtrl focus;
	CodeEditor code;
	MenuBar menu;
	Vector<MetaNode*> stmt_ptrs, focus_ptrs;
	MetaNodeSubset subset;
	
public:
	typedef MetaEnvTree CLASSNAME;
	MetaEnvTree();
	
	void Data();
	void DataPkg();
	void DataFile();
	void DataTreeSelection();
	void DataFocusSelection();
	bool Key(dword key, int count) override;
	void AddStmtNodes(int parent, MetaNode& n, MetaNodeSubset* ns);
	void AddFocusNodes(int parent, MetaNode& n, MetaNodeSubset* ns);
	
};


END_UPP_NAMESPACE

#endif
