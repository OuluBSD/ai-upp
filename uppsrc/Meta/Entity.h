#ifndef _Meta_Entity_h_
#define _Meta_Entity_h_

#define DATASET_ITEM(type, name, kind, group, desc) class type;
DATASET_LIST
#undef DATASET_ITEM

struct DatasetPtrs {
	#define DATASET_ITEM(type, name, kind, group, desc) Ptr<type> name;
	DATASET_LIST
	#undef DATASET_ITEM
	
	bool editable_biography = false;
	
	DatasetPtrs() {}
	DatasetPtrs(const DatasetPtrs& p) {*this = p;}
	void operator=(const DatasetPtrs& p);
	static DatasetPtrs& Single() {static DatasetPtrs p; return p;}
	void Clear();
};

void FillDataset(DatasetPtrs& p, MetaNode& n, Component* this_comp);

class DatasetProvider {
public:
	virtual DatasetPtrs GetDataset() const = 0;
	
};

struct Component : MetaNodeExt, DatasetProvider {
	Component(MetaNode& owner) : MetaNodeExt(owner) {}
	DatasetPtrs GetDataset() const override;
	
};

#define COMPONENT_CONSTRUCTOR_(x) x(MetaNode& n) : Component(n)
#define COMPONENT_CONSTRUCTOR(x) COMPONENT_CONSTRUCTOR_(x) {}
#define COMPONENT_OVERRIDE_TODO void Visit(Vis& s) override {TODO}
#define METANODE_EXT_CONSTRUCTOR_(x) x(MetaNode& n) : MetaNodeExt(n)
#define METANODE_EXT_CONSTRUCTOR(x) METANODE_EXT_CONSTRUCTOR_(x) {}

#define COMPONENT_STUB_HEADER(type, kind) \
struct type : Component \
{ \
	COMPONENT_CONSTRUCTOR(type) \
	void Visit(Vis& v) override { \
		v.Ver(1)(1);} \
	static int GetKind() {return kind;} \
}; \
INITIALIZE(type)

#define COMPONENT_STUB_IMPL(type, kind) \
	INITIALIZER_COMPONENT(type);

struct EntityData : Pte<EntityData> {
	virtual ~EntityData() {}
	virtual int GetKind() const = 0;
	virtual void Visit(Vis& s) = 0;
};

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
	x(MetaNode& n) : ValueComponent(n) {} \
}; \
INITIALIZE(x)


#endif
