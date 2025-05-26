#include "MetaCtrl.h"
#include <ide/Meta/Meta.h>

NAMESPACE_UPP


DatasetPtrs ComponentCtrl::GetDataset() const {
	if (!ext)
		return DatasetPtrs();
	Component& comp = dynamic_cast<Component&>(*ext);
	return comp.GetDataset();
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
	int vnode_type_hash = vnode.GetTypeHash();
	
	// Check if vnode_ctrl is correct
	bool create_new = !vnode_ctrl || vnode_ctrl->type_hash != vnode_type_hash;
	if (create_new) {
		if (vnode_ctrl) {
			RemoveChild(&*vnode_ctrl);
			vnode_ctrl.Clear();
		}
		// Make new ctrl if needed
		vnode_ctrl = CreateCtrl(vnode);
		if (vnode_ctrl) {
			vnode_ctrl->type_hash = vnode_type_hash;
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
	VfsPath full_path(GetValue().GetPath());
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
		hash_t type_hash = s.GetTypeHash();
		String qtf;
		if (!TreeItemString(s, name, qtf))
			qtf = DeQtf(name.ToString() + " (" + TypeStringHasherIndex::ToString(type_hash) + ")");
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

#if 0
VirtualNode VirtualFSComponentCtrl::GetAdd(const VfsPath& rel_path, hash_t type_hash) {
	VirtualNode n = Root();
	TODO;
	return n;
}
#endif



ValueVFSComponentCtrl::ValueVFSComponentCtrl() {
	
}

VirtualNode ValueVFSComponentCtrl::Root() {
	if (!root) {
		VfsPath root_path; // empty
		ValueComponentBase* base = &this->GetExt<ValueComponentBase>();
		ASSERT(base);
		auto& data = root.Create(root_path, &base->value, "Root");
		data.vfs_value = &this->GetValue();
	}
	return root;
}

Value* ValueVFSComponentCtrl::GetPolyValue() {
	ValueComponentBase& base = GetExt<ValueComponentBase>();
	return &base.value;
}

void ValueVFSComponentCtrl::Set(Value key, Value value) {
	Value* db = this->GetPolyValue();
	ASSERT(db);
	if (db) {
		ValueMap map = *db;
		map.Set(key, value);
		*db = map;
	}
}

Value ValueVFSComponentCtrl::Get(Value key) {
	Value* db = this->GetPolyValue();
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
		if (data->poly_value) {
			if (data->poly_value->IsNull())
				*data->poly_value = ValueMap();
			if (data->poly_value->Is<ValueMap>()) {
				ValueMap map = *data->poly_value;
				int i = map.Find(name);
				if (i >= 0)
					n.Create(data->path + name, &const_cast<Value&>(map.GetValue(i)), map.GetKey(i));
			}
			else TODO;
		}
		else if (data->vfs_value) {
			VfsValue& mn = *data->vfs_value;
			int i = mn.Find(name);
			if (i >= 0) {
				VfsValue& sub = mn.sub[i];
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
	if (data->poly_value) {
		if (data->poly_value->Is<ValueMap>()) {
			*data->poly_value = ValueMap();
		}
		else TODO;
	}
	else if (data->vfs_value) {
		VfsValue& mn = *data->vfs_value;
		mn.sub.Clear();
	}
	else TODO;
}

void VirtualNode::Remove(const Value& name) {
	ASSERT(data);
	if (!data) return;
	if (data->poly_value) {
		if (data->poly_value->Is<ValueMap>()) {
			ValueMap map = *data->poly_value;
			map.RemoveKey(name);
			*data->poly_value = map;
		}
		else TODO;
	}
	else if (data->vfs_value) {
		VfsValue& mn = *data->vfs_value;
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
			if (data->poly_value) {
				if (data->poly_value->IsNull())
					*data->poly_value = ValueMap();
				if (data->poly_value->Is<ValueMap>()) {
					ValueMap map = *data->poly_value;
					v.Reserve(map.GetCount());
					for(int i = 0; i < map.GetCount(); i++) {
						Value& val = const_cast<Value&>(map.GetValue(i));
						if (val.Is<ValueMap>()) {
							Value key = map.GetKey(i);
							auto& o = v.Add().Create(data->path + key, &val, key);
							ASSERT(o.poly_value->Is<ValueMap>());
						}
					}
				}
				else if (data->poly_value->Is<ValueArray>()) {
					ValueArray arr = *data->poly_value;
					v.Reserve(arr.GetCount());
					for(int i = 0; i < arr.GetCount(); i++) {
						Value& val = arr.At(i);
						if (val.Is<ValueMap>()) {
							v.Add().Create(data->path + i, &val, i);
						}
					}
				}
				else {
					LOG(data->poly_value->GetTypeName());
					LOG(data->poly_value->ToString());
					TODO;
				}
			}
			else if (data->vfs_value) {
				ASSERT_(0, "only entity pointer in value based vfs");
			}
			else TODO;
		}
		else if (mode == VirtualNode::VFS_ENTITY) {
			if (data->vfs_value) {
				VfsValue& n = *data->vfs_value;
				v.Reserve(n.sub.GetCount());
				for(int i = 0; i < n.sub.GetCount(); i++) {
					auto& sub = n.sub[i];
					if (sub.id.IsEmpty())
						v.Add().Create(data->path + i, &n);
					else
						v.Add().Create(data->path + (Value)sub.id, &n);
				}
			}
			else if (data->poly_value) {
				ASSERT_(0, "only value pointer in entity based vfs");
			}
			else TODO;
		}
		else TODO;
	}
	return v;
}

Vector<VirtualNode> VirtualNode::FindAll(hash_t type_hash) {
	// TODO optimized solution (this is lazy)
	Vector<VirtualNode> n = GetAll();
	Vector<int> rmlist;
	for(int i = 0; i < n.GetCount(); i++) {
		if (n[i].GetTypeHash() != type_hash)
			rmlist << i;
	}
	if (!rmlist.IsEmpty()) n.Remove(rmlist);
	return n;
}

VirtualNode VirtualNode::Add(Value name, hash_t type_hash) {
	VirtualNode n;
	if (data) {
		if (data->mode == VirtualNode::VFS_VALUE) {
			if (data->poly_value) {
				if (!data->poly_value->Is<ValueMap>())
					*data->poly_value = ValueMap();
				{
					ValueMap map = *data->poly_value;
					{
						ValueMap sub_map;
						sub_map.Set(".type_hash", (int64)type_hash);
						map.Set(name, sub_map);
					}
					*data->poly_value = map;
				}
				auto& val = data->poly_value->GetAdd(name);
				ASSERT(val.Is<ValueMap>());
				n.Create(data->path + name, &val, name);
			}
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->vfs_value) {
				VfsValue& sub = data->vfs_value->Add(name, type_hash);
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
			if (data->poly_value)
				return data->key;
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->vfs_value)
				return data->vfs_value->id;
		}
		else TODO;
	}
	return Value();
}

String VirtualNode::GetTypeString() const {
	if (data) {
		if (data->mode == VirtualNode::VFS_VALUE) {
			if (data->poly_value) {
				if (data->poly_value->Is<ValueMap>()) {
					ValueMap map = *data->poly_value;
					int i = map.Find(".type_hash");
					if (i >= 0) {
						Value type_hash_value = map.GetValue(i);
						ASSERT(type_hash_value.Is<int64>());
						hash_t type_hash = (hash_t)(int64)type_hash_value;
						return TypeStringHasherIndex::ToString(type_hash);
					}
				}
			}
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->vfs_value)
				return data->vfs_value->AstGetKindString();
		}
		else TODO;
	}
	return String();
}

hash_t VirtualNode::GetTypeHash() const {
	if (data) {
		if (data->mode == VirtualNode::VFS_VALUE) {
			if (data->poly_value) {
				if (data->poly_value->Is<ValueMap>()) {
					ValueMap map = *data->poly_value;
					int i = map.Find(".type_hash");
					if (i >= 0)
						return (hash_t)(int64)map.GetValue(i);
				}
				return 0;
			}
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->vfs_value)
				return data->vfs_value->type_hash;
		}
		else TODO;
	}
	ASSERT_(0, "no pointer");
	return 0;
}

void VirtualNode::SetType(hash_t type_hash) {
	ASSERT(data);
	if (data) {
		if (data->mode == VirtualNode::VFS_VALUE) {
			if (data->poly_value->IsNull())
				*data->poly_value = ValueMap();
			if (data->poly_value->Is<ValueMap>()) {
				ValueMap map = *data->poly_value;
				map.Set(".type_hash", (int64)type_hash);
				*data->poly_value = map;
			}
			else TODO;
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->vfs_value) {
				if (data->vfs_value->type_hash != type_hash)
					data->vfs_value->CreateExt(type_hash);
			}
			else TODO;
		}
		else TODO;
	}
}

