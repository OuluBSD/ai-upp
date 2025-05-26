#ifndef _Meta_Entity_h_
#define _Meta_Entity_h_

struct Component : MetaNodeExt, DatasetProvider {
	Component(MetaNode& owner) : MetaNodeExt(owner) {}
	DatasetPtrs GetDataset() const override;
	
};

#define COMPONENT_CONSTRUCTOR_(x) CLASSTYPE(x) x(MetaNode& n) : Component(n)
#define COMPONENT_CONSTRUCTOR(x) COMPONENT_CONSTRUCTOR_(x) {}
#define COMPONENT_OVERRIDE_TODO void Visit(Vis& s) override {TODO}
#define METANODE_EXT_CONSTRUCTOR_(x) CLASSTYPE(x) x(MetaNode& n) : MetaNodeExt(n)
#define METANODE_EXT_CONSTRUCTOR(x) METANODE_EXT_CONSTRUCTOR_(x) {}

#define COMPONENT_STUB_HEADER(type) \
struct type : Component \
{ \
	COMPONENT_CONSTRUCTOR(type) \
	void Visit(Vis& v) override {v.Ver(1)(1);} \
}; \
INITIALIZE(type)

#define COMPONENT_STUB_IMPL(type) \
	INITIALIZER_COMPONENT(type);

struct Entity : MetaNodeExt {
	METANODE_EXT_CONSTRUCTOR(Entity)
	void Clear() {data.Clear();}
	void Visit(Vis& v) override;
	int GetGender() const;
	EntityData* FindData(const VfsPath& path);
	
	bool operator()(const Entity& a, const Entity& b) const {
		return a.data.Get("order", Value()) < b.data.Get("order", Value());
	}
	
	static int GetKind() {return METAKIND_ECS_ENTITY;}
	
protected:
	friend class EntityInfoCtrl;
	friend class VNodeComponentCtrl;
	friend struct MetaEnvironment;
	friend struct VirtualNode;
	friend struct IdeMetaEnvironment;
	VectorMap<String, Value> data;
	ArrayMap<VfsPath, EntityData> objs;
	Value& Data(const String& key) {return data.GetAdd(key);}
};

INITIALIZE(Entity);


struct ValueComponentBase : Component
{
	Value value;
	ValueComponentBase(MetaNode& n) : Component(n) {}
	void Visit(Vis& v) override {
		v.Ver(1)
		(1)	("value",value);
	}
};

template <int kind>
struct ValueComponent : ValueComponentBase
{
	using Type = ValueComponent<kind>;
	ValueComponent(MetaNode& n) : ValueComponentBase(n) {}
	static int GetKind() {return kind;}
};

#define INITIALIZE_VALUECOMPONENT(x, kind) \
struct x : ValueComponent<kind> { \
	CLASSTYPE(x) \
	x(MetaNode& n) : ValueComponent(n) {} \
}; \
INITIALIZE(x)


#endif
