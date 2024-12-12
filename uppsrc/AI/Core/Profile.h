#ifndef _AI_Core_Profile_h_
#define _AI_Core_Profile_h_

NAMESPACE_UPP

struct Profile : Component
{
	String name;
	Date begin;
	String biography;
	String preferred_genres;
	Index<int> languages;
	//Array<BiographySnapshot> snapshots;
	
	BiographySnapshot* FindSnapshotRevision(int i);
	
	Profile(MetaNode& owner) : Component(owner) {}
	void Serialize(Stream& s) override {Panic("TODO");}
	void Jsonize(JsonIO& json) override {
		json
			("name", name)
			("begin", begin)
			("biography", biography)
			("preferred_genres", preferred_genres)
			("languages", languages)
			//("snapshots", snapshots)
			;
		TODO
		#if 0
		if (json.IsLoading() && snapshots.IsEmpty()) {
			auto& s = snapshots.Add();
			json
				("biography_detailed", s.data)
				("biography_analysis", s.analysis)
			;
			s.last_modified = GetSysTime();
		}
		#endif
	}
	hash_t GetHashValue() const override {Panic("TODO"); return 0;}
	
	Owner* GetOwner() const; // TODO rename
	
	static int GetKind() {return METAKIND_ECS_COMPONENT_PROFILE;}
	
};

INITIALIZE(Profile)

END_UPP_NAMESPACE

#endif
