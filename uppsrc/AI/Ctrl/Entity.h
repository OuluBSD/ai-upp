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
	
	static bool AcceptsExt(String e) { return e == ".ecs"; }
	static String GetID() { return "Entity Editor"; }
	static bool IsSaveDirectory() {return true;}
	
	void Do(int i);
};

END_UPP_NAMESPACE

#endif
