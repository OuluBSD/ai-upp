#include "Ctrl.h"


NAMESPACE_UPP


EntityDataCtrl::EntityDataCtrl() {
	ParentCtrl::Add(list.SizePos());
	list.AddColumn("Key");
	list.AddColumn("Value");
	
}




EntityCtrl::EntityCtrl() {
	Add(hsplit.Horz().SizePos());
	
	hsplit << vsplit << vfs_cont;
	hsplit.SetPos(2000);
	
	vsplit.Vert();
	vsplit << vfs_browser << vfs_content;
	
	
	vfs_browser.WhenVfsChanged << THISBACK(OnVfsCursorChanged);
	vfs_content.WhenContentCursor << THISBACK(OnContentCursorChanged);
	
}

void EntityCtrl::OnVfsCursorChanged() {
	VfsValuePtr v = vfs_browser.GetSelected();
	if (v != selected) {
		selected = v;
		vfs_content.SetSelected(selected);
		vfs_content.Update();
	}
}

void EntityCtrl::OnContentCursorChanged() {
	if (selected) {
		VfsValuePtr v;
		vfs_content.GetCursor(v);
		
		if (v) {
			Component* c = v->FindExt<Component>();
			if (c)
				SetComponentCtrl(*c);
			else
				SetDataCtrl();
		}
		else
			SetDataCtrl();
	}
	else
		ClearActiveCtrl();
}

void EntityCtrl::Updated() {
	vfs_browser.Updated();
	vfs_content.Updated();
}

void EntityCtrl::ClearActiveCtrl() {
	if (active_ctrl) {
		vfs_cont.RemoveChild(active_ctrl);
		active_ctrl = 0;
	}
}

void EntityCtrl::SetDataCtrl() {
	ClearActiveCtrl();
	
	active_ctrl = &vfs_data;
	vfs_cont.Add(vfs_data.SizePos());
	UpdateData();
}

void EntityCtrl::SetComponentCtrl(Component& c) {
	ClearActiveCtrl();
	
	TypeCls type = c.GetTypeCls();
	int i = comp_ctrls.Find(type);
	if (i < 0) {
		active_ctrl = NewComponentCtrl(type);
		if (!active_ctrl)
			return;
		comp_ctrls.Add(type, active_ctrl);
	}
	else {
		active_ctrl = &comp_ctrls[i];
		ASSERT(active_ctrl);
	}
	
	vfs_cont.Add(active_ctrl->SizePos());
	
	active_ctrl->SetComponent(c);
	active_ctrl->Update();
}

void EntityCtrl::UpdateData() {
	if (selected)
		vfs_data.UpdateData(*selected);
}






void EntityDataCtrl::UpdateData(VfsValue& v) {
	ent_cursor = 0;
	
	AddRow("id", v.id);
	AddRow("serial", (int64)v.serial);
	AddRow("type_hash", (int64)v.type_hash);
	
	Entity* e = v.FindExt<Entity>();
	if (e) {
		AddRow("entity-idx", e->GetIdx());
		AddRow("created", e->GetCreated());
		AddRow("prefab", e->GetPrefab());
	}
	
	AddRow("sub-count", v.sub.GetCount());
	
	list.SetCount(ent_cursor);
}

void EntityDataCtrl::AddRow(UPP::Value key, UPP::Value value) {
	list.Set(ent_cursor, 0, key);
	list.Set(ent_cursor, 1, value);
	ent_cursor++;
}


END_UPP_NAMESPACE
