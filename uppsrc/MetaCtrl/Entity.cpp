#include "MetaCtrl.h"

NAMESPACE_UPP


DatasetPtrs ComponentCtrl::GetDataset() const {
	if (!ext)
		return DatasetPtrs();
	Component& comp = dynamic_cast<Component&>(*ext);
	return comp.GetDataset();
}

Script& ComponentCtrl::GetScript() {
	return *GetDataset().script; // TODO fix: unsafe
}



VirtualFSComponentCtrl::VirtualFSComponentCtrl() {
	
}

void VirtualFSComponentCtrl::Data() {
	if (!data_iter++)
		Init();
	
	// Get current VirtualNode
	VirtualNode vnode = Find(vnode_path);
	while (!vnode && vnode_path.GetPartCount() > 0) {
		vnode_path.RemoveLast();
		vnode = Find(vnode_path);
	}
	if (!vnode) {
		PromptOK("vnode error");
		return;
	}
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
	Vector<Value> parts;
	for(int i = 1; i < path.GetCount(); i++) {
		Value val = tree->Get(path[i]);
		parts << val;
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

VfsPath VirtualFSComponentCtrl::GetCursorRelativePath() const {
	return vnode_path;
}

void VirtualFSComponentCtrl::EditPos(JsonIO& json) {
	if (vnode_ctrl)
		vnode_ctrl->EditPos(json);
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
	tree.SetDisplay(QTFDisplay());
	tree.SetRoot(MetaImgs::RedRing(), name);
	Visit(tree, 0, Root());
	tree.OpenDeep(0);
	tree.SetCursor(0);
	tree.WhenCursor = THISBACK1(OnTreeCursor, &tree);
}

bool VirtualFSComponentCtrl::Visit(TreeCtrl& tree, int id, VirtualNode n) {
	auto sub = n.GetAll();
	for (VirtualNode& s : sub) {
		Value name = s.GetName();
		//DLOG("Visit " << name.GetTypeName() << ": " << name.ToString());
		int kind = s.GetKind();
		String qtf;
		if (!TreeItemString(s, name, qtf))
			qtf = DeQtf(name.ToString() + " (" + s.GetKindString() + ")");
		int sub_id = tree.Add(id, MetaImgs::BlueRing(), name, qtf);
		if (!Visit(tree, sub_id, s))
			return false;
	}
	return true;
}

VirtualNode VirtualFSComponentCtrl::Find(const VfsPath& rel_path) {
	VirtualNode p = Root();
	for(int i = 0; i < rel_path.GetPartCount(); i++) {
		Value item = rel_path.Parts()[i];
		//DLOG(i << ": " << item.GetTypeName() << ": " << item.ToString());
		p = p.Find(item);
		if (!p)
			break;
	}
	return p;
}

VirtualNode VirtualFSComponentCtrl::GetAdd(const VfsPath& rel_path, int kind) {
	VirtualNode n = Root();
	TODO;
	return n;
}




ValueVFSComponentCtrl::ValueVFSComponentCtrl() {
	
}

VirtualNode ValueVFSComponentCtrl::Root() {
	if (!root) {
		VfsPath root_path; // empty
		ValueComponentBase* base = &this->GetExt<ValueComponentBase>();
		ASSERT(base);
		auto& data = root.Create(root_path, &base->value, "Root");
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

VirtualNode VirtualNode::Find(Value name) {
	VirtualNode n;
	if (data) {
		if (data->value) {
			if (data->value->IsNull())
				*data->value = ValueMap();
			if (data->value->Is<ValueMap>()) {
				ValueMap map = *data->value;
				int i = map.Find(name);
				if (i >= 0)
					n.Create(data->path + name, &const_cast<Value&>(map.GetValue(i)), map.GetKey(i));
			}
			else TODO;
		}
		else if (data->node) {
			MetaNode& mn = *data->node;
			int i = mn.Find(name);
			if (i >= 0) {
				MetaNode& sub = mn.sub[i];
				n.Create(data->path + name, &sub);
			}
		}
		else TODO;
	}
	return n;
}

void VirtualNode::RemoveSubNodes() {
	ASSERT(data);
	if (!data) return;
	if (data->value) {
		if (data->value->Is<ValueMap>()) {
			*data->value = ValueMap();
		}
		else TODO;
	}
	else if (data->node) {
		MetaNode& mn = *data->node;
		mn.sub.Clear();
	}
	else TODO;
}

void VirtualNode::Remove(const Value& name) {
	ASSERT(data);
	if (!data) return;
	if (data->value) {
		if (data->value->Is<ValueMap>()) {
			ValueMap map = *data->value;
			map.RemoveKey(name);
			*data->value = map;
		}
		else TODO;
	}
	else if (data->node) {
		MetaNode& mn = *data->node;
		int i = mn.Find(name);
		ASSERT(i >= 0);
		if (i >= 0)
			mn.Remove(i);
	}
	else TODO;
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
							Value key = map.GetKey(i);
							auto& o = v.Add().Create(data->path + key, &val, key);
							ASSERT(o.value->Is<ValueMap>());
						}
					}
				}
				else if (data->value->Is<ValueArray>()) {
					ValueArray arr = *data->value;
					v.Reserve(arr.GetCount());
					for(int i = 0; i < arr.GetCount(); i++) {
						Value& val = arr.At(i);
						if (val.Is<ValueMap>()) {
							v.Add().Create(data->path + i, &val, i);
						}
					}
				}
				else {
					LOG(data->value->GetTypeName());
					LOG(data->value->ToString());
					TODO;
				}
			}
			else if (data->node) {
				ASSERT_(0, "only entity pointer in value based vfs");
			}
			else TODO;
		}
		else if (mode == VirtualNode::VFS_ENTITY) {
			if (data->node) {
				MetaNode& n = *data->node;
				v.Reserve(n.sub.GetCount());
				for(int i = 0; i < n.sub.GetCount(); i++) {
					auto& sub = n.sub[i];
					if (sub.id.IsEmpty())
						v.Add().Create(data->path + i, &n);
					else
						v.Add().Create(data->path + (Value)sub.id, &n);
				}
			}
			else if (data->value) {
				ASSERT_(0, "only value pointer in entity based vfs");
			}
			else TODO;
		}
		else TODO;
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

