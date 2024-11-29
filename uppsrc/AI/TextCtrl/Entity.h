#ifndef _AI_TextCtrl_Entity_h_
#define _AI_TextCtrl_Entity_h_

NAMESPACE_UPP


class EntityEditorCtrl : public ToolAppCtrl {
	Splitter hsplit, lsplit;
	ArrayCtrl entlist, complist;
	Ctrl comp_place;
	
protected:
	Ptr<MetaNode> file_root;
	Vector<Ptr<Entity>> entities;
	Vector<Vector<Ptr<Component>>> components;
	
public:
	typedef EntityEditorCtrl CLASSNAME;
	EntityEditorCtrl();
	
	void Data() override;
	void SetFont(Font fnt);
	void ToolMenu(Bar& bar) override;
	MetaSrcFile& RealizeFileRoot();
	void DataEntity();
	void DataComponent();
	void SaveFile();
	
	void SetComponentCtrl(Ctrl* c);
	
	static String GetExt() { return ".ecs"; }
	static String GetID() { return "Entity Editor"; }
	
	void Do(int i);
};

END_UPP_NAMESPACE

#endif
