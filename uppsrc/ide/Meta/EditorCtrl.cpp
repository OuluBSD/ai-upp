#include "Meta.h"

NAMESPACE_UPP


EntityEditorCtrl::EntityEditorCtrl() {
	AddMenu();
	Add(hsplit.SizePos());
	
	hsplit.Horz() << lsplit << ext_place;
	hsplit.SetPos(2000);
	lsplit.Vert() << ecs_tree << content_tree;
	lsplit.SetPos(10000*2/3);
	
	ecs_tree.SetRoot(MetaImgs::RedRing(), "Project");
	ecs_tree.WhenCursor = THISBACK(DataEcsTree);
	ecs_tree.WhenBar = [this](Bar& b) {
		if (ecs_tree.IsCursor()) {
			int cur = ecs_tree.GetCursor();
			if (cur < 0 || cur >= ecs_tree_nodes.GetCount()) return;
			VfsValue& n = *ecs_tree_nodes[cur];
			if (n.type_hash == 0) {
				b.Add("Add entity", THISBACK3(AddValue, &n, "", AsTypeHash<Entity>()));
				b.Add("Add empty", THISBACK3(AddValue, &n, "", 0));
				b.Separator();
				if (cur != 0)
					b.Add("Remove empty", THISBACK1(RemoveValue, &n));
			}
			else if (n.type_hash == AsTypeHash<Entity>()) {
				b.Add("Add Component", THISBACK(AddComponent));
				b.Separator();
				b.Add("Remove entity", THISBACK1(RemoveValue, &n));
			}
			else if (VfsValueExtFactory::FindComponent(n.type_hash) >= 0) {
				b.Add("Remove Component", THISBACK1(RemoveValue, &n));
			}
			b.Add("Move", THISBACK1(Move, &n));
		}
	};
	
}

void EntityEditorCtrl::SetExtensionCtrl(hash_t type_hash, VfsValueExtCtrl* c) {
	if (ext_ctrl) {
		ext_place.RemoveChild(&*ext_ctrl);
		ext_ctrl.Clear();
		ext_ctrl_type_hash = 0;
	}
	if (c) {
		ext_ctrl_type_hash = type_hash;
		c->owner = this;
		ext_ctrl.Attach(c);
		ext_place.Add(c->SizePos());
		PostCallback(THISBACK(DataExtCtrl));
		c->WhenEditorChange = THISBACK(DataExtCtrl);
	}
	UpdateMenu();
}

void EntityEditorCtrl::DataEcsTree_RefreshNames() {
	for(int i = 0; i < ecs_tree.GetLineCount(); i++) {
		VfsValue* n = ecs_tree_nodes[i];
		String type_str = n->GetTypeString();
		Value key = ecs_tree.Get(i);
		String val = n->id + " (" + type_str + ")";
		ecs_tree.Set(i, key, val);
	}
}

void EntityEditorCtrl::Data() {
	RealizeFileRoot();
	
	int cur = ecs_tree.IsCursor() ? ecs_tree.GetCursor() : -1;
	ecs_tree.Clear();
	
	if (!file_root) {
		ClearExtensionCtrl();
		return;
	}
	
	String key = file_root->id;
	
	const AstValue* a = *file_root;
	if (a)
		key += " (" + file_root->GetTypeString() + ")";
	
	ecs_tree.SetRoot(MetaImgs::RedRing(), key);
	ecs_tree_nodes.SetCount(1);
	ecs_tree_nodes[0] = file_root;
	
	DataEcsTreeVisit(0, *file_root);
	ecs_tree.OpenDeep(0);
	
	if (cur >= 0 && cur < ecs_tree.GetLineCount())
		ecs_tree.SetCursor(cur);
	else
		DataEcsTree();
}

