#ifndef _AI_TextCore_Entity_h_
#define _AI_TextCore_Entity_h_

NAMESPACE_UPP

struct Entity;
struct SrcTxtHeader;
struct Component;
struct Script;
struct Lyrics;

struct DatasetPtrs {
	Ptr<SrcTxtHeader>		src;
	Ptr<Entity>				entity;
	Ptr<Component>			component;
	
	// Specialized components
	Ptr<Script>				script; // TODO rename to lyrics_draft
	Ptr<Lyrics>				lyrics;
	Ptr<MetaNode>			env;
	
	DatasetPtrs() {}
	DatasetPtrs(const DatasetPtrs& p) {*this = p;}
	void operator=(const DatasetPtrs& p) {
		src = p.src;
		entity = p.entity;
		component = p.component;
		script = p.script;
		lyrics = p.lyrics;
		env = p.env;
	}
	static DatasetPtrs& Single() {static DatasetPtrs p; return p;}
	
};

struct Component : MetaNodeExt {
	
	Component(MetaNode& owner) : MetaNodeExt(owner) {}
	DatasetPtrs GetDataset();
	
};

struct Entity : MetaNodeExt {
	VectorMap<String, Value> data;
	
	Entity(MetaNode& owner) : MetaNodeExt(owner) {}
	void Clear()
	{
		data.Clear();
	}
	void Serialize(Stream& s) override {s % data; }
	void Jsonize(JsonIO& json) override {json("data", data); }
	hash_t GetHashValue() const override {CombineHash ch; ch.Do(data); return ch;}
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
