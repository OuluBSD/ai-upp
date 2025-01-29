#include "Ctrl.h"

NAMESPACE_UPP


DatasetPtrs ComponentCtrl::GetDataset() {
	if (!ext)
		return DatasetPtrs();
	Component& comp = dynamic_cast<Component&>(*ext);
	return comp.GetDataset();
}

Script& ComponentCtrl::GetScript() {
	return *GetDataset().script; // TODO fix: unsafe
}

const Index<String>& ComponentCtrl::GetTypeclasses() const {
	TODO static Index<String> i; return i;
}

const Vector<ContentType>& ComponentCtrl::GetContents() const {
	TODO static Vector<ContentType> i; return i;
}

const Vector<String>& ComponentCtrl::GetContentParts() const {
	TODO static Vector<String> i; return i;
}



VirtualFSComponentCtrl::VirtualFSComponentCtrl() {
	
}

void VirtualFSComponentCtrl::Data() {
	if (!data_iter++)
		Init();
	
	// Get current VirtualNode
	VirtualNode vnode = Find(vnode_path);
	int vnode_kind = vnode.GetKind();
	
	// Check if vnode_ctrl is correct
	bool create_new = !vnode_ctrl || vnode_ctrl->kind != vnode_kind;
	if (create_new) {
		if (vnode_ctrl) {
			RemoveChild(&*vnode_ctrl);
			vnode_ctrl.Clear();
		}
		// Make new ctrl if needed
		vnode_ctrl = CreateCtrl(vnode);
		if (vnode_ctrl) {
			vnode_ctrl->kind = vnode_kind;
			Add(vnode_ctrl->SizePos());
		}
	}
	
	if (vnode_ctrl)
		vnode_ctrl->Data();
}

void VirtualFSComponentCtrl::OnTreeCursor(TreeCtrl* tree) {
	if (!tree->IsCursor()) return;
	Vector<int> path;
	int cur = tree->GetCursor();
	path << cur;
	while (cur) {
		cur = tree->GetParent(cur);
		path << cur;
	}
	Reverse(path);
	Vector<String> parts;
	for(int i = 1; i < path.GetCount(); i++) {
		Value val = tree->GetValue(path[i]);
		parts << val.ToString();
	}
	//DUMPC(parts);
	VfsPath vfspart(parts);
	this->vnode_path = vfspart;
	PostCallback(THISBACK(Data));
}

VfsPath VirtualFSComponentCtrl::GetCursorPath() const {
	VfsPath full_path(GetNode().GetPath());
	full_path.Append(vnode_path);
	return full_path;
}

void VirtualFSComponentCtrl::DataTree(TreeCtrl& tree) {
	// Fill tree with virtual-node data: visit root
	tree.WhenCursor.Clear(); // prevent useless calls
	tree.Clear();
	String name = GetExt().GetName();
	if (name.IsEmpty())
		name = GetTitle();
	if (name.IsEmpty())
		name = "This component";
	tree.SetRoot(TextImgs::RedRing(), name);
	Visit(tree, 0, Root());
	tree.OpenDeep(0);
	tree.SetCursor(0);
	tree.WhenCursor = THISBACK1(OnTreeCursor, &tree);
}

bool VirtualFSComponentCtrl::Visit(TreeCtrl& tree, int id, VirtualNode n) {
	auto sub = n.GetAll();
	for (VirtualNode& s : sub) {
		String name = s.GetName();
		int kind = s.GetKind();
		String key = name + " (" + s.GetKindString() + ")";
		int sub_id = tree.Add(id, TextImgs::BlueRing(), key, name);
		if (!Visit(tree, sub_id, s))
			return false;
	}
	return true;
}

VirtualNode VirtualFSComponentCtrl::Find(const VfsPath& rel_path) {
	VirtualNode p = Root();
	for(int i = 0; i < rel_path.GetPartCount(); i++) {
		String item = rel_path.Parts()[i];
		p = p.Find(item);
		if (!p)
			break;
	}
	return p;
}

