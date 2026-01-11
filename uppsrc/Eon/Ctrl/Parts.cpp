#include "Ctrl.h"

NAMESPACE_UPP

VfsTreeCtrl::VfsTreeCtrl() {
	Add(tree.SizePos());
	tree.WhenCursor << THISBACK(OnCursor);
}

void VfsTreeCtrl::SetEngine(Engine& m) {
	mach = &m;
}

void VfsTreeCtrl::OnCursor() {
	if (!tree.IsCursor())
		return;
	
	int cursor = tree.GetCursor();
	UPP::Value val = tree.Get(cursor);
	
	if (val.Is<VfsPath>()) {
		VfsPath new_sel = val.To<VfsPath>();
		
		if (new_sel != selected) {
			selected = new_sel;
			WhenVfsChanged();
		}
	}
}

hash_t VfsTreeCtrl::GetPathTreeHash() const {
	hash_t h = 0;
	if (mach) {
		h = mach->GetRootPool().GetTotalHash();
	}
	return h;
}

void VfsTreeCtrl::Updated() {
	hash_t h = GetPathTreeHash();
	if (h == last_hash)
		ClearModify();
	else {
		last_hash = h;
		SetModify();
	}
	
	if (IsModified()) {
		ClearModify();
		Data();
	}
}

void VfsTreeCtrl::Data() {
	if (!mach) return;
	VfsValue& root = mach->GetRootPool();
	
	String root_name = root.id;
	if (root_name.IsEmpty())
		root_name = "Pools";
	tree.Clear();
	tree.SetRoot(CtrlImg::Dir(), RawToValue(root.GetPath()), root_name);
	AddPath(0, root);
	tree.OpenDeep(0);
	
	if (!tree.IsCursor())
		tree.SetCursor(0);
}

void VfsTreeCtrl::AddPath(int parent, VfsValue& parent_vfs) {
	int idx = 0;
	for (VfsValue& p : parent_vfs.sub) {
		String name = p.id;
		if (name.IsEmpty())
			name = IntStr64(idx);
		int i = tree.Add(parent, CtrlImg::Dir(), RawToValue(p.GetPath()), name);
		AddPath(i, p);
		idx++;
	}
}









VfsListCtrl::VfsListCtrl() {
	ParentCtrl::Add(list.SizePos());
	
	list.AddIndex();
	list.AddColumn("Created/Serial");
	list.AddColumn("Prefab/Type");
	list.AddColumn("Id/Path");
	list.ColumnWidths("1 2 1");
	
	list.WhenCursor << THISBACK(OnCursor);
	
}

void VfsListCtrl::SetScope(VfsPath path) {
	this->scope = path;
}

VfsValue* VfsListCtrl::GetCurrent() const {
	VfsValue* p = mach ? mach->GetRootPool().FindPath(scope) : 0;
	return p;
}

hash_t VfsListCtrl::GetVfsListHash() const {
	hash_t v = 0;
	VfsValue* p = GetCurrent();
	if (p) {
		v = p->serial; // Just shallow serial for the list itself
	}
	return v;
}

void VfsListCtrl::Updated() {
	hash_t h = GetVfsListHash();
	if (h == last_hash)
		ClearModify();
	else {
		last_hash = h;
		SetModify();
	}
	
	if (IsModified()) {
		ClearModify();
		Data();
	}
}

void VfsListCtrl::OnCursor() {
	VfsValue* p = GetCurrent();
	if (!p || !list.IsCursor())
		return;
	
	int cursor = list.GetCursor();
	int vfs_i = list.Get(cursor, 0);
	
	VfsValuePtr new_sel;
	
	if (vfs_i >= 0 && vfs_i < p->sub.GetCount())
		new_sel =  &p->sub.At(vfs_i);
	
	if (new_sel != selected) {
		selected = new_sel;
		WhenVfsChanged();
	}
}

