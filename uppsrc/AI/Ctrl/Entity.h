#ifndef _AI_TextCtrl_Entity_h_
#define _AI_TextCtrl_Entity_h_

NAMESPACE_UPP

struct Script;
class ToolAppCtrl;

class ComponentCtrl : public MetaExtCtrl {
	
public:
	DatasetPtrs GetDataset();
	Script& GetScript();
	
	const Index<String>& GetTypeclasses() const;
	const Vector<ContentType>& GetContents() const;
	const Vector<String>& GetContentParts() const;
	
};

struct VirtualNode : Moveable<VirtualNode> {
	enum {
		VFS_INVALID = -1,
		VFS_VALUE,
		VFS_ENTITY
	};
	struct Data {
		Atomic refs = 0;
		MetaNode* node = 0;
		Value* value = 0;
		String key;
		int mode = -1;
		void Inc() {refs++;}
		void Dec() {refs--; if (refs <= 0) delete this;}
	};
	Data* data = 0;
	VirtualNode();
	VirtualNode(const VirtualNode& vn);
	VirtualNode(VirtualNode&& vn);
	VirtualNode& operator=(const VirtualNode& vn);
	~VirtualNode();
	VirtualNode Find(String name);
	Vector<VirtualNode> GetAll();
	Vector<VirtualNode> FindAll(int kind);
	VirtualNode Add(String name, int kind);
	String GetName() const;
	String GetKindString() const;
	int GetKind() const;
	void SetKind(int k);
	operator bool() const;
	void Clear();
	void RemoveSubNodes();
	void Remove(const String& name);
	Data& Create();
	Data& Create(MetaNode* n);
	Data& Create(Value* v, String key);
};

class VNodeComponentCtrl;

class VirtualFSComponentCtrl : public ComponentCtrl {
	One<VNodeComponentCtrl> vnode_ctrl;
	VfsPath vnode_path;
	int data_iter = 0;
	
	void Data() override;
	bool Visit(TreeCtrl& tree, int id, VirtualNode n);
	void OnTreeCursor(TreeCtrl* tree);
protected:
	friend class ValueVFSComponentCtrl;
	VirtualFSComponentCtrl();
	void DataTree(TreeCtrl& tree) override;
	VfsPath GetCursorPath() const override;
	VfsPath GetCursorRelativePath() const;
public:
	typedef VirtualFSComponentCtrl CLASSNAME;
	
	virtual VirtualNode Root() = 0;
	virtual VNodeComponentCtrl* CreateCtrl(const VirtualNode& vnode) = 0;
	virtual void Init() {}
	
	VirtualNode GetAdd(const VfsPath& rel_path, int kind);
	VirtualNode Find(const VfsPath& rel_path);
	
};

class ValueVFSComponentCtrl : public VirtualFSComponentCtrl {
	VirtualNode root;
public:
	typedef ValueVFSComponentCtrl CLASSNAME;
	ValueVFSComponentCtrl();
	VirtualNode Root() override;
	Value* GetValue() override;
	void Set(Value key, Value value);
	Value Get(Value key);
};

class VNodeComponentCtrl : public ParentCtrl {
	
protected:
	friend class VirtualFSComponentCtrl;
	int kind = 0;
	
public:
	typedef VNodeComponentCtrl CLASSNAME;
	VNodeComponentCtrl();
	
	int GetKind() const {return kind;}
	virtual void Data() {}
	
};

class EntityInfoCtrl : public MetaExtCtrl {
	WithEntityInfo<Ctrl> info;
	VectorMap<String,MetaNode*> all_ctxs;
	ArrayCtrl data;
	DocEdit value;
	
public:
	typedef EntityInfoCtrl CLASSNAME;
	EntityInfoCtrl();
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void OnEdit();
	void OnEditValue();
	void DataCursor();
	DatasetPtrs GetDataset();
	
	Event<> WhenValueChange;
};

INITIALIZE(EntityInfoCtrl)


class EntityEditorCtrl : public ToolAppCtrl {
	Splitter hsplit, lsplit;
	//ArrayCtrl entlist, extlist;
	TreeCtrl ecs_tree;
	TreeCtrl content_tree;
	Ctrl ext_place;
	One<MetaExtCtrl> ext_ctrl;
	int ext_ctrl_kind = -1;
	
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
	void Visit(NodeVisitor& vis) override;
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

END_UPP_NAMESPACE

#endif
