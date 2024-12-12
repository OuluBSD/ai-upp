#ifndef _AI_Core_Owner_h_
#define _AI_Core_Owner_h_

NAMESPACE_UPP


struct PlatformNeed {
	bool enabled = false;
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("enabled", enabled)
			;
	}
};

struct Need {
	String name;
	Vector<String> causes, messages;
	Array<PlatformNeed> platforms;
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("name", name)
			("causes", causes)
			("messages", messages)
			.VisitVector("platforms", platforms)
			;
	}
};

struct RoleEvent {
	String text;
	VectorMap<int,String> entries;
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("text", text)
			("entries", entries)
			;
	}
};

struct RoleAction {
	struct NeedCause : Moveable<NeedCause> {
		int need_i = -1;
		int cause_i = -1;
		void Visit(NodeVisitor& v) {v("n", need_i)("c", cause_i);}
	};
	String name;
	Vector<NeedCause> need_causes;
	Array<RoleEvent> events;
	
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("name", name)
			.VisitVector("need_causes", need_causes)
			.VisitVector("events", events)
			;
	}
	int FindEvent(const String& event) const;
	
};

struct Role {
	String name;
	Array<Need> needs;
	Array<RoleAction> actions;
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("name", name)
			.VisitVector("needs", needs)
			.VisitVector("actions", actions)
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
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1)	("name", name)
			("year_of_birth", year_of_birth)
			("year_of_hobbyist_begin", year_of_hobbyist_begin)
			("year_of_career_begin", year_of_career_begin)
			("biography", biography)
			("is_guitarist", is_guitarist)
			("electronic_tools", electronic_tools)
			//("profiles", profiles)
			.VisitVector("roles", roles)
			//("marketplace", marketplace)
			;
	}
	static int GetKind() {return METAKIND_ECS_COMPONENT_OWNER;}
	
	int GetOpportunityScore(const LeadOpportunity& opp) const;
	
	static void CopyOld();
};

INITIALIZE(Owner);


END_UPP_NAMESPACE

#endif