void EntityEditorCtrl::DataEcsTreeVisit(int treeid, VfsValue& n) {
	ecs_tree_nodes.Reserve(ecs_tree_nodes.GetCount() + n.sub.GetCount());
	
	for(VfsValue& s : n.sub) {
		const AstValue* a = s;
		if (!a)
			continue;
		
		if (a->kind == METAKIND_ECS_SPACE) {
			String key = s.id + " (" + VfsValue::AstGetKindString(a->kind) + ")";
			int id = ecs_tree.Add(treeid, MetaImgs::RedRing(), key);
			ecs_tree_nodes.Add(&s);
			DataEcsTreeVisit(id, s);
		}
		else if (a->kind == METAKIND_ECS_ENTITY) {
			String key = s.id + " (" + VfsValue::AstGetKindString(a->kind) + ")";
			int id = ecs_tree.Add(treeid, MetaImgs::VioletRing(), key);
			ecs_tree_nodes.Add(&s);
			DataEcsTreeVisit(id, s);
		}
		else if (a->kind >= METAKIND_ECS_COMPONENT_BEGIN &&
				 a->kind <= METAKIND_ECS_COMPONENT_END) {
			String key = s.id + " (" + VfsValue::AstGetKindString(a->kind) + ")";
			int id = ecs_tree.Add(treeid, MetaImgs::BlueRing(), key);
			ecs_tree_nodes.Add(&s);
			// Don't visit component: DataEcsTreeVisit(id, s);
		}
	}
}

void EntityEditorCtrl::DataEcsTree() {
	int ecs_i = ecs_tree.GetCursor();
	
	if (!ecs_tree.IsCursor() || ecs_i >= ecs_tree_nodes.GetCount()) {
		content_tree.Clear();
		ClearExtensionCtrl();
		return;
	}
	
	auto& enode = *ecs_tree_nodes[ecs_i];
	
	if (enode.ext.IsEmpty()) {
		content_tree.Clear();
		ClearExtensionCtrl();
		return;
	}
	
	VfsValueExt& ext = *enode.ext;
	hash_t type_hash = ext.val.type_hash;
	
	if (ext_ctrl_type_hash != type_hash) {
		int fac_i = VfsValueExtFactory::FindTypeHashFactory(type_hash);
		
		if (fac_i < 0) {
			ClearExtensionCtrl();
			return;
		}
		const auto& fac = VfsValueExtFactory::List()[fac_i];
		if (fac.new_ctrl_fn) {
			VfsValueExtCtrl* ctrl = fac.new_ctrl_fn();
			ctrl->ext = &ext;
			SetExtensionCtrl(type_hash, ctrl);
			
			if (fac.type_hash == AsTypeHash<Entity>()) {
				EntityInfoCtrl& e = dynamic_cast<EntityInfoCtrl&>(*ctrl);
				e.WhenValueChange = THISBACK(DataEcsTree_RefreshNames);
			}
		}
		else {
			ClearExtensionCtrl();
			return;
		}
	}
	else {
		ASSERT(ext_ctrl);
		ext_ctrl->ext = &ext;
	}
	
	DataExtCtrl();
}

void EntityEditorCtrl::DataExtCtrl() {
	if (ext_ctrl) {
		ext_ctrl->DataTree(content_tree);
		
		// Do postponed tree cursor loading
		if (post_content_cursor >= 0 && post_content_cursor < content_tree.GetLineCount())
			content_tree.SetCursor(post_content_cursor);
		post_content_cursor = -1;
		
		ext_ctrl->Data();
	}
}

void EntityEditorCtrl::SetFont(Font fnt) {
	
}

void EntityEditorCtrl::ToolMenu(Bar& bar) {
	if (ext_ctrl)
		ext_ctrl->ToolMenu(bar);
	else
		bar.Add("", Callback());
}

void EntityEditorCtrl::Visit(Vis& v) {
	if (v.IsLoading()) {
		VfsValue* n = 0;
		IdeMetaEnv().LoadFileRootVisit(GetFileIncludes(), GetFilePath(), v, true, n);
		if (n)
			SetFileNode(n);
	}
	else {
		VfsSrcFile& file = RealizeFileRoot();
		file.MakeTempFromEnv(false);
		file.Visit(v);
		file.ClearTemp();
	}
}