VirtualNode VirtualFSComponentCtrl::GetAdd(const VfsPath& rel_path, int kind) {
	VirtualNode n = Root();
	Panic("TODO");
	return n;
}




ValueVFSComponentCtrl::ValueVFSComponentCtrl() {
	
}

VirtualNode ValueVFSComponentCtrl::Root() {
	if (!root) {
		ValueComponentBase* base = &this->GetExt<ValueComponentBase>();
		ASSERT(base);
		auto& data = root.Create(&base->value, "Root");
		data.node = &this->GetNode();
	}
	return root;
}

Value* ValueVFSComponentCtrl::GetValue() {
	ValueComponentBase& base = GetExt<ValueComponentBase>();
	return &base.value;
}

void ValueVFSComponentCtrl::Set(Value key, Value value) {
	Value* db = this->GetValue();
	ASSERT(db);
	if (db) {
		ValueMap map = *db;
		map.Set(key, value);
		*db = map;
	}
}

Value ValueVFSComponentCtrl::Get(Value key) {
	Value* db = this->GetValue();
	ASSERT(db);
	if (db) {
		ValueMap map = *db;
		int i = map.Find(key);
		if (i >= 0)
			return map.GetValue(i);
	}
	return Value();
}





VirtualNode::VirtualNode() {}

VirtualNode::VirtualNode(const VirtualNode& vn) : data(vn.data) {
	if (data) data->Inc();
}

VirtualNode::VirtualNode(VirtualNode&& vn) : data(vn.data) {
	vn.data = 0;
}

VirtualNode::~VirtualNode() {
	Clear();
}

VirtualNode& VirtualNode::operator=(const VirtualNode& vn) {
	Clear();
	data = vn.data;
	if (data) data->Inc();
	return *this;
}

VirtualNode VirtualNode::Find(String name) {
	VirtualNode n;
	if (data) {
		if (data->value) {
			if (data->value->IsNull())
				*data->value = ValueMap();
			if (data->value->Is<ValueMap>()) {
				ValueMap map = *data->value;
				int i = map.Find(name);
				if (i >= 0)
					n.Create(&const_cast<Value&>(map.GetValue(i)), name);
			}
			else Panic("TODO");
		}
		else if (data->node) {
			MetaNode& mn = *data->node;
			int i = mn.Find(name);
			if (i >= 0) {
				MetaNode& sub = mn.sub[i];
				n.Create(&sub);
			}
		}
		else Panic("TODO");
	}
	return n;
}

Vector<VirtualNode> VirtualNode::GetAll() {
	Vector<VirtualNode> v;
	ASSERT(data);
	if (data) {
		int mode = data->mode;
		if (mode == VirtualNode::VFS_VALUE) {
			if (data->value) {
				if (data->value->IsNull())
					*data->value = ValueMap();
				if (data->value->Is<ValueMap>()) {
					ValueMap map = *data->value;
					v.Reserve(map.GetCount());
					for(int i = 0; i < map.GetCount(); i++) {
						Value& val = const_cast<Value&>(map.GetValue(i));
						if (val.Is<ValueMap>()) {
							String key = map.GetKey(i).ToString();
							auto& data = v.Add().Create(&val, key);
							ASSERT(data.value->Is<ValueMap>());
						}
					}
				}
				else if (data->value->Is<ValueArray>()) {
					ValueArray arr = *data->value;
					v.Reserve(arr.GetCount());
					for(int i = 0; i < arr.GetCount(); i++) {
						Value& val = arr.At(i);
						if (val.Is<ValueMap>()) {
							v.Add().Create(&val, IntStr(i));
						}
					}
				}
				else {
					LOG(data->value->GetTypeName());
					LOG(data->value->ToString());
					Panic("TODO");
				}
			}
			else if (data->node) {
				ASSERT_(0, "only entity pointer in value based vfs");
			}
			else Panic("TODO");
		}
		else if (mode == VirtualNode::VFS_ENTITY) {
			if (data->node) {
				MetaNode& n = *data->node;
				v.Reserve(n.sub.GetCount());
				for(int i = 0; i < n.sub.GetCount(); i++) {
					auto& data = v.Add().Create(&n);
				}
			}
			else if (data->value) {
				ASSERT_(0, "only value pointer in entity based vfs");
			}
			else Panic("TODO");
		}
		else Panic("TODO");
	}
	return v;
}