VirtualNode VirtualNode::Add(Value name, int kind) {
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
				n.Create(data->path + name, &val, name);
			}
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->node) {
				MetaNode& sub = data->node->Add(kind, name);
				n.Create(data->path + name, &sub);
			}
		}
		else TODO;
	}
	return n;
}

Value VirtualNode::GetName() const {
	if (data) {
		if (data->mode == VirtualNode::VFS_VALUE) {
			if (data->value)
				return data->key;
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->node)
				return data->node->id;
		}
		else TODO;
	}
	return Value();
}

String VirtualNode::GetKindString() const {
	if (data) {
		if (data->mode == VirtualNode::VFS_VALUE) {
			if (data->value) {
				if (data->value->Is<ValueMap>()) {
					ValueMap map = *data->value;
					int i = map.Find(".kind");
					if (i >= 0) {
						Value kind_value = map.GetValue(i);
						ASSERT(kind_value.Is<int>());
						int kind = kind_value;
						return MetaNode::GetKindString(kind);
					}
				}
			}
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->node)
				return data->node->GetKindString();
		}
		else TODO;
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
		else TODO;
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
			else TODO;
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->node)
				data->node->kind = k;
			else TODO;
		}
		else TODO;
	}
}

bool VirtualNode::IsValue() const {return data && data->mode == VFS_VALUE && data->value;}
bool VirtualNode::IsEntity() const {return data && data->mode == VFS_ENTITY && data->node;}
Value VirtualNode::GetValue() const {return data && data->mode == VFS_VALUE && data->value ? *data->value : Value();}
void VirtualNode::WriteValue(Value val) {
	ASSERT(IsValue());
	if (data && data->value)
		*data->value = val;
}

