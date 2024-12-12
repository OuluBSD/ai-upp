#ifndef _AI_Core_Platform_h_
#define _AI_Core_Platform_h_

NAMESPACE_UPP

struct Platform : Component {
	const char* group = 0;
	const char* name = 0;
	const char* description = 0;
	int profile_type = PLATFORM_PROFILE_ANY;
	bool attrs[PLATFORM_ATTR_COUNT];
	Vector<String> functions;
	
	Platform(MetaNode& o) : Component(o) {memset(attrs, 0, sizeof(attrs));}
	
	void SetAttr(String name, bool value);
	
	Platform& operator << (const char* fn) {functions << fn; return *this;}
	
	void Jsonize(JsonIO& json) override {TODO}
	void Serialize(Stream& s) override {TODO}
	hash_t GetHashValue() const override {TODO; return 0;}
	static int GetKind() {return METAKIND_ECS_COMPONENT_PLATFORM;}
	
	/*String name;
	bool has_title = false;
	bool has_message = false;
	bool has_hashtags = false;
	bool has_audio = false;
	bool has_music = false;
	bool has_video = false;
	bool has_reel = false;
	bool has_image = false;
	bool has_link_promotion = false;
	bool has_comments = false;
	bool has_comment_self_promotion_hack = false; // it's a unofficial policy to self-promote in comments
	
	bool has_description = false;
	bool has_profile_image = false;
	bool has_music_cover = false;
	bool has_Q_and_A = false;
	bool has_Q_and_A_hack = false;
	bool has_policy_real_person = false;
	bool has_policy_artist_only = false;
	bool has_testimonial_hack = false; // real testimonials, but injected inside normal text
	
	Vector<int> roles;
	
	void AddRole(int i);*/
};

INITIALIZE(Platform)

//const Vector<Platform>& GetPlatforms();


struct PlatformComment {
	String user;
	String orig_message, message, keywords, location;
	String text_merged_status;
	Time published;
	bool generate = false;
	Array<PlatformComment> responses;
	
	
	int GetTotalComments() const;
	void ClearMerged();
	
	void Jsonize(JsonIO& json) {
		json
			("user", user)
			("orig_message", orig_message)
			("message", message)
			("keywords", keywords)
			("location", location)
			("tms", text_merged_status)
			("published", published)
			("generate", generate)
			("responses", responses)
			;
	}
};

struct PlatformThread {
	String user, title;
	Array<PlatformComment> comments;
	
	int GetTotalComments() const;
	
	void Jsonize(JsonIO& json) {
		json
			("user", user)
			("title", title)
			("comments", comments)
			;
	}
};

struct PlatformEntry {
	Array<PlatformThread> threads;
	String title, subforum;
	
	void Jsonize(JsonIO& json) {
		json
			("threads", threads)
			("title", title)
			("subforum", subforum)
			;
	}
};

struct PlatformData {
	Array<PlatformEntry> entries;
	
	
	int GetTotalEntryCount() const;
	void Jsonize(JsonIO& json) {
		json
			("entries", entries)
			;
	}
};

struct ProfileData {
	hash_t hash = 0;
	Array<PlatformData> platforms;
	
	Profile* profile = 0;
	
	
	void Jsonize(JsonIO& json);
	void Load();
	void Store();
	
	static void StoreAll();
	static Array<ProfileData>& GetAll();
	static ProfileData& Get(Profile& p);
	
	
};


struct PlatformAnalysis {
	Index<int> roles;
	VectorMap<String,String> epk_text_fields;
	VectorMap<String,PlatformAnalysisPhoto> epk_photos;
	
	
	void Jsonize(JsonIO& json) {
		json
			("roles", roles)
			("epk_text_fields", epk_text_fields)
			("epk_photos", epk_photos)
			;
	}
	
	int GetRoleScoreSum(int score_i) const;
	double GetRoleScoreSumWeighted(int score_i) const;
	
};

END_UPP_NAMESPACE

#endif