void VfsListCtrl::Data() {
	VfsValue* pool = GetCurrent();
	if (!pool)
		return;
	
	int cursor = -1;
	int i = 0;
	for (VfsValue& v : pool->sub) {
		if (selected == &v)
			cursor = i;
		list.Set(i, 0, i);
		Entity* e = v.FindExt<Entity>();
		if (e) {
			list.Set(i, 1, e->GetCreated());
			list.Set(i, 2, e->GetPrefab());
			list.Set(i, 3, v.GetPath().ToString());
		}
		else {
			list.Set(i, 1, (int64)v.serial);
			list.Set(i, 2, v.GetTypeString());
			list.Set(i, 3, v.id.IsEmpty() ? v.GetPath().ToString() : v.id);
		}
		i++;
	}
	list.SetCount(pool->sub.GetCount());
	
	if (!list.IsCursor() && list.GetCount())
		list.SetCursor(0);
	else if (cursor >= 0 && list.GetCursor() != cursor)
		list.SetCursor(cursor);
}








VfsContentCtrl::VfsContentCtrl() {
	ParentCtrl::Add(tree.SizePos());
	
	tree.WhenCursor << THISBACK(OnCursor);
	
	ent_icon	= StreamRaster::LoadFileAny(ShareDirFile("imgs" DIR_SEPS "icons" DIR_SEPS "package.png"));
	comp_icon	= StreamRaster::LoadFileAny(ShareDirFile("imgs" DIR_SEPS "icons" DIR_SEPS "component.png"));
	iface_icon	= StreamRaster::LoadFileAny(ShareDirFile("imgs" DIR_SEPS "icons" DIR_SEPS "interface.png"));
	vfs_icon	= CtrlImg::File();
}

void VfsContentCtrl::Updated() {
	if (!selected)
		return;
	
	if (!IsModified()) {
		int64 time = (int64)selected->serial;
		if (time != vfs_changed_time) {
			vfs_changed_time = time;
			SetModify();
		}
	}
	
	if (IsModified()) {
		ClearModify();
		
		tree.Clear();
		
		String name = selected->id;
		Entity* e = selected->FindExt<Entity>();
		if (e) {
			if (name.IsEmpty()) name = e->GetPrefab();
			if (name.IsEmpty())	name = "Entity";
			tree.SetRoot(ent_icon, RawToValue(selected), name);
		}
		else {
			if (name.IsEmpty()) name = selected->GetTypeString();
			if (name.IsEmpty()) name = "VfsValue";
			tree.SetRoot(vfs_icon, RawToValue(selected), name);
		}
		
		AddTreeVfs(0, *selected);
		tree.OpenDeep(0);
		tree.SetCursor(0);
		
		WhenContentCursor();
	}
}

void VfsContentCtrl::OnCursor() {
	if (!selected || !tree.IsCursor())
		return;
	
	WhenContentCursor();
}

void VfsContentCtrl::AddTreeVfs(int tree_i, VfsValue& v) {
	int i = 0;
	for (VfsValue& sub : v.sub) {
		Image icon = vfs_icon;
		String name = sub.id;
		if (sub.FindExt<Entity>()) icon = ent_icon;
		else if (sub.FindExt<Component>()) icon = comp_icon;
		
		if (name.IsEmpty()) name = sub.GetTypeString();
		if (name.IsEmpty()) name = "[" + IntStr(i) + "]";
		
		int node_i = tree.Add(tree_i, icon, RawToValue(VfsValuePtr(&sub)), name);
		AddTreeVfs(node_i, sub);
		i++;
	}
}

void VfsContentCtrl::GetCursor(VfsValuePtr& v) {
	v = nullptr;
	
	if (tree.IsCursor()) {
		Value val = tree.Get(tree.GetCursor());
		if (val.Is<VfsValuePtr>())
			v = val.To<VfsValuePtr>();
	}
}











VfsBrowserCtrl::VfsBrowserCtrl() {
	vfs_list.WhenVfsChanged = WhenVfsChanged.Proxy();
	
	Add(vsplit.SizePos());
	
	vsplit.Vert();
	vsplit << vfs_tree << vfs_list;
	
	vfs_tree.WhenVfsChanged << THISBACK(OnVfsCursorChanged);
}

void VfsBrowserCtrl::Updated() {
	vfs_tree.Updated();
	vfs_list.Updated();
}

void VfsBrowserCtrl::Data() {
	vfs_tree.Data();
	vfs_list.Data();
}

void VfsBrowserCtrl::OnVfsCursorChanged() {
	VfsPath path = vfs_tree.GetSelected();
	if (path != scope) {
		scope = path;
		vfs_list.SetScope(scope);
		vfs_list.Update();
	}
}






END_UPP_NAMESPACE