/*
void EntityEditorCtrl::OnLoadDirectory(VersionControlSystem& vcs) {
	MetaEnv().LoadVCS(GetFileIncludes(), vcs);
}

void EntityEditorCtrl::OnSave(String& data, const String& filepath) {
	VfsSrcFile& file = RealizeFileRoot();
	file.MakeTempFromEnv(false);
	#if 0
	LOG("### ROOT ###");
	LOG(MetaEnv().root.GetTreeString());
	LOG("### Temp ###");
	LOG(file.temp->GetTreeString());
	#endif
	data = file.StoreJson();
}*/

VfsSrcFile& EntityEditorCtrl::RealizeFileRoot() {
	IdeMetaEnvironment& env = IdeMetaEnv();
	String path = this->GetFilePath();
	ASSERT(path.GetCount());
	VfsSrcFile& file = env.ResolveFile("", path);
	VfsSrcPkg& pkg = *file.pkg;
	ASSERT(file.id >= 0);
	VfsValue& n = env.RealizeFileNode(pkg.id, file.id, 0);
	this->file_root = &n;
	return file;
}

void EntityEditorCtrl::SelectEcsTree(VfsValue* n) {
	// the 'most high performace algorithm' (for real)
	int i = 0;
	for (auto* n0 : ecs_tree_nodes) {
		if (n0 == n) {
			ecs_tree.SetCursor(i);
			break;
		}
		i++;
	}
}

VfsValue* EntityEditorCtrl::SelectTreeValue(String title) {
	WithTreeDialog<TopWindow> dlg;
	CtrlLayoutOKCancel(dlg, title);
	for(int i = 0; i < ecs_tree.GetLineCount(); i++) {
		Value key = ecs_tree.Get(i);
		Value val = ecs_tree.GetValue(i);
		int par = ecs_tree.GetParent(i);
		Image icon = ecs_tree.GetNode(i).image;
		if (i > 0) {
			ASSERT(par >= 0);
			dlg.tree.Add(par, icon, key, val);
		}
		else {
			dlg.tree.SetRoot(icon, key, val);
		}
	}
	dlg.tree.OpenDeep(0);
	if(dlg.Execute() != IDOK || !dlg.tree.IsCursor())
		return 0;
	int cur = dlg.tree.GetCursor();
	VfsValue* tgt = this->ecs_tree_nodes[cur];
	return tgt;
}

void EntityEditorCtrl::Move(VfsValue* n) {
	if (!n || !n->owner) return;
	
	VfsValue* tgt = SelectTreeValue("Select where to move");
	if (!tgt || tgt == n->owner)
		return;
	
	
	hash_t src_type_hash = n->type_hash;
	hash_t tgt_type_hash = tgt->type_hash;
	if ((src_type_hash == 0 && tgt_type_hash != 0) ||
		(src_type_hash == AsTypeHash<Entity>() && tgt_type_hash != 0) ||
		(VfsValueExtFactory::FindComponent(src_type_hash) >= 0 && tgt_type_hash != AsTypeHash<Entity>()))
	{
		String src_kind_str = n->GetTypeString();
		String tgt_kind_str = tgt->GetTypeString();
		String err = "The parent type is not acceptable. '" + src_kind_str + "' can't have parent '" + tgt_kind_str + "'";
		PromptOK(err);
		return;
	}
	
	VfsValue& o = *n->owner;
	n = o.Detach(n);
	if (!n) return;
	tgt->Add(n);
	
	PostCallback(THISBACK(Data));
	PostCallback(THISBACK1(SelectEcsTree, n));
}

void EntityEditorCtrl::RemoveValue(VfsValue* n) {
	if (!n || !n->owner) return;
	VfsValue& o = *n->owner;
	o.Remove(n);
	PostCallback(THISBACK(Data));
	PostCallback(THISBACK1(SelectEcsTree, &o));
}

void EntityEditorCtrl::AddValue(VfsValue* n, String id, hash_t type_hash) {
	if (!n || !n->owner) return;
	if (id.IsEmpty()) {
		WString ws;
		if (!EditText(ws, "Name of the node", "Name:"))
			return;
		id = ws.ToString();
	}
	VfsValue& s = n->Add(id, type_hash);
	PostCallback(THISBACK(Data));
	PostCallback(THISBACK1(SelectEcsTree, &s));
}

