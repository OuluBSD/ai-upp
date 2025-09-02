#ifndef _ide_Vfs_MetaEnvTree_h_
#define _ide_Vfs_MetaEnvTree_h_

class MetaEnvTree : public ParentCtrl {
	
protected:
	friend struct ::VfsDlg;
	
	Splitter split, lsplit, ltsplit;
	ArrayCtrl pkgs, files;
	TreeCtrl stmts;
	TreeCtrl focus;
	CodeEditor code;
	MenuBar menu;
	Vector<VfsValue*> stmt_ptrs, focus_ptrs;
	VfsValueSubset subset;
	int tree_limit = 1000;
	bool dlgmode = false;
	
public:
	typedef MetaEnvTree CLASSNAME;
	MetaEnvTree();
	
	void Data();
	void DataPkg();
	void DataFile();
	void DataTreeSelection();
	void DataFocusSelection();
	bool Key(dword key, int count) override;
	void AddStmtNodes(int parent, VfsValue& n, VfsValueSubset* ns, int& count);
	void AddFocusNodes(int parent, VfsValue& n, VfsValueSubset* ns, int& count);
	
};

#endif