VirtualNode::operator bool() const {return data;}
void VirtualNode::Clear() {if (data) {data->Dec(); data = 0;}}
//VirtualNode::Data& VirtualNode::Create() {Clear(); data = new Data(); data->Inc(); return *data;}
VirtualNode::Data& VirtualNode::Create(const VfsPath& p, MetaNode* n)
{
	Clear();
	data = new Data();
	data->path = p;
	data->node = n;
	data->mode = VFS_ENTITY;
	data->Inc();
	return *data;
}

VirtualNode::Data& VirtualNode::Create(const VfsPath& p, Value* v, Value key)
{
	Clear();
	data = new Data();
	data->path = p;
	data->key = key;
	data->value = v;
	data->mode = VFS_VALUE;
	data->Inc();
	return *data;
}

VNodeComponentCtrl::VNodeComponentCtrl(ValueVFSComponentCtrl& o, const VirtualNode& vnode) : owner(o), vnode(vnode) {
	ASSERT(vnode);
}

DatasetPtrs VNodeComponentCtrl::RealizeEntityVfsObject(const VirtualNode& vnode, int kind) {
	DatasetPtrs p = owner.GetDataset();
	if (!p.entity)
		return p;
	
	//DUMP(vnode.data->path);
	
	for (const auto& it : MetaExtFactory::List()) {
		if (it.kind == kind) {
			VfsPath path = vnode.data->path;
			path.Add(it.name);
			int i = p.entity->objs.Find(path);
			if (i >= 0) {
				EntityData& data = p.entity->objs[i];
				MetaExtFactory::Set(p, it.kind, data);
			}
			else {
				type* o = new type();
				p.field = o;
				p.entity->objs.Add(path, o);
			}
			break;
		}
	}
	
	#define DATASET_ITEM(type,field,item_kind,d,e) \
	if (kind == item_kind) { \
		VfsPath path = vnode.data->path; \
		path.Add(#field); \
		int i = p.entity->objs.Find(path); \
		if (i >= 0) { \
			EntityData& data = p.entity->objs[i]; \
			p.field = dynamic_cast<type*>(&data); \
			ASSERT(p.field); \
		} \
		else {\
			type* o = new type(); \
			p.field = o; \
			p.entity->objs.Add(path, o); \
		} \
	}
	VIRTUALNODE_DATASET_LIST
	#undef DATASET_ITEM
	
	return p;
}

DatasetPtrs VNodeComponentCtrl::GetDataset() const {
	DatasetPtrs p = owner.GetDataset();
	
	// Get entity-vfs-objects
	if (vnode.IsValue() && p.entity) {
		const VfsPath& path = vnode.data->path;
		for(auto ed : ~p.entity->objs) {
			if (ed.key.IsLeft(path) && ed.key.GetPartCount() == path.GetPartCount()+1) {
				EntityData& data = ed.value;
				int data_kind = data.GetKind();
				switch (data_kind) {
					#define DATASET_ITEM(type,field,kind,d,e) \
					case kind: { \
						p.field = dynamic_cast<type*>(&data); \
						ASSERT(p.field);}
					VIRTUALNODE_DATASET_LIST
					#undef DATASET_ITEM
					default: break;
				}
			}
		}
	}
	
	return p;
}







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
			MetaNode& n = *ecs_tree_nodes[cur];
			if (n.kind == METAKIND_ECS_SPACE) {
				b.Add("Add entity", THISBACK3(AddNode, &n, METAKIND_ECS_ENTITY, ""));
				b.Add("Add space", THISBACK3(AddNode, &n, METAKIND_ECS_SPACE, ""));
				b.Separator();
				if (cur != 0)
					b.Add("Remove space", THISBACK1(RemoveNode, &n));
			}
			else if (n.kind == METAKIND_ECS_ENTITY) {
				b.Add("Add Component", THISBACK(AddComponent));
				b.Separator();
				b.Add("Remove entity", THISBACK1(RemoveNode, &n));
			}
			else if (n.kind >= METAKIND_ECS_COMPONENT_BEGIN &&
					 n.kind < METAKIND_ECS_COMPONENT_END) {
				b.Add("Remove Component", THISBACK1(RemoveNode, &n));
			}
			b.Add("Move", THISBACK1(MoveNode, &n));
		}
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