void EntityEditorCtrl::AddEntity() {
	RealizeFileRoot();
	VfsValue& n = *file_root;
	Entity& e = n.Add<Entity>();
	ASSERT(e.val.type_hash == AsTypeHash<Entity>());
	auto& enode = e.val;
	enode.id = "Unnamed";
	PostCallback(THISBACK(Data));
	PostCallback(THISBACK1(SelectEcsTree, &enode));
}

void EntityEditorCtrl::RemoveEntity() {
	if (!ecs_tree.IsCursor()) return;
	VfsValue& n = *file_root;
	int ecs_i = ecs_tree.GetCursor();
	if (ecs_i >= 0 && ecs_i < n.sub.GetCount())
		n.sub.Remove(ecs_i);
	PostCallback(THISBACK(Data));
}

VfsValueExt* EntityEditorCtrl::GetSelected() {
	if (!ecs_tree.IsCursor()) return 0;
	VfsValue& n = *file_root;
	int ecs_i = ecs_tree.GetCursor();
	if (ecs_i >= 0 && ecs_i < ecs_tree_nodes.GetCount()) {
		return &*ecs_tree_nodes[ecs_i]->ext;
	}
	return 0;
}

void EntityEditorCtrl::AddComponent() {
	VfsValueExt* ext = GetSelected();
	if (!ext) return;
	Entity* e = dynamic_cast<Entity*>(ext);
	if (!e) return;
	
	String title = "Add Component";
	WithComponentSelection<TopWindow> dlg;
	CtrlLayoutOKCancel(dlg, title);
	dlg.complist.WhenLeftDouble = dlg.ok.WhenAction;
	Vector<int> list;
	auto on_group = [&dlg] {
		int idx = dlg.catgroup.GetIndex();
		int prev_value = idx >= 0 && idx < dlg.catgroup.GetCount() ? (int)dlg.catgroup.GetKey(idx) : -1;
		int g = dlg.group.GetIndex() - 1;
		bool filter_group = g >= 0;
		dlg.catgroup.Clear();
		dlg.catgroup.Add(-1,"Any");
		int new_cursor = 0;
		for(int i = 0; i < CATEGORY_GROUP_COUNT; i++) {
			if (filter_group) {
				int count = 0;
				for(const auto& factory : VfsValueExtFactory::List()) {
					int group = (int)factory.category % 2;
					if (g == group)
						count++;
				}
				if (!count)
					continue;
			}
			if (prev_value == i)
				new_cursor = dlg.catgroup.GetCount();
			dlg.catgroup.Add(i, GetCategoryGroupString(i));
		}
		dlg.catgroup.SetIndex(new_cursor);
		dlg.catgroup.WhenAction();
	};
	auto on_catgroup = [&dlg] {
		int idx = dlg.catgroup.GetIndex();
		int prev_value = idx >= 0 && idx < dlg.category.GetCount() ? (int)dlg.category.GetKey(idx) : -1;
		int g = dlg.group.GetIndex() - 1;
		bool filter_group = g >= 0;
		int cg = dlg.catgroup.GetKey(idx);
		bool filter_catgroup = cg >= 0;
		dlg.category.Clear();
		dlg.category.Add(-1,"Any");
		int new_cursor = 0;
		for(int i = 0; i < CATEGORY_COUNT; i++) {
			if (filter_group && i % 2 != g)
				continue;
			if (filter_catgroup && i / 2 != cg)
				continue;
			if (prev_value == i)
				new_cursor = dlg.category.GetCount();
			dlg.category.Add(i, GetCategoryString(i));
		}
		dlg.category.SetIndex(new_cursor);
		dlg.category.WhenAction();
	};
	auto on_filter_change = [&dlg] {
		int g = dlg.group.GetIndex() - 1;
		int cg = dlg.catgroup.GetKey(dlg.catgroup.GetIndex());
		int c = dlg.category.GetKey(dlg.category.GetIndex());
		hash_t cursor_type_hash = dlg.complist.IsCursor() ? (hash_t)(int64)dlg.complist.Get("KIND") : -1;
		dlg.complist.SetCount(0);
		auto on_row = [&](hash_t type_hash, int cat, String desc) {
			//int mod = (int)cat % 2;
			//int div = (int)cat / 2;
			/*if ((g  < 0 || mod == g) &&
				(cg < 0 || div == cg) &&
				(c  < 0 || cat == c))*/ {
				const char* grp = "TODO"; //mod == 0 ? "A":"B";
				dlg.complist.Add(
					desc,
					"TODO", //GetCategoryString(cat),
					"TODO", //GetCategoryGroupString(div),
					grp,
					TypeStringHasherIndex::ToString(type_hash));
			}
		};
		#define DATASET_ITEM(type, name, desc) on_row(AsTypeHash<type>(),0,desc);
		COMPONENT_LIST
		#undef DATASET_ITEM
		dlg.complist.SetSortColumn(0);
		if (cursor_type_hash >= 0) {
			for(int i = 0; i < dlg.complist.GetCount(); i++) {
				if ((hash_t)(int64)dlg.complist.Get(i, "TYPEHASH") == cursor_type_hash) {
					dlg.complist.SetCursor(i);
					break;
				}
			}
		}
		else if (dlg.complist.GetCount())
			dlg.complist.SetCursor(0);
	};
	dlg.complist.AddColumn("Name");
	dlg.complist.AddColumn("Category");
	dlg.complist.AddColumn("Cat. Group");
	dlg.complist.AddColumn("Sub-Group");
	dlg.complist.AddIndex("TYPEHASH");
	dlg.group.Add("Any");
	dlg.group.Add("A");
	dlg.group.Add("B");
	dlg.group.SetIndex(0);
	dlg.group.WhenAction = on_group;
	on_group();
	dlg.catgroup.SetIndex(0);
	dlg.catgroup.WhenAction = on_catgroup;
	on_catgroup();
	dlg.category.SetIndex(0);
	dlg.category.WhenAction = on_filter_change;
	on_filter_change();
	dlg.complist.SetCursor(0);
	dlg.complist.SetFocus();
	if(dlg.Execute() == IDOK) {
		hash_t type_hash = (int64)dlg.complist.Get("TYPEHASH");
		for(const auto& factory : VfsValueExtFactory::List()) {
			if (factory.type_hash != type_hash)
				continue;
			String id = factory.name;
			auto& ext = e->val.Add(id, factory.type_hash);
			ASSERT(ext.type_hash == factory.type_hash);
			PostCallback(THISBACK(Data));
			PostCallback(THISBACK1(SelectEcsTree, &e->val));
			break;
		}
	}
}