bool VirtualNode::IsValue() const {return data && data->mode == VFS_VALUE && data->poly_value;}
bool VirtualNode::IsEntity() const {return data && data->mode == VFS_ENTITY && data->vfs_value;}
Value VirtualNode::GetValue() const {return data && data->mode == VFS_VALUE && data->poly_value ? *data->poly_value : Value();}
void VirtualNode::WriteValue(Value val) {
	ASSERT(IsValue());
	if (data && data->poly_value)
		*data->poly_value = val;
}

VirtualNode::operator bool() const {return data;}
void VirtualNode::Clear() {if (data) {data->Dec(); data = 0;}}
//VirtualNode::Data& VirtualNode::Create() {Clear(); data = new Data(); data->Inc(); return *data;}
VirtualNode::Data& VirtualNode::Create(const VfsPath& p, VfsValue* n)
{
	Clear();
	data = new Data();
	data->path = p;
	data->vfs_value = n;
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
	data->poly_value = v;
	data->mode = VFS_VALUE;
	data->Inc();
	return *data;
}

VNodeComponentCtrl::VNodeComponentCtrl(ValueVFSComponentCtrl& o, const VirtualNode& vnode) : owner(o), vnode(vnode) {
	ASSERT(vnode);
}

DatasetPtrs VNodeComponentCtrl::RealizeEntityVfsObject(const VirtualNode& vnode, hash_t type_hash) {
	DatasetPtrs p = owner.GetDataset();
	TODO
	#if 0
	if (!p.entity)
		return p;
	
	//DUMP(vnode.data->path);
	
	VfsPath path = vnode.data->path;
	ASSERT(vnode.data->vfs_value);
	if (!vnode.data->vfs_value)
		Panic("internal error: todo?");
	
	// needed? vnode.data->vfs_value->CreateExt(type_hash);
	
	path.Add(vnode.data->vfs_value->ext->GetTypeName());
	int i = p.entity->objs.Find(path);
	if (i >= 0) {
		EntityData& data = p.entity->objs[i];
		VfsValueExtFactory::SetEntityData(p, type_hash, data);
	}
	else {
		EntityData* data = it.create_ed_fn();
		ASSERT(data); // ???
		if (data) {
			VfsValueExtFactory::SetEntityData(p, type_hash, *data);
			p.entity->objs.Add(path, data);
		}
	}
	
	#endif
	return p;
}

