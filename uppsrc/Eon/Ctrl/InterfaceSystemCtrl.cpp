#include "Ctrl.h"


NAMESPACE_UPP


InterfaceDataCtrl::InterfaceDataCtrl() {
	ParentCtrl::Add(list.SizePos());
	
	list.AddColumn("Key");
	list.AddColumn("Value");
	
}


	
InterfaceSystemCtrl::InterfaceSystemCtrl() {
	Add(hsplit.Horz().SizePos());
	
	hsplit << vsplit << vfs_cont;
	hsplit.SetPos(2000);
	
	vsplit.Vert();
	vsplit << vfs_browser << iface_list;
	vsplit.SetPos(4000);
	
	vfs_browser.WhenVfsChanged << THISBACK(OnVfsCursorChanged);
	iface_list.WhenInterfaceCursor << THISBACK(OnInterfaceCursorChanged);
	
}

void InterfaceSystemCtrl::OnVfsCursorChanged() {
	VfsValuePtr v = vfs_browser.GetSelected();
	if (v != selected) {
		selected = v;
		iface_list.SetSelected(selected);
		iface_list.Update();
	}
}

void InterfaceSystemCtrl::OnInterfaceCursorChanged() {
	if (selected) {
		ComponentPtr c;
		ExchangeProviderBasePtr b;
		iface_list.GetCursor(c, b);
		
		if (c && b)
			SetInterfaceCtrl(c, b);
		else
			ClearActiveCtrl();
	}
	else
		ClearActiveCtrl();
}

void InterfaceSystemCtrl::Updated() {
	vfs_browser.Updated();
	iface_list.Updated();
}

void InterfaceSystemCtrl::ClearActiveCtrl() {
	if (active_ctrl) {
		vfs_cont.RemoveChild(active_ctrl);
		active_ctrl = 0;
	}
}

void InterfaceSystemCtrl::SetInterfaceCtrl(ComponentPtr c, ExchangeProviderBasePtr b) {
	ClearActiveCtrl();
	
	TypeCls type = b->GetTypeCls();
	int i = iface_ctrls.Find(type);
	if (i < 0) {
		active_ctrl = NewInterfaceCtrl(type);
		if (!active_ctrl)
			return;
		iface_ctrls.Add(type, active_ctrl);
	}
	else {
		active_ctrl = &iface_ctrls[i];
		ASSERT(active_ctrl);
	}
	
	vfs_cont.Add(active_ctrl->SizePos());
	
	active_ctrl->SetInterface(c, b);
	active_ctrl->Update();
}

void InterfaceDataCtrl::UpdateData(ExchangeProviderBasePtr e) {
	iface_cursor = 0;
	if (e) {
		AddRow("type", e->GetTypeCls().GetName());
		AddRow("config", e->GetConfigString());
	}
	list.SetCount(iface_cursor);
}

void InterfaceDataCtrl::AddRow(UPP::Value key, UPP::Value value) {
	list.Set(iface_cursor, 0, key);
	list.Set(iface_cursor, 1, value);
	iface_cursor++;
}


END_UPP_NAMESPACE
