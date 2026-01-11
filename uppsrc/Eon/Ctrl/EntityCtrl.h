#ifndef _EonCtrl_EntityCtrl_h_
#define _EonCtrl_EntityCtrl_h_


struct EntityDataCtrl : public ComponentCtrl {
	ArrayCtrl list;
	int ent_cursor = 0;
	
	EntityDataCtrl();
	
	void UpdateData(VfsValue& v);
	void AddRow(UPP::Value key, UPP::Value value);
	
	void SetComponent(Component&) override {}
	
};

class EntityCtrl : public ParentCtrl {
	ArrayMap<TypeCls, ComponentCtrl>	comp_ctrls;
	Splitter							vsplit, hsplit;
	VfsBrowserCtrl						vfs_browser;
	VfsContentCtrl						vfs_content;
	EntityDataCtrl						vfs_data;
	ParentCtrl							vfs_cont;
	
	VfsValuePtr							selected;
	ComponentCtrl*						active_ctrl = 0;
	Engine*								mach = 0;
	
	void ClearActiveCtrl();
	void UpdateData();
	void SetDataCtrl();
	void SetComponentCtrl(Component& c);
	
public:
	typedef EntityCtrl CLASSNAME;
	EntityCtrl();
	
	void SetEngine(Engine& m) {mach = &m; vfs_browser.SetEngine(m);}
	void Updated() override;
	void OnVfsCursorChanged();
	void OnContentCursorChanged();
	
	
};


#endif
