#ifndef _AI_Core_Profile_h_
#define _AI_Core_Profile_h_



struct Profile : Component
{
	String name;
	Date created;
	String description;
	String preferences;
	Index<int> languages;
	
	BiographyPerspectives* FindSnapshotRevision(int i);
	
	CLASSTYPE(Profile)
	Profile(VfsValue& owner) : Component(owner) {}
	String GetName() const override {return name;}
	void Visit(Vis& v) override {
		v.Ver(1)
		(1)
			("name", name)
			("created", created)
			("description", description)
			("preferences", preferences)
			("languages", languages)
			;
	}
	
	static int GetKind() {return METAKIND_ECS_COMPONENT_PROFILE;}
	
};

INITIALIZE(Profile)



#endif