void EntityEditorCtrl::RemoveComponent() {
	VfsValueExt* ext = GetSelected();
	if (!ext) return;
	Entity* e = dynamic_cast<Entity*>(ext);
	if (!e) return;
	int ecs_i = ecs_tree.GetCursor();
	if (!ecs_i) return;
	int parent_i = ecs_tree.GetParent(ecs_i);
	int idx = ecs_tree.GetChildIndex(parent_i, ecs_i);
	if (idx >= 0 && idx < e->val.sub.GetCount())
		e->val.sub.Remove(idx);
	PostCallback(THISBACK(Data));
}

void EntityEditorCtrl::Do(int i) {
	
}

void EntityEditorCtrl::EditPos(JsonIO& json) {
	int ecs_i = ecs_tree.IsCursor() ? ecs_tree.GetCursor() : -1;
	int content_i = content_tree.IsCursor() ? content_tree.GetCursor() : -1;
	json	("ecs_tree", ecs_i)
			("content_tree", content_i)
			;
	if (json.IsLoading()) {
		if (ecs_i >= 0 && ecs_i < ecs_tree.GetLineCount())
			ecs_tree.SetCursor(ecs_i);
		
		// Content tree haven't loaded yet, so postpone loading until tree visit has finished
		post_content_cursor = content_i;
	}
	
	if (ext_ctrl)
		ext_ctrl->EditPos(json);
}


END_UPP_NAMESPACE
