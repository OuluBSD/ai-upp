#ifndef _AI_TextCore_Entity_h_
#define _AI_TextCore_Entity_h_

NAMESPACE_UPP

class Component : Pte<Component> {
public:
	String name;
	
	virtual ~Component() {}
	virtual String GetTypename() const = 0;
	virtual const std::type_info& GetType() const = 0;
	
	Component* GetComponent() {return this;}
	void Serialize(Stream& s) {s % name;}
	void Jsonize(JsonIO& json) {json("name",name);}
};

class Entity : Pte<Entity> {
public:
	String name, type;
	VectorMap<String, Value> data;
	Array<Component> comps;

	Entity() {}
	virtual ~Entity() {}
	void Clear()
	{
		name.Clear();
		type.Clear();
		data.Clear();
		comps.Clear();
	}
	void Serialize(Stream& s) { s % name % type % data /*% comps*/; }
	void Jsonize(JsonIO& json) { json("name", name)("type", type)("data", data)/*("comps", comps)*/; }

	bool operator()(const Entity& a, const Entity& b) const {
		return a.data.Get("order", Value()) < b.data.Get("order", Value());
	}
	template <class T> Vector<T*> FindAll() {
		Vector<T*> v; for (auto& comp : comps) {T* o = dynamic_cast<T*>(&comp); v << o;} return v;
	}
	template <class T> Vector<const T*> FindAll() const {
		Vector<const T*> v; for (const auto& comp : comps) {const T* o = dynamic_cast<const T*>(&comp); v << o;} return v;
	}
};

END_UPP_NAMESPACE

#endif
