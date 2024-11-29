#ifndef _AI_TextCore_Entity_h_
#define _AI_TextCore_Entity_h_

NAMESPACE_UPP

struct Component : MetaNodeExt {
	String name;
	
	Component* GetComponent() {return this;}
	void Serialize(Stream& s) override {s % name;}
	void Jsonize(JsonIO& json) override {json("name",name);}
	
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
	
	bool operator()(const Entity& a, const Entity& b) const {
		return a.data.Get("order", Value()) < b.data.Get("order", Value());
	}
	
	static int GetKind() {return METAKIND_ECS_ENTITY;}
	
};


INITIALIZE(Entity);


END_UPP_NAMESPACE

#endif
