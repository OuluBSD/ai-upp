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
	
	DatasetPtrs() {}
	DatasetPtrs(const DatasetPtrs& p) {*this = p;}
	void operator=(const DatasetPtrs& p) {
		src = p.src;
		entity = p.entity;
		component = p.component;
		script = p.script;
		lyrics = p.lyrics;
	}
	static DatasetPtrs& Single() {static DatasetPtrs p; return p;}
	
};

struct Component : MetaNodeExt {
	
	DatasetPtrs GetDataset();
	
};

struct Entity : MetaNodeExt {
	String name, type;
	VectorMap<String, Value> data;
	bool gender = false;
	
	Entity() {}
	void Clear()
	{
		name.Clear();
		type.Clear();
		data.Clear();
		gender = 0;
	}
	void Serialize(Stream& s) override {s % name % type % data % gender; }
	void Jsonize(JsonIO& json) override {json("name", name)("type", type)("data", data)("gender",gender); }
	hash_t GetHashValue() const override {CombineHash ch; ch.Do(name).Do(type).Do(data).Do(gender); return ch;}
	Value& Data(const String& key) {return data.GetAdd(key);}
	
	bool operator()(const Entity& a, const Entity& b) const {
		return a.data.Get("order", Value()) < b.data.Get("order", Value());
	}
	
	static int GetKind() {return METAKIND_ECS_ENTITY;}
	
};


INITIALIZE(Entity);


END_UPP_NAMESPACE

#endif
