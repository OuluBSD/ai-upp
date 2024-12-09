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
	
	
	void Jsonize(JsonIO& json) {
		json	("hash", (int64&)hash)
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

struct LeadDataPublisher {
	String name, info;
	String genres, url;
	Vector<String> artists;
	
	void Jsonize(JsonIO& json) {
		json	("name", name)
				("info", info)
				("genres", genres)
				("url", url)
				("artists", artists)
				;
	}
};

struct LeadDataTemplate {
	Array<LeadTemplate> templates;
	Index<String> author_classes;
	Index<String> author_specialities;
	Index<String> profit_reasons;
	Index<String> organizational_reasons;
	
	Array<LeadDataPublisher> publishers;
	
	LeadDataTemplate();
	LeadDataTemplate(LeadDataTemplate&&) {}
	LeadDataTemplate(const LeadDataTemplate&) {}
	void Load();
	void Store();
	//LeadOpportunity& GetAddOpportunity(int leadsite, String id);
	void Jsonize(JsonIO& json);
	
	static LeadDataTemplate& Single() {static LeadDataTemplate o; return o;}
	
};

END_UPP_NAMESPACE

#endif
