#ifndef _AI_TextCore_Entity_h_
#define _AI_TextCore_Entity_h_

NAMESPACE_UPP

#define EXT_LIST \
	DATASET_ITEM(Entity,			entity,			METAKIND_ECS_ENTITY) \
	DATASET_ITEM(Component,			component,		METAKIND_ECS_COMPONENT_UNDEFINED) \

#define COMPONENT_LIST \
	DATASET_ITEM(SrcTxtHeader,		src,			METAKIND_DATABASE_SOURCE) \
	DATASET_ITEM(LyricalStructure,	lyric_struct,	METAKIND_ECS_COMPONENT_LYRICAL_STRUCTURE) \
	DATASET_ITEM(Script,			script,			METAKIND_ECS_COMPONENT_SCRIPT) /* TODO rename to lyrics_draft */ \
	DATASET_ITEM(Lyrics,			lyrics,			METAKIND_ECS_COMPONENT_LYRICS) \
	DATASET_ITEM(Song,				song,			METAKIND_ECS_COMPONENT_SONG) \
	DATASET_ITEM(Owner,				owner,			METAKIND_ECS_COMPONENT_OWNER) /* TODO rename to human? */ \
	DATASET_ITEM(Release,			release,		METAKIND_ECS_COMPONENT_RELEASE) \
	DATASET_ITEM(Profile,			profile,		METAKIND_ECS_COMPONENT_PROFILE) \
	DATASET_ITEM(BiographySnapshot,	snap,			METAKIND_ECS_COMPONENT_BIOGRAPHY_SNAPSHOT) \
	DATASET_ITEM(LeadData,			lead_data,		METAKIND_ECS_COMPONENT_LEAD_DATA) \
	DATASET_ITEM(LeadDataTemplate,	lead_tmpl,		METAKIND_ECS_COMPONENT_LEAD_TEMPLATE) \

#define NODE_LIST \
	DATASET_ITEM(MetaNode,			env,			METAKIND_PKG_ENV) \

#define DATASET_LIST \
	EXT_LIST \
	COMPONENT_LIST \
	NODE_LIST \

struct DatasetPtrs {
	#define DATASET_ITEM(type, name, kind) Ptr<type> name;
	DATASET_LIST
	#undef DATASET_ITEM
	
	bool editable_biography = false;
	
	BiographyAnalysis*		analysis = 0;
	Biography*				biography = 0;
	
	DatasetPtrs() {}
	DatasetPtrs(const DatasetPtrs& p) {*this = p;}
	void operator=(const DatasetPtrs& p) {
		#define DATASET_ITEM(type, name, kind) name = p.name;
		DATASET_LIST
		#undef DATASET_ITEM
		
		editable_biography = p.editable_biography;
		
		analysis = p.analysis;
		biography = p.biography;
	}
	static DatasetPtrs& Single() {static DatasetPtrs p; return p;}
	void Clear() {
		#define DATASET_ITEM(type, name, kind) name = 0;
		DATASET_LIST
		#undef DATASET_ITEM
		editable_biography = 0;
		analysis = 0;
		biography = 0;
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

struct Entity : MetaNodeExt {
	VectorMap<String, Value> data;
	
	METANODE_EXT_CONSTRUCTOR(Entity)
	void Clear() {data.Clear();}
	void Visit(NodeVisitor& v) override {v.Ver(1)(1)("data",data);}
	Value& Data(const String& key) {return data.GetAdd(key);}
	int GetGender() const;
	
	bool operator()(const Entity& a, const Entity& b) const {
		return a.data.Get("order", Value()) < b.data.Get("order", Value());
	}
	
	static int GetKind() {return METAKIND_ECS_ENTITY;}
	
};


INITIALIZE(Entity);


END_UPP_NAMESPACE

#endif