Vector<VirtualNode> VirtualNode::FindAll(int kind) {
	// TODO optimized solution (this is lazy)
	Vector<VirtualNode> n = GetAll();
	Vector<int> rmlist;
	for(int i = 0; i < n.GetCount(); i++) {
		if (n[i].GetKind() != kind)
			rmlist << i;
	}
	if (!rmlist.IsEmpty()) n.Remove(rmlist);
	return n;
}

VirtualNode VirtualNode::Add(String name, int kind) {
	VirtualNode n;
	if (data) {
		if (data->mode == VirtualNode::VFS_VALUE) {
			if (data->value) {
				if (!data->value->Is<ValueMap>())
					*data->value = ValueMap();
				{
					ValueMap map = *data->value;
					{
						ValueMap sub_map;
						sub_map.Set(".kind", kind);
						map.Set(name, sub_map);
					}
					*data->value = map;
				}
				auto& val = data->value->GetAdd(name);
				ASSERT(val.Is<ValueMap>());
				n.Create(&val, name);
			}
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->node) {
				MetaNode& sub = data->node->Add(kind, name);
				n.Create(&sub);
			}
		}
		else Panic("TODO");
	}
	return n;
}

String VirtualNode::GetName() const {
	if (data) {
		if (data->mode == VirtualNode::VFS_VALUE) {
			if (data->value)
				return data->key;
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->node)
				return data->node->id;
		}
		else Panic("TODO");
	}
	return String();
}

String VirtualNode::GetKindString() const {
	if (data) {
		if (data->mode == VirtualNode::VFS_VALUE) {
			if (data->value) {
				if (data->value->Is<ValueMap>()) {
					ValueMap map = *data->value;
					int i = map.Find(".kind");
					if (i >= 0) {
						int kind = map.GetValue(i);
						return MetaNode::GetKindString(kind);
					}
				}
			}
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->node)
				return data->node->GetKindString();
		}
		else Panic("TODO");
	}
	return String();
}

int VirtualNode::GetKind() const {
	if (data) {
		if (data->mode == VirtualNode::VFS_VALUE) {
			if (data->value) {
				if (data->value->Is<ValueMap>()) {
					ValueMap map = *data->value;
					int i = map.Find(".kind");
					if (i >= 0)
						return (int)map.GetValue(i);
				}
				return 0;
			}
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->node)
				return data->node->kind;
		}
		else Panic("TODO");
	}
	ASSERT_(0, "no pointer");
	return 0;
}

void VirtualNode::SetKind(int k) {
	ASSERT(data);
	if (data) {
		if (data->mode == VirtualNode::VFS_VALUE) {
			if (data->value->IsNull())
				*data->value = ValueMap();
			if (data->value->Is<ValueMap>()) {
				ValueMap map = *data->value;
				map.Set(".kind", k);
				*data->value = map;
			}
			else Panic("TODO");
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->node)
				data->node->kind = k;
			else Panic("TODO");
		}
		else Panic("TODO");
	}
}

VirtualNode::operator bool() const {return data;}
void VirtualNode::Clear() {if (data) {data->Dec(); data = 0;}}
VirtualNode::Data& VirtualNode::Create() {Clear(); data = new Data(); data->Inc(); return *data;}
VirtualNode::Data& VirtualNode::Create(MetaNode* n) {Clear(); data = new Data(); data->node = n; data->mode = VFS_ENTITY; data->Inc(); return *data;}
VirtualNode::Data& VirtualNode::Create(Value* v, String key) {Clear(); data = new Data(); data->key = key; data->value = v; data->mode = VFS_VALUE; data->Inc(); return *data;}





