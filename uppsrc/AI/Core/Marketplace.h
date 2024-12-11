#ifndef _AI_Core_Marketplace_h_
#define _AI_Core_Marketplace_h_

NAMESPACE_UPP


struct MarketplaceItem : Moveable<MarketplaceItem> {
	int priority = 0;
	Time added;
	String generic, brand, model;
	double price = 0., cx = 0., cy = 0., cz = 0., weight = 0.;
	String faults, works;
	bool broken = false, good = false;
	Vector<int64> images;
	int64 input_hash = 0;
	String other;
	String title, description;
	int category = 0, subcategory = 0;
	int year_of_manufacturing = 0;
	
	void Serialize(Stream& s) {TODO}
	void Jsonize(JsonIO& json) {
		json
			("priority",priority)
			("added",added)
			("generic",generic)
			("brand",brand)
			("model",model)
			("price",price)
			("cx",cx)
			("cy",cy)
			("cz",cz)
			("weight",weight)
			("faults",faults)
			("works",works)
			("broken",broken)
			("good",good)
			("images",images)
			("title",title)
			("category",category)
			("subcategory",subcategory)
			("description",description)
			("input_hash",input_hash)
			("year_of_manufacturing",year_of_manufacturing)
			("other",other)
			;
	}
	
	String GetTitle() const {
		String s;
		s << generic;
		if (brand.GetCount()) {if (!s.IsEmpty()) s << " "; s << brand;}
		if (model.GetCount()) {if (!s.IsEmpty()) s << " "; s << model;}
		return s;
	}
};

struct MarketplaceData : Component
{
	Vector<MarketplaceItem> items;
	
	MarketplaceData(MetaNode& e) : Component(e) {}
	void Serialize(Stream& s) override {s % items;}
	void Jsonize(JsonIO& json) override {json("items",items);}
	hash_t GetHashValue() const override {return CombineHash(items);}
	static int GetKind() {return METAKIND_ECS_COMPONENT_MARKETPLACE;}
	
};

INITIALIZE(MarketplaceData);


END_UPP_NAMESPACE

#endif
