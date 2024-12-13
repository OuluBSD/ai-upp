#ifndef _AI_TextCore_Entity_h_
#define _AI_TextCore_Entity_h_

NAMESPACE_UPP

struct DatasetPtrs {
	Ptr<SrcTxtHeader>		src;
	Ptr<Entity>				entity;
	Ptr<Component>			component;
	
	// Specialized components
	Ptr<LyricalStructure>	lyric_struct;
	Ptr<Script>				script; // TODO rename to lyrics_draft
	Ptr<Lyrics>				lyrics;
	Ptr<Song>				song;
	Ptr<MetaNode>			env;
	Ptr<Owner>				owner; // TODO rename to human?
	Ptr<Release>			release;
	Ptr<Profile>			profile;
	Ptr<BiographySnapshot>	snap;
	
	bool editable_biography = false;
	
	BiographyAnalysis*		analysis = 0;
	Biography*				biography = 0;
	LeadData*				lead_data = 0;
	LeadDataAnalysis*		lead_data_anal = 0;
	LeadDataTemplate*		lead_tmpl = 0;
	
	DatasetPtrs() {}
	DatasetPtrs(const DatasetPtrs& p) {*this = p;}
	void operator=(const DatasetPtrs& p) {
		src = p.src;
		entity = p.entity;
		component = p.component;
		lyric_struct = p.lyric_struct;
		script = p.script;
		lyrics = p.lyrics;
		song = p.song;
		env = p.env;
		owner = p.owner;
		release = p.release;
		profile = p.profile;
		snap = p.snap;
		
		editable_biography = p.editable_biography;
		
		analysis = p.analysis;
		biography = p.biography;
		lead_data = p.lead_data;
		lead_data_anal = p.lead_data_anal;
		lead_tmpl = p.lead_tmpl;
	}
	static DatasetPtrs& Single() {static DatasetPtrs p; return p;}
	
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