VNodeComponentCtrl::VNodeComponentCtrl() {
	
}







EntityEditorCtrl::EntityEditorCtrl() {
	AddMenu();
	Add(hsplit.SizePos());
	
	hsplit.Horz() << lsplit << ext_place;
	hsplit.SetPos(1000);
	lsplit.Vert() << entlist << extlist << tree;
	
	entlist.AddColumn("Entity");
	entlist.AddColumn("Extensions");
	entlist.ColumnWidths("3 1");
	entlist.WhenCursor = THISBACK(DataEntity);
	entlist.WhenBar = [this](Bar& b) {
		b.Add("Add entity", THISBACK(AddEntity));
		if (entlist.IsCursor())
			b.Add("Remove entity", THISBACK(RemoveEntity));
	};
	
	extlist.AddColumn("Kind");
	extlist.AddColumn("Name");
	extlist.ColumnWidths("4 1");
	extlist.WhenCursor = THISBACK(DataExtension);
	extlist.WhenBar = [this](Bar& b) {
		b.Add("Add Component", THISBACK(AddComponent));
		if (extlist.IsCursor())
			b.Add("Remove Component", THISBACK(RemoveComponent));
	};
	
}

void EntityEditorCtrl::SetExtensionCtrl(int kind, MetaExtCtrl* c) {
	if (ext_ctrl) {
		ext_place.RemoveChild(&*ext_ctrl);
		ext_ctrl.Clear();
		ext_ctrl_kind = -1;
	}
	if (c) {
		ext_ctrl_kind = kind;
		c->owner = this;
		ext_ctrl.Attach(c);
		ext_place.Add(c->SizePos());
		PostCallback(THISBACK(DataExtCtrl));
		c->WhenEditorChange = THISBACK(DataExtCtrl);
	}
	UpdateMenu();
}

void EntityEditorCtrl::DataEntityListOnly() {
	int row = 0;
	for(int i = 0; i < entities.GetCount(); i++) {
		auto& ep = entities[i];
		if (!ep) continue;
		auto& e = *ep;
		auto& enode = e.node;
		entlist.Set(row, 0, enode.id);
		
		auto& e_exts = extensions[i];
		e_exts = e.node.GetAllExtensions();
		
		entlist.Set(row, 1, e_exts.GetCount());
		row++;
	}
}

void EntityEditorCtrl::Data() {
	RealizeFileRoot();
	
	if (!file_root) {
		entlist.Clear();
		extlist.Clear();
		ClearExtensionCtrl();
		return;
	}
	
	entities = this->file_root->FindAll<Entity>();
	
	extensions.SetCount(entities.GetCount());
	
	int row = 0;
	for(int i = 0; i < entities.GetCount(); i++) {
		auto& ep = entities[i];
		if (!ep) continue;
		auto& e = *ep;
		auto& enode = e.node;
		entlist.Set(row, 0, enode.id);
		
		auto& e_exts = extensions[i];
		e_exts = e.node.GetAllExtensions();
		
		entlist.Set(row, 1, e_exts.GetCount());
		row++;
	}
	entlist.SetCount(row);
	
	if (!entlist.IsCursor() && entlist.GetCount())
		entlist.SetCursor(0);
	else
		DataEntity();
}

void EntityEditorCtrl::DataEntity() {
	if (!entlist.IsCursor()) {
		extlist.Clear();
		ClearExtensionCtrl();
		return;
	}
	
	int ent_i = entlist.GetCursor();
	auto& e = *entities[ent_i];
	auto& exts = extensions[ent_i];
	exts = e.node.GetAllExtensions();
	exts.Insert(0, &e);
	int row = 0;
	for(int i = 0; i < exts.GetCount(); i++) {
		auto& cp = exts[i];
		if (!cp) continue;
		auto& c = *cp;
		extlist.Set(row, 0, c.node.GetKindString());
		extlist.Set(row, 1, c.GetName());
		row++;
	}
	extlist.SetCount(row);
	
	if (!extlist.IsCursor() && extlist.GetCount())
		extlist.SetCursor(0);
	else
		DataExtension();
}

