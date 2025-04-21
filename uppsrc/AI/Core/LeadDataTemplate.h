#ifndef _AI_Core_LeadDataTemplate_h_
#define _AI_Core_LeadDataTemplate_h_

NAMESPACE_UPP

struct LeadTemplate {
	hash_t hash = 0;
	int orig_lead_lng = 0;
	int orig_lead_idx = 0;
	String title;
	String text;
	double submission_price = 0;
	Index<int> author_classes;
	Index<int> author_specialities;
	Index<int> profit_reasons;
	Index<int> organizational_reasons;
	
	
	void Visit(Vis& v) {
		v.Ver(1)
		(1)		("hash", (int64&)hash)
				("lng", orig_lead_lng)
				("idx", orig_lead_idx)
				("title", title)
				("text", text)
				("submission", submission_price)
				("author", author_classes)
				("specialities", author_specialities)
				("profit", profit_reasons)
				("organizational", organizational_reasons)
				;
	}
};

struct LeadDataPublisher : Component {
	String name, info;
	String genres, url;
	Vector<String> artists;
	
	COMPONENT_CONSTRUCTOR(LeadDataPublisher)
	void Visit(Vis& v) {
		v.Ver(1)
		(1)		("name", name)
				("info", info)
				("genres", genres)
				("url", url)
				("artists", artists)
				;
	}
	static int GetKind() {return METAKIND_ECS_COMPONENT_LEAD_PUBLISHER;}
};

INITIALIZE(LeadDataPublisher)

struct LeadDataTemplate : Component {
	Array<LeadTemplate> templates;
	Index<String> author_classes;
	Index<String> author_specialities;
	Index<String> profit_reasons;
	Index<String> organizational_reasons;
	
	LeadDataTemplate(MetaNode& n) : Component(n) {}
	void Load();
	void Store();
	void Visit(Vis& json) override;
	static int GetKind() {return METAKIND_ECS_COMPONENT_LEAD_TEMPLATE;}
	
	
};

INITIALIZE(LeadDataTemplate)

END_UPP_NAMESPACE

#endif
