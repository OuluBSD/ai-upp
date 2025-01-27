#ifndef _AI_TextCore_Entity_h_
#define _AI_TextCore_Entity_h_

NAMESPACE_UPP

struct DatasetPtrs {
	#define DATASET_ITEM(type, name, kind, group, desc) Ptr<type> name;
	DATASET_LIST
	#undef DATASET_ITEM
	
	bool editable_biography = false;
	
	DatasetPtrs() {}
	DatasetPtrs(const DatasetPtrs& p) {*this = p;}
	void operator=(const DatasetPtrs& p) {
		#define DATASET_ITEM(type, name, kind, group, desc) name = p.name;
		DATASET_LIST
		#undef DATASET_ITEM
		
		editable_biography = p.editable_biography;
	}
	static DatasetPtrs& Single() {static DatasetPtrs p; return p;}
	void Clear() {
		#define DATASET_ITEM(type, name, kind, group, desc) name = 0;
		DATASET_LIST
		#undef DATASET_ITEM
		editable_biography = 0;
	}
};

void FillDataset(DatasetPtrs& p, MetaNode& n, Component* this_comp);

struct Component : MetaNodeExt {
	Component(MetaNode& owner) : MetaNodeExt(owner) {}
	DatasetPtrs GetDataset() const;
	
};

#define COMPONENT_CONSTRUCTOR_(x) x(MetaNode& n) : Component(n)
#define COMPONENT_CONSTRUCTOR(x) COMPONENT_CONSTRUCTOR_(x) {}
#define COMPONENT_OVERRIDE_TODO void Visit(NodeVisitor& s) override {TODO}
#define METANODE_EXT_CONSTRUCTOR_(x) x(MetaNode& n) : MetaNodeExt(n)
#define METANODE_EXT_CONSTRUCTOR(x) METANODE_EXT_CONSTRUCTOR_(x) {}

#define COMPONENT_STUB_HEADER(type, kind) \
struct type : Component \
{ \
	COMPONENT_CONSTRUCTOR(type) \
	void Visit(NodeVisitor& v) override { \
		v.Ver(1)(1);} \
	static int GetKind() {return kind;} \
}; \
INITIALIZE(type)

#define COMPONENT_STUB_IMPL(type, kind) \
	INITIALIZER_COMPONENT(type);


struct Entity : MetaNodeExt {
	METANODE_EXT_CONSTRUCTOR(Entity)
	void Clear() {data.Clear();}
	void Visit(NodeVisitor& v) override {v.Ver(1)(1)("data",data);}
	int GetGender() const;
	
	bool operator()(const Entity& a, const Entity& b) const {
		return a.data.Get("order", Value()) < b.data.Get("order", Value());
	}
	
	static int GetKind() {return METAKIND_ECS_ENTITY;}
	
protected:
	friend class EntityInfoCtrl;
	friend struct MetaEnvironment;
	VectorMap<String, Value> data;
	Value& Data(const String& key) {return data.GetAdd(key);}
};


INITIALIZE(Entity);


END_UPP_NAMESPACE

#endif
