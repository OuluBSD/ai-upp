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
	VirtualNode() {}
	~VirtualNode() {Clear();}
	Vector<VirtualNode> GetAll();
	Vector<VirtualNode> FindAll(int kind);
	String GetName() const;
	String GetKindString() const;
	int GetKind() const;
	void SetKind(int k);
	operator bool() const {return data;}
	void Clear() {if (data) {data->Dec(); data = 0;}}
	Data& Create() {Clear(); data = new Data(); data->Inc(); return *data;}
	Data& Create(MetaNode* n) {Clear(); data = new Data(); data->node = n; data->Inc(); return *data;}
	Data& Create(Value* v) {Clear(); data = new Data(); data->value = v; data->Inc(); return *data;}
};

class VirtualFSComponentCtrl : public ComponentCtrl {
	Splitter hsplit;
	TreeCtrl tree;
	Ctrl placeholder;
	
	void Data() override;
	void RefreshTree();
	bool Visit(int id, VirtualNode& n);
protected:
	friend class ValueVFSComponentCtrl;
	VirtualFSComponentCtrl();
	
	
public:
	typedef VirtualFSComponentCtrl CLASSNAME;
	
	virtual void VirtualData() = 0;
	virtual VirtualNode& Root() = 0;
	virtual void Init() {}
	
	VirtualNode& GetAdd(String rel_path, int kind);
};

class ValueVFSComponentCtrl : public VirtualFSComponentCtrl {
	VirtualNode root;
public:
	typedef ValueVFSComponentCtrl CLASSNAME;
	ValueVFSComponentCtrl();
	VirtualNode& Root() override;
};

class VNodeComponentCtrl : public ParentCtrl {
	
public:
	typedef VNodeComponentCtrl CLASSNAME;
	VNodeComponentCtrl();
	
	virtual void Data();
	
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
	ArrayCtrl entlist, extlist;
	Ctrl ext_place;
	One<MetaExtCtrl> ext_ctrl;
	int ext_ctrl_kind = -1;
	
protected:
	Ptr<MetaNode> file_root;
	Vector<Ptr<Entity>> entities;
	Vector<Vector<Ptr<MetaNodeExt>>> extensions;
	
public:
	typedef EntityEditorCtrl CLASSNAME;
	EntityEditorCtrl();
	
	void Data() override;
	void SetFont(Font fnt);
	void ToolMenu(Bar& bar) override;
	MetaSrcFile& RealizeFileRoot();
	void DataEntityListOnly();
	void DataEntity();
	void DataExtension();
	void DataExtCtrl();
	void Visit(NodeVisitor& vis) override;
	void AddEntity();
	void RemoveEntity();
	void AddComponent();
	void RemoveComponent();
	void SetExtensionCtrl(int kind, MetaExtCtrl* ctrl);
	void ClearExtensionCtrl() {SetExtensionCtrl(-1,0);}
	Entity* GetSelectedEntity();
	void EditPos(JsonIO& jio) override;
	
	static bool AcceptsExt(String e) { return e == ".ecs"; }
	static String GetID() { return "Entity Editor"; }
	
	void Do(int i);
};

END_UPP_NAMESPACE

#endif