void EntityEditorCtrl::DataExtension() {
	if (!entlist.IsCursor() || !extlist.IsCursor()) {
		ClearExtensionCtrl();
		return;
	}
	
	int ent_i = entlist.GetCursor();
	int ext_i = extlist.GetCursor();
	auto& e = *entities[ent_i];
	auto& exts = extensions[ent_i];
	if (ext_i < 0 || ext_i >= exts.GetCount())
		return;
	MetaNodeExt& ext = *exts[ext_i];
	
	if (ext_ctrl_kind != ext.node.kind) {
		int fac_i = MetaExtFactory::FindKindFactory(ext.node.kind);
		
		if (fac_i < 0) {
			ClearExtensionCtrl();
			return;
		}
		const auto& fac = MetaExtFactory::List()[fac_i];
		if (fac.new_ctrl_fn) {
			MetaExtCtrl* ctrl = fac.new_ctrl_fn();
			ctrl->ext = &ext;
			SetExtensionCtrl(ext.node.kind, ctrl);
			
			if (fac.kind == METAKIND_ECS_ENTITY) {
				EntityInfoCtrl& e = dynamic_cast<EntityInfoCtrl&>(*ctrl);
				e.WhenValueChange = THISBACK(DataEntityListOnly);
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
		ext_ctrl->DataTree(tree);
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

void EntityEditorCtrl::Visit(NodeVisitor& vis) {
	if (vis.IsLoading())
		MetaEnv().LoadFileRootVisit(GetFileIncludes(), GetFilePath(), vis, true);
	else {
		MetaSrcFile& file = RealizeFileRoot();
		file.MakeTempFromEnv(false);
		file.Visit(vis);
		file.ClearTemp();
	}
}

/*
void EntityEditorCtrl::OnLoadDirectory(VersionControlSystem& vcs) {
	MetaEnv().LoadVCS(GetFileIncludes(), vcs);
}

void EntityEditorCtrl::OnSave(String& data, const String& filepath) {
	MetaSrcFile& file = RealizeFileRoot();
	file.MakeTempFromEnv(false);
	#if 0
	LOG("### ROOT ###");
	LOG(MetaEnv().root.GetTreeString());
	LOG("### Temp ###");
	LOG(file.temp->GetTreeString());
	#endif
	data = file.StoreJson();
}*/

MetaSrcFile& EntityEditorCtrl::RealizeFileRoot() {
	MetaEnvironment& env = MetaEnv();
	String path = this->GetFilePath();
	ASSERT(path.GetCount());
	MetaSrcFile& file = env.ResolveFile("", path);
	MetaSrcPkg& pkg = *file.pkg;
	ASSERT(file.id >= 0);
	MetaNode& n = env.RealizeFileNode(pkg.id, file.id, METAKIND_ECS_SPACE);
	this->file_root = &n;
	return file;
}

void EntityEditorCtrl::AddEntity() {
	RealizeFileRoot();
	MetaNode& n = *file_root;
	Entity& e = n.Add<Entity>();
	ASSERT(e.node.kind == METAKIND_ECS_ENTITY);
	auto& enode = e.node;
	enode.id = "Unnamed";
	PostCallback(THISBACK(Data));
	PostCallback([this]{entlist.SetCursor(entlist.GetCount()-1);}); // select last entity
}

void EntityEditorCtrl::RemoveEntity() {
	if (!entlist.IsCursor()) return;
	MetaNode& n = *file_root;
	int ent_i = entlist.GetCursor();
	if (ent_i >= 0 && ent_i < n.sub.GetCount())
		n.sub.Remove(ent_i);
	PostCallback(THISBACK(Data));
}

Entity* EntityEditorCtrl::GetSelectedEntity() {
	if (!entlist.IsCursor()) return 0;
	MetaNode& n = *file_root;
	int ent_i = entlist.GetCursor();
	if (ent_i >= 0 && ent_i < entities.GetCount()) {
		return entities[ent_i];
	}
	return 0;
}

void EntityEditorCtrl::AddComponent() {
	Entity* e = GetSelectedEntity();
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
				for(const auto& factory : MetaExtFactory::List()) {
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
		int cursor_kind = dlg.complist.IsCursor() ? (int)dlg.complist.Get("KIND") : -1;
		dlg.complist.SetCount(0);
		auto on_row = [&](int kind, int cat, String desc) {
			int mod = (int)cat % 2;
			int div = (int)cat / 2;
			if ((g  < 0 || mod == g) &&
				(cg < 0 || div == cg) &&
				(c  < 0 || cat == c)) {
				const char* grp = mod == 0 ? "A":"B";
				dlg.complist.Add(desc, GetCategoryString(cat), GetCategoryGroupString(div), grp, kind);
			}
		};
		#define DATASET_ITEM(type, name, kind, cat, desc) on_row(kind,cat,desc);
		COMPONENT_LIST
		#undef DATASET_ITEM
		dlg.complist.SetSortColumn(0);
		if (cursor_kind >= 0) {
			for(int i = 0; i < dlg.complist.GetCount(); i++) {
				if (dlg.complist.Get(i, "KIND") == cursor_kind) {
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
	dlg.complist.AddIndex("KIND");
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
		int kind = dlg.complist.Get("KIND");
		for(const auto& factory : MetaExtFactory::List()) {
			if (factory.kind != kind) continue;
			auto& ext = e->node.Add(factory.kind);
			ASSERT(ext.kind == factory.kind);
			PostCallback(THISBACK(Data));
			PostCallback([this]{extlist.SetCursor(extlist.GetCount()-1);}); // select last extension (component)
			break;
		}
	}
}

void EntityEditorCtrl::RemoveComponent() {
	Entity* e = GetSelectedEntity();
	if (!e || !extlist.IsCursor()) return;
	int ext_i = extlist.GetCursor() - 1; // -1 --> don't remove EntityInfoCtrl
	if (ext_i >= 0 && ext_i < e->node.sub.GetCount())
		e->node.sub.Remove(ext_i);
	PostCallback(THISBACK(Data));
}

void EntityEditorCtrl::Do(int i) {
	
	//DatasetPtrs p = GetDataset();
	//p.file_root = file_root;
	
	if (i == 0) {
		
		
		
	}
	
}

void EntityEditorCtrl::EditPos(JsonIO& json) {
	int ent_i = entlist.IsCursor() ? entlist.GetCursor() : -1;
	int ext_i = extlist.IsCursor() ? extlist.GetCursor() : -1;
	json	("entity", ent_i)
			("ext", ext_i)
			;
	if (json.IsLoading()) {
		if (ent_i >= 0 && ent_i < entlist.GetCount())
			entlist.SetCursor(ent_i);
		
		if (ext_i >= 0 && ext_i < extlist.GetCount())
			extlist.SetCursor(ext_i);
	}
	
	if (ext_ctrl)
		ext_ctrl->EditPos(json);
}










EntityInfoCtrl::EntityInfoCtrl() {
	CtrlLayout(info);
	Add(info.SizePos());
	info.split.Horz() << data << value;
	info.split.SetPos(7500);
	
	data.AddColumn("Key");
	data.AddColumn("Value");
	data.AddIndex("IDX");
	data.ColumnWidths("1 4");
	data.WhenCursor = THISBACK(DataCursor);
	
	info.name.WhenAction = THISBACK(OnEdit);
	info.show_hidden_values.WhenAction = THISBACK(Data);
	value.WhenAction = THISBACK(OnEditValue);
}

void EntityInfoCtrl::Data() {
	Entity& ent = GetExt<Entity>();
	MetaNode& enode = ent.node;
	
	info.name = enode.id;
	
	// Realize some default fields
	ent.Data("description");
	ent.Data("gender");
	
	Vector<MetaNode*> envs = MetaEnv().FindAllEnvs();
	
	// Get all contexts
	all_ctxs.Clear();
	for (MetaNode* env : envs) {
		Vector<MetaNode*> ctxs = env->FindAllShallow(METAKIND_CONTEXT);
		for (MetaNode* ctx : ctxs) {
			String key = /*env->id + ": " +*/ ctx->id;
			all_ctxs.Add(key, ctx);
		}
	}
	
	// Entity's context
	String ent_ctx = ent.Data("ctx");
	String match_key = ent_ctx;
	
	
	// Entity data fields
	data.SetCount(0);
	bool show_hidden_values = info.show_hidden_values.Get();
	int row = 0;
	for(int i = 0; i < ent.data.GetCount(); i++) {
		String key = ent.data.GetKey(i);
		if (!show_hidden_values && key.GetCount() && key[0] == '.')
			continue;
		Value value = ent.data[i];
		data.Set(row, "IDX", i);
		data.Set(row, 0, key);
		if (key == "gender") {
			DropList* dl = new DropList;
			for (auto g : GetCategories())
				dl->Add(g);
			data.SetCtrl(row, 1, dl);
			int gender_i = max(0, FindCategory(value.ToString()));
			dl->SetIndex(gender_i);
			dl->WhenAction = [&ent,dl]{
				ent.data.GetAdd("gender") = GetCategoryString(dl->GetIndex());
			};
		}
		else if (key == "ctx") {
			DropList* dl = new DropList;
			data.SetCtrl(row, 1, dl);
			dl->Clear();
			dl->Add("");
			int active_ctx = 0;
			for(int i = 0; i < all_ctxs.GetCount(); i++) {
				String ctx_key = all_ctxs.GetKey(i);
				dl->Add(ctx_key);
				if (ctx_key == match_key)
					active_ctx = 1+i;
			}
			if (active_ctx >= 0 && active_ctx < dl->GetCount()) {
				dl->SetIndex(active_ctx);
			}
			dl->WhenAction = [this,&ent,dl]{
				int ctx_i = dl->GetIndex()-1;
				if (ctx_i >= 0 && ctx_i < dl->GetCount()) {
					MetaNode& ctx = *all_ctxs[ctx_i];
					ent.Data("ctx") = ctx.id;
				}
				else {
					ent.Data("ctx") = String();
				}
			};
		}
		else {
			data.Set(row, 1, value);
		}
		row++;
	}
	data.SetCount(row);
	data.SetSortColumn(0);
	
	if (!data.IsCursor())
		data.SetCursor(0);
	else
		DataCursor();
}

void EntityInfoCtrl::DataCursor() {
	if (!data.IsCursor()) {
		value.Clear();
		return;
	}
	
	Entity& ent = GetExt<Entity>();
	int data_i = data.Get("IDX");
	Value val = ent.data[data_i];
	value.SetData(val.ToString());
}

void EntityInfoCtrl::OnEdit() {
	Entity& e = GetExt<Entity>();
	auto& enode = e.node;
	enode.id = ~info.name;
		
	WhenValueChange();
}

void EntityInfoCtrl::OnEditValue() {
	if (!data.IsCursor())
		return;
	Entity& e = GetExt<Entity>();
	int data_i = data.Get("IDX");
	Value val = value.GetData();
	e.data[data_i] = val.ToString();
	data.Set(1, val);
}

void EntityInfoCtrl::ToolMenu(Bar& bar) {
	
}

DatasetPtrs EntityInfoCtrl::GetDataset() {
	DatasetPtrs p;
	MetaNode& n = GetNode();
	p.entity = &GetExt<Entity>();
	FillDataset(p, n, 0);
	return p;
}

INITIALIZER_COMPONENT_CTRL(Entity, EntityInfoCtrl)

END_UPP_NAMESPACE
