#ifndef _EonCtrl_InterfaceSystemCtrl_h_
#define _EonCtrl_InterfaceSystemCtrl_h_


struct InterfaceDataCtrl : public InterfaceCtrl {
	ArrayCtrl list;
	int iface_cursor = 0;
	
	InterfaceDataCtrl();
	
	void UpdateData(ExchangeProviderBasePtr e);
	void AddRow(UPP::Value key, UPP::Value value);
	
	void SetInterface(ComponentPtr, ExchangeProviderBasePtr) override {}
	
};

class InterfaceSystemCtrl : public ParentCtrl {
	ArrayMap<TypeCls, InterfaceCtrl>	iface_ctrls;
	Splitter							vsplit, hsplit;
	VfsBrowserCtrl						vfs_browser;
	InterfaceListCtrl					iface_list;
	InterfaceDataCtrl					iface_data;
	ParentCtrl							vfs_cont;
	
	VectorMap<int, int>					node_ifaces;
	VfsValuePtr							selected;
	ExchangeProviderBasePtr				sel_iface;
	InterfaceCtrl*						active_ctrl = 0;
	
	Image ent_icon, comp_icon, iface_icon;
	
	void OnVfsCursorChanged();
	void OnInterfaceCursorChanged();
	void ClearActiveCtrl();
	void SetInterfaceCtrl(ComponentPtr c, ExchangeProviderBasePtr b);
	
public:
	typedef InterfaceSystemCtrl CLASSNAME;
	InterfaceSystemCtrl();
	
	void Updated() override;
	void SetEngine(Engine& m) {vfs_browser.SetEngine(m);}
	
};


#endif
