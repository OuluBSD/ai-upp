#ifndef _AI_TextCore_Entity_h_
#define _AI_TextCore_Entity_h_

NAMESPACE_UPP

class Component {
public:
	~Component() {}

	void Serialize(Stream& s) {}
	void Jsonize(JsonIO& json) {}
};

class Entity {
public:
	VectorMap<String, Value> data;
	Array<Component> comps;

	void Clear()
	{
		data.Clear();
		comps.Clear();
	}
	void Serialize(Stream& s) { s % data % comps; }
	void Jsonize(JsonIO& json) { json("data", data)("comps", comps); }

	bool operator()(const Entity& a, const Entity& b) const
	{
		return a.data.Get("order", Value()) < b.data.Get("order", Value());
	}
};

END_UPP_NAMESPACE

#endif