void EntityEditorCtrl::DataEcsTree_RefreshNames() {
	for(int i = 0; i < ecs_tree.GetLineCount(); i++) {
		MetaNode* n = ecs_tree_nodes[i];
		String kind_str = n->GetKindString();
		Value key = ecs_tree.Get(i);
		String val = n->id + " (" + kind_str + ")";
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
	
	String key = file_root->id + " (" + file_root->GetKindString() + ")";
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

void EntityEditorCtrl::DataEcsTreeVisit(int treeid, MetaNode& n) {
	ecs_tree_nodes.Reserve(ecs_tree_nodes.GetCount() + n.sub.GetCount());
	
	for(MetaNode& s : n.sub) {
		if (s.kind == METAKIND_ECS_SPACE) {
			String key = s.id + " (" + s.GetKindString() + ")";
			int id = ecs_tree.Add(treeid, MetaImgs::RedRing(), key);
			ecs_tree_nodes.Add(&s);
			DataEcsTreeVisit(id, s);
		}
		else if (s.kind == METAKIND_ECS_ENTITY) {
			String key = s.id + " (" + s.GetKindString() + ")";
			int id = ecs_tree.Add(treeid, MetaImgs::VioletRing(), key);
			ecs_tree_nodes.Add(&s);
			DataEcsTreeVisit(id, s);
		}
		else if (s.kind >= METAKIND_ECS_COMPONENT_BEGIN &&
				 s.kind <= METAKIND_ECS_COMPONENT_END) {
			String key = s.id + " (" + s.GetKindString() + ")";
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
	
	MetaNodeExt& ext = *enode.ext;
	
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
		MetaNode* n = 0;
		MetaEnv().LoadFileRootVisit(GetFileIncludes(), GetFilePath(), v, true, n);
		if (n)
			SetFileNode(n);
	}
	else {
		MetaSrcFile& file = RealizeFileRoot();
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

void EntityEditorCtrl::SelectEcsTree(MetaNode* n) {
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

MetaNode* EntityEditorCtrl::SelectTreeNode(String title) {
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
	MetaNode* tgt = this->ecs_tree_nodes[cur];
	return tgt;
}

void EntityEditorCtrl::MoveNode(MetaNode* n) {
	if (!n || !n->owner) return;
	
	MetaNode* tgt = SelectTreeNode("Select where to move");
	if (!tgt || tgt == n->owner)
		return;
	
	int src_kind = n->kind;
	int tgt_kind = tgt->kind;
	if ((src_kind == METAKIND_ECS_SPACE && tgt_kind != METAKIND_ECS_SPACE) ||
		(src_kind == METAKIND_ECS_ENTITY && tgt_kind != METAKIND_ECS_SPACE) ||
		(src_kind >= METAKIND_ECS_COMPONENT_BEGIN && src_kind <= METAKIND_ECS_COMPONENT_END && tgt_kind != METAKIND_ECS_ENTITY))
	{
		String src_kind_str = n->GetKindString();
		String tgt_kind_str = tgt->GetKindString();
		if (src_kind >= METAKIND_ECS_COMPONENT_BEGIN && src_kind <= METAKIND_ECS_COMPONENT_END)
			src_kind_str = "Component(" + src_kind_str + ")";
		if (tgt_kind >= METAKIND_ECS_COMPONENT_BEGIN && tgt_kind <= METAKIND_ECS_COMPONENT_END)
			tgt_kind_str = "Component(" + tgt_kind_str + ")";
		String err = "The parent type is not acceptable. '" + src_kind_str + "' can't have parent '" + tgt_kind_str + "'";
		PromptOK(err);
		return;
	}
	
	MetaNode& o = *n->owner;
	n = o.Detach(n);
	if (!n) return;
	tgt->Add(n);
	
	PostCallback(THISBACK(Data));
	PostCallback(THISBACK1(SelectEcsTree, n));
}

void EntityEditorCtrl::RemoveNode(MetaNode* n) {
	if (!n || !n->owner) return;
	MetaNode& o = *n->owner;
	o.Remove(n);
	PostCallback(THISBACK(Data));
	PostCallback(THISBACK1(SelectEcsTree, &o));
}

void EntityEditorCtrl::AddNode(MetaNode* n, int kind, String id) {
	if (!n || !n->owner) return;
	if (id.IsEmpty()) {
		WString ws;
		if (!EditText(ws, "Name of the node", "Name:"))
			return;
		id = ws.ToString();
	}
	MetaNode& s = n->Add(kind, id);
	PostCallback(THISBACK(Data));
	PostCallback(THISBACK1(SelectEcsTree, &s));
}

void EntityEditorCtrl::AddEntity() {
	RealizeFileRoot();
	MetaNode& n = *file_root;
	Entity& e = n.Add<Entity>();
	ASSERT(e.node.kind == METAKIND_ECS_ENTITY);
	auto& enode = e.node;
	enode.id = "Unnamed";
	PostCallback(THISBACK(Data));
	PostCallback(THISBACK1(SelectEcsTree, &enode));
}

void EntityEditorCtrl::RemoveEntity() {
	if (!ecs_tree.IsCursor()) return;
	MetaNode& n = *file_root;
	int ecs_i = ecs_tree.GetCursor();
	if (ecs_i >= 0 && ecs_i < n.sub.GetCount())
		n.sub.Remove(ecs_i);
	PostCallback(THISBACK(Data));
}

MetaNodeExt* EntityEditorCtrl::GetSelected() {
	if (!ecs_tree.IsCursor()) return 0;
	MetaNode& n = *file_root;
	int ecs_i = ecs_tree.GetCursor();
	if (ecs_i >= 0 && ecs_i < ecs_tree_nodes.GetCount()) {
		return &*ecs_tree_nodes[ecs_i]->ext;
	}
	return 0;
}

void EntityEditorCtrl::AddComponent() {
	MetaNodeExt* ext = GetSelected();
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
			PostCallback(THISBACK1(SelectEcsTree, &e->node));
			break;
		}
	}
}

void EntityEditorCtrl::RemoveComponent() {
	MetaNodeExt* ext = GetSelected();
	if (!ext) return;
	Entity* e = dynamic_cast<Entity*>(ext);
	if (!e) return;
	int ecs_i = ecs_tree.GetCursor();
	if (!ecs_i) return;
	int parent_i = ecs_tree.GetParent(ecs_i);
	int idx = ecs_tree.GetChildIndex(parent_i, ecs_i);
	if (idx >= 0 && idx < e->node.sub.GetCount())
		e->node.sub.Remove(idx);
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

DatasetPtrs EntityInfoCtrl::GetDataset() const {
	EntityInfoCtrl* e = const_cast<EntityInfoCtrl*>(this);
	DatasetPtrs p;
	MetaNode& n = e->GetNode();
	p.entity = &e->GetExt<Entity>();
	FillDataset(p, n, 0);
	return p;
}

INITIALIZER_COMPONENT_CTRL(Entity, EntityInfoCtrl)

END_UPP_NAMESPACE
