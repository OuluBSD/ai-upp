#ifndef _AI_TextCtrl_Entity_h_
#define _AI_TextCtrl_Entity_h_


struct Script;
class ToolAppCtrl;

struct ComponentCtrl : MetaExtCtrl, DatasetProvider {
	DatasetPtrs GetDataset() const override;
	Script& GetScript();
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
		EntityData* edata = 0;
		Value* value = 0;
		Value key;
		VfsPath path;
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
	VirtualNode Find(Value name);
	Vector<VirtualNode> GetAll();
	Vector<VirtualNode> FindAll(int kind);
	VirtualNode Add(Value name, int kind);
	Value GetName() const;
	String GetKindString() const;
	int GetKind() const;
	void SetKind(int k);
	bool IsValue() const;
	bool IsEntity() const;
	Value GetValue() const;
	void WriteValue(Value val);
	operator bool() const;
	void Clear();
	void RemoveSubNodes();
	void Remove(const Value& name);
	//Data& Create();
	Data& Create(const VfsPath& p, MetaNode* n);
	Data& Create(const VfsPath& p, Value* v, Value key);
	
	template <class T>
	T& GetAddExt(String name) {
		ASSERT(data && data->mode == VFS_VALUE);
		int kind = 0;
		const std::type_info& type = typeid(T);
		for (const auto it : MetaExtFactory::List()) {
			if (*it.type == type) {
				kind = it.kind;
				break;
			}
		}
		ASSERT(kind > 0);
		/*VirtualNode n = Find(name);
		if (n.data) {
			if (n.data->mode == VFS_ENTITY) {
				auto& ent = *n.data->node;
				if (ent.ext) {
					T* o = dynamic_cast<T*>(&*ent.ext);
					if (o)
						return *o;
				}
				TODO; void* p = 0; return *(T*)p;
			}
			else if (n.data->mode == VFS_VALUE) {
				Value val = *n.data->value;
				if (val.Is<T>())
					return val.Get<T>();
				TODO; void* p = 0; return *(T*)p;
			}
		}
		else {
			
		}*/
		TODO; void* p = 0; return *(T*)p;
	}
};

class VNodeComponentCtrl;

class VirtualFSComponentCtrl : public ComponentCtrl {
	One<VNodeComponentCtrl> vnode_ctrl;
	VfsPath vnode_path;
	int data_iter = 0;
	
	bool Visit(TreeCtrl& tree, int id, VirtualNode n);
	void OnTreeCursor(TreeCtrl* tree);
	virtual bool TreeItemString(const VirtualNode& n, const Value& key, String& qtf_value) {return false;}
protected:
	friend class ValueVFSComponentCtrl;
	VirtualFSComponentCtrl();
	void DataTree(TreeCtrl& tree) override;
	VfsPath GetCursorPath() const override;
	VfsPath GetCursorRelativePath() const;
	void EditPos(JsonIO& json) override;
public:
	typedef VirtualFSComponentCtrl CLASSNAME;
	
	virtual VirtualNode Root() = 0;
	virtual VNodeComponentCtrl* CreateCtrl(const VirtualNode& vnode) = 0;
	virtual void Init() {}
	void Data() override;
	
	VirtualNode GetAdd(const VfsPath& rel_path, int kind);
	VirtualNode Find(const VfsPath& rel_path);
	VNodeComponentCtrl* GetVNodeComponentCtrl() {return vnode_ctrl ? &*vnode_ctrl : 0;}
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

class VNodeComponentCtrl : public ParentCtrl, public DatasetProvider  {
	
protected:
	friend class VirtualFSComponentCtrl;
	int kind = 0;
	VirtualNode vnode;
	ValueVFSComponentCtrl& owner;
public:
	typedef VNodeComponentCtrl CLASSNAME;
	VNodeComponentCtrl(ValueVFSComponentCtrl&, const VirtualNode& vnode);
	
	DatasetPtrs GetDataset() const override;
	DatasetPtrs RealizeEntityVfsObject(const VirtualNode& vnode, int kind);
	int GetKind() const {return kind;}
	virtual void Data() {}
	virtual void EditPos(JsonIO& json) {}
	
	VirtualNode GetVnode() const {return vnode;}
	template <class T>
	T& GetAddValue(String name) {
		ASSERT(vnode);
		T& o = vnode.template GetAddExt<T>(name);
		return o;
	}
	#define DATASET_ITEM(type, field, kind, cat, strname) \
		type& GetAdd##type() {return this->template GetAddValue<type>(#field);}
	VIRTUALNODE_DATASET_LIST
	#undef DATASET_ITEM
};

class EntityInfoCtrl : public MetaExtCtrl, public DatasetProvider {
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
	DatasetPtrs GetDataset() const override;
	
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