DatasetPtrs VNodeComponentCtrl::GetDataset() const {
	DatasetPtrs p = owner.GetDataset();
	
	// Get entity-vfs-objects
	if (vnode.IsValue() && p.entity) {
		TODO // seems to be type_hash logic instead of AstValue::kind logic
		#if 0
		const VfsPath& path = vnode.data->path;
		for(auto ed : ~p.entity->objs) {
			if (ed.key.IsLeft(path) && ed.key.GetPartCount() == path.GetPartCount()+1) {
				EntityData& data = ed.value;
				int data_kind = data.GetKind();
				VfsValueExtFactory::Set(p, data_kind, data);
			}
		}
		#endif
	}
	
	return p;
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
	VfsValue& enode = ent.val;
	
	info.name = enode.id;
	
	// Realize some default fields
	ent.Data("description");
	ent.Data("gender");
	
	Vector<VfsValue*> envs = IdeMetaEnv().FindAllEnvs();
	
	// Get all contexts
	all_ctxs.Clear();
	for (VfsValue* env : envs) {
		Vector<VfsValue*> ctxs = env->FindAllShallow(AsTypeHash<Context>());
		for (VfsValue* ctx : ctxs) {
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
					VfsValue& ctx = *all_ctxs[ctx_i];
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
	auto& enode = e.val;
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
	VfsValue& n = e->GetValue();
	p.entity = &e->GetExt<Entity>();
	FillDataset(p, n, 0);
	return p;
}

INITIALIZER_COMPONENT_CTRL(Entity, EntityInfoCtrl)

END_UPP_NAMESPACE
