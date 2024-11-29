#ifndef _AI_TextCore_Entity_h_
#define _AI_TextCore_Entity_h_

NAMESPACE_UPP

struct Component : MetaNode {
	String name;
	
	virtual String GetTypename() const = 0;
	virtual const std::type_info& GetType() const = 0;
	
	Component* GetComponent() {return this;}
	void Serialize(Stream& s) override {MetaNode::Serialize(s); s % name;}
	void Jsonize(JsonIO& json) override {MetaNode::Jsonize(json); json("name",name);}
	
};

struct Entity : MetaNode {
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
	void Serialize(Stream& s) override {MetaNode::Serialize(s); s % name % type % data % gender; }
	void Jsonize(JsonIO& json) override {MetaNode::Jsonize(json); json("name", name)("type", type)("data", data)("gender",gender); }

	bool operator()(const Entity& a, const Entity& b) const {
		return a.data.Get("order", Value()) < b.data.Get("order", Value());
	}
};

END_UPP_NAMESPACE

#endif
