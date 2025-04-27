#ifndef _ide_Meta_EditorCtrl_h_
#define _ide_Meta_EditorCtrl_h_


class EntityEditorCtrl : public ToolAppCtrl {
	Splitter hsplit, lsplit;
	//ArrayCtrl entlist, extlist;
	TreeCtrl ecs_tree;
	TreeCtrl content_tree;
	Ctrl ext_place;
	One<MetaExtCtrl> ext_ctrl;
	int ext_ctrl_kind = -1;
	int post_content_cursor = -1;
	
	void DataEcsTreeVisit(int treeid, MetaNode& n);
	
protected:
	Ptr<MetaNode> file_root;
	Vector<MetaNode*> ecs_tree_nodes;
	
public:
	typedef EntityEditorCtrl CLASSNAME;
	EntityEditorCtrl();
	
	void Data() override;
	void SetFont(Font fnt);
	void ToolMenu(Bar& bar) override;
	MetaSrcFile& RealizeFileRoot();
	void DataEcsTree_RefreshNames();
	void DataEcsTree();
	void DataExtension();
	void DataExtCtrl();
	void Visit(Vis& v) override;
	void MoveNode(MetaNode* n);
	void RemoveNode(MetaNode* n);
	void AddNode(MetaNode* n, int kind, String id);
	void AddEntity();
	void RemoveEntity();
	void AddComponent();
	void RemoveComponent();
	void SetExtensionCtrl(int kind, MetaExtCtrl* ctrl);
	void ClearExtensionCtrl() {SetExtensionCtrl(-1,0);}
	MetaNodeExt* GetSelected();
	void EditPos(JsonIO& jio) override;
	void SelectEcsTree(MetaNode* n);
	MetaNode* SelectTreeNode(String title);
	
	static bool AcceptsExt(String e) { return e == ".ecs"; }
	static String GetID() { return "Entity Editor"; }
	
	void Do(int i);
};


#endif
