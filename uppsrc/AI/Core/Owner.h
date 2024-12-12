#ifndef _AI_Core_Owner_h_
#define _AI_Core_Owner_h_

NAMESPACE_UPP


struct PlatformNeed {
	bool enabled = false;
	
	void Jsonize(JsonIO& json) {
		json
			("enabled", enabled)
			;
	}
};

struct Need {
	String name;
	Vector<String> causes, messages;
	Array<PlatformNeed> platforms;
	
	void Jsonize(JsonIO& json) {
		json
			("name", name)
			("causes", causes)
			("messages", messages)
			("platforms", platforms)
			;
	}
};

struct RoleEvent {
	String text;
	VectorMap<int,String> entries;
	
	void Jsonize(JsonIO& json) {
		json
			("text", text)
			("entries", entries)
			;
	}
};

struct RoleAction {
	struct NeedCause : Moveable<NeedCause> {
		int need_i = -1;
		int cause_i = -1;
		void Jsonize(JsonIO& json) {json("n", need_i)("c", cause_i);}
	};
	String name;
	Vector<NeedCause> need_causes;
	Array<RoleEvent> events;
	
	
	void Jsonize(JsonIO& json) {
		json
			("name", name)
			("need_causes", need_causes)
			("events", events)
			;
	}
	int FindEvent(const String& event) const;
	
};

struct Role {
	String name;
	Array<Need> needs;
	Array<RoleAction> actions;
	
	void Jsonize(JsonIO& json) {
		json
			("name", name)
			("needs", needs)
			("actions", actions)
			;
	}
	int FindAction(const String& name) const;
	
};

struct Owner : Component
{
	String name;
	int year_of_birth = 0;
	int year_of_hobbyist_begin = 0;
	int year_of_career_begin = 0;
	String biography;
	bool is_guitarist = false;
	String electronic_tools;
	//Array<Profile> profiles;
	Array<Role> roles;
	//MarketplaceData marketplace;
	
	Owner(MetaNode& owner) : Component(owner) {}
	int FindRole(const String& name) const;
	void Store();
	void Load(String name);
	
	void Serialize(Stream& s) override {Panic("TODO");}
	void Jsonize(JsonIO& json) override {
		json
			("name", name)
			("year_of_birth", year_of_birth)
			("year_of_hobbyist_begin", year_of_hobbyist_begin)
			("year_of_career_begin", year_of_career_begin)
			("biography", biography)
			("is_guitarist", is_guitarist)
			("electronic_tools", electronic_tools)
			//("profiles", profiles)
			("roles", roles)
			//("marketplace", marketplace)
			;
	}
	hash_t GetHashValue() const override {Panic("TODO"); return 0;}
	static int GetKind() {return METAKIND_ECS_COMPONENT_OWNER;}
	
	int GetOpportunityScore(const LeadOpportunity& opp) const;
	
	static void CopyOld();
};

INITIALIZE(Owner);


END_UPP_NAMESPACE

#endif
