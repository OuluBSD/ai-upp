#include "Core.h"

NAMESPACE_UPP


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
					n.CreateValue(data->path + name, &const_cast<Value&>(map.GetValue(i)), map.GetKey(i));
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
							auto& o = v.Add().CreateValue(data->path + key, &val, key);
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
							v.Add().CreateValue(data->path + i, &val, i);
						}
					}
				}
				else if (data->poly_value->Is<AstValue>()) {
					TODO
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
				if (!n.ext || !n.ext->GetAll(v)) {
					v.Reserve(n.sub.GetCount());
					for(int i = 0; i < n.sub.GetCount(); i++) {
						auto& sub = n.sub[i];
						if (sub.id.IsEmpty())
							v.Add().Create(data->path + i, &sub);
						else
							v.Add().Create(data->path + (Value)sub.id, &sub);
					}
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
				ValueMap map = *data->poly_value;
				{
					ValueMap sub_map;
					sub_map.Set(".type_hash", (int64)type_hash);
					map.Set(name, sub_map);
				}
				Value val = ValueMap();
				ASSERT(val.Is<ValueMap>());
				n.CreateValue(data->path + name, &val, name);
				map.Add(name, val);
				*data->poly_value = map;
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

VirtualNode VirtualNode::GetAdd(Value name, hash_t type_hash) {
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
				n.CreateValue(data->path + name, &val, name);
			}
		}
		else if (data->mode == VirtualNode::VFS_ENTITY) {
			if (data->vfs_value) {
				VfsValue& sub = data->vfs_value->GetAdd(name, type_hash);
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

VirtualNode::Data& VirtualNode::CreateValue(const VfsPath& p, Value* v, Value key)
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

VfsPath VirtualNode::GetPath() const {
	return data ? data->path : VfsPath();
}


END_UPP_NAMESPACE
