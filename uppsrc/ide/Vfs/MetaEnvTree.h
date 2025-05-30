#ifndef _ide_Vfs_MetaEnvTree_h_
#define _ide_Vfs_MetaEnvTree_h_

class MetaEnvTree : public ParentCtrl {
	Splitter split, lsplit, ltsplit;
	ArrayCtrl pkgs, files;
	TreeCtrl stmts;
	TreeCtrl focus;
	CodeEditor code;
	MenuBar menu;
	Vector<VfsValue*> stmt_ptrs, focus_ptrs;
	VfsValueSubset subset;
	
public:
	typedef MetaEnvTree CLASSNAME;
	MetaEnvTree();
	
	void Data();
	void DataPkg();
	void DataFile();
	void DataTreeSelection();
	void DataFocusSelection();
	bool Key(dword key, int count) override;
	void AddStmtNodes(int parent, VfsValue& n, VfsValueSubset* ns);
	void AddFocusNodes(int parent, VfsValue& n, VfsValueSubset* ns);
	
};

#endif
