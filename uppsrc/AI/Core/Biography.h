#ifndef _AI_Core_Biography_h_
#define _AI_Core_Biography_h_

NAMESPACE_UPP

struct BioImage {
	String keywords, text, native_text;
	String image_keywords, image_text;
	String location;
	String people;
	Time time;
	int time_accuracy = TIME_ACCURACY_NONE;
	int64 image_hash = 0;
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("keywords", keywords)
			("text", text)
			("native_text", native_text)
			("image_keywords", image_keywords)
			("image_text", image_text)
			("location", location)
			("people", people)
			("time", time)
			("time_accuracy", time_accuracy)
			("image_hash", image_hash)
			;
	}
};

struct BioRange : Moveable<BioRange> {
	int off = 0, len = 0;
	BioRange() {}
	BioRange(BioRange&& r) {*this = r;}
	BioRange(const BioRange& r) {*this = r;}
	void operator=(const BioRange& r) {off = r.off; len = r.len;}
	hash_t GetHashValue() const {CombineHash c; c.Do(off).Do(len); return c;}
	bool operator==(const BioRange& r) const {return r.off == off && r.len == len;}
	void Visit(NodeVisitor& v) {v("off", off)("len", len);}
	bool operator()(const BioRange& a, const BioRange& b) const {
		int a0 = a.off + a.len - 1; // last item in range
		int b0 = b.off + b.len - 1;
		if (a0 != b0) return a0 < b0; // sort primarily by lesser last item
		return a.len < b.len; // otherwise sort by lesser range length
	}
};


struct BioYear {
	struct Element : Moveable<Element> {
		String key, value;
		byte scores[SCORE_COUNT] = {0,0,0,0,0, 0,0,0,0,0};
		void Visit(NodeVisitor& v) {v("k",key)("v",value); for(int i = 0; i < SCORE_COUNT; i++) v("s" + IntStr(i),scores[i]);}
		void ResetScore() {memset(scores, 0, sizeof(scores));}
		double GetAverageScore() const;
	};
	int year = 0;
	String keywords, text, native_text;
	Array<BioImage> images;
	ArrayMap<BioRange,BioImage> image_summaries;
	String image_text;
	Vector<Element> elements;
	hash_t source_hash = 0;
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("year", year)
			("keywords", keywords)
			("native_text", native_text)
			("text", text)
		.VisitVector("images", images)
		.VisitMapKV("image_summaries", image_summaries)
			("image_text", image_text)
		.VisitVector("elements", elements)
			("source_hash", (int64&)source_hash)
			;
	}
	bool operator()(const BioYear& a, const BioYear& b) const {return a.year < b.year;}
	void RealizeImageSummaries();
	BioImage& GetAddImageSummary(int begin_year, int years);
	int FindElement(const String& key) const;
	String JoinElementMap(String delim0, String delim1);
	double GetAverageScore() const {double d = 0; for (const auto& el : elements) d += el.GetAverageScore(); return d;}
	
};


struct BiographyCategory {
	Array<BioYear> years;
	ArrayMap<BioRange,BioYear> summaries;
	
	
	BiographyCategory() {}
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	.VisitVector("years", years)
			.VisitMapKV("summaries", summaries)
			;
	}
	BioYear& GetAdd(int year);
	int GetFilledCount() const;
	int GetFilledImagesCount() const;
	void RealizeSummaries();
	BioYear& GetAddSummary(int begin_year, int years);
	
};

struct Biography : Component {
	
protected:
	ArrayMap<String, BiographyCategory> categories;
	
public:
	COMPONENT_CONSTRUCTOR(Biography);
	
	struct CatSorter {
		bool operator()(const String& a, const String& b) const {
			int ai = FindBiographyCategoryEnum(a);
			int bi = FindBiographyCategoryEnum(b);
			if (bi == -1) return true;
			if (ai == -1) return false;
			return ai < bi;
		}
	};
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	.VisitMap("categories", categories)
			;
		if (v.IsLoading()) {
			Sort();
		}
	}
	static int GetKind() {return METAKIND_ECS_COMPONENT_BIOGRAPHY;}
	
	BiographyCategory& GetAdd(Owner& o, int enum_);
	BiographyCategory* Find(Owner& o, int enum_);
	const BiographyCategory* Find(Owner& o, int enum_) const;
	void Sort() {
		ArrayMap<String, BiographyCategory> tmp;
		Swap(tmp, categories);
		for(int i = 0; i < BIOCATEGORY_COUNT; i++) {
			String key = GetBiographyCategoryEnumCstr(i);
			int j = tmp.Find(key);
			if (j >= 0) {
				BiographyCategory& cat = tmp[j];
				Swap(categories.Add(key), cat);
			}
			else categories.Add(key);
		}
	}
	Array<BiographyCategory>& AllCategories() {return categories.GetValues();}
	const Array<BiographyCategory>& AllCategories() const {return categories.GetValues();}
	String GetCategoryName(int i) {return KeyToName(categories.GetKey(i));}
	
	void ClearSummary();
	void ClearAll() {categories.Clear();}
};

INITIALIZE(Biography)

struct PhotoPrompt : Moveable<PhotoPrompt> {
	String prompt;
	Vector<String> instructions;
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("prompt", prompt)
			("instructions", instructions)
			;
	}
	
	String GetFilePath(int i) const;
	
};

struct PlatformAnalysisPhoto : Moveable<PlatformAnalysisPhoto> {
	String description;
	Array<PhotoPrompt> prompts;
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("description", description)
			.VisitVector("prompts", prompts)
			;
	}
};


struct PlatformBiographyAnalysis {
	Vector<String> packed_reactions;
	String profile_description_from_biography;
	String descriptions[PLATDESC_LEN_COUNT][PLATDESC_MODE_COUNT];
	bool platform_enabled = false;
	ArrayMap<String,PlatformAnalysisPhoto> epk_photos;
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("packed_reactions", packed_reactions)
			("profile_description_from_biography", profile_description_from_biography)
			("platform_enabled", platform_enabled)
			.VisitMap("epk_photos", epk_photos)
			;
		for(int i = 0; i < PLATDESC_LEN_COUNT; i++) {
			for(int j = 0; j < PLATDESC_MODE_COUNT; j++) {
				String key = GetPlatformDescriptionModeKey(j) + "_" + GetPlatformDescriptionLengthKey(i);
				v(key, descriptions[i][j]);
			}
		}
	}
};

struct BiographyRoleAnalysis {
	Vector<String> merged_biography_reactions;
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("merged_biography_reactions", merged_biography_reactions)
			;
	}
	
};

struct BiographyProfileAnalysis {
	struct Response : Moveable<Response> {
		int year, category;
		String text, keywords;
		double score[BIOSCORE_COUNT] = {0,0,0,0};
		void Visit(NodeVisitor& v) {
			v.Ver(1)
			(1)	("year", year)
				("cat", category)
				("txt", text)
				("keyw", keywords)
				;
			for(int i = 0; i < BIOSCORE_COUNT; i++)
				v("sc" + IntStr(i), score[i]);
		}
	};
	
	Vector<Response> responses;
	VectorMap<int, String> categories;
	String biography_reaction;
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	.VisitVector("responses", responses)
			("categories", categories)
			("biography_reaction", biography_reaction)
			;
	}
};

struct PhotoPromptGroupAnalysis {
	Vector<PlatformAnalysisPhoto> cluster_centers;
	int image_count = 0;
	String prompt;
	
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	.VisitVector("cluster_centers", cluster_centers)
			("image_count", image_count)
			("prompt", prompt)
			;
	}
};

struct PhotoPromptLink : Moveable<PhotoPromptLink> {
	PlatformBiographyAnalysis* pba;
	PlatformAnalysisPhoto* pap;
	PhotoPrompt* pp;
};

struct BiographyAnalysis  {
	Array<Array<BiographyProfileAnalysis>> profiles;
	Array<BiographyRoleAnalysis> roles;
	Array<PlatformBiographyAnalysis> platforms;
	ArrayMap<String, PhotoPromptGroupAnalysis> image_types;
	
	void Realize();
	void RealizePromptImageTypes();
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	.VisitVectorVector("profiles", profiles)
			.VisitVector("roles", roles)
			.VisitVector("platforms", platforms)
			.VisitMap("image_types", image_types)
			;
	}
	void Serialize(Stream& s) {TODO}
	hash_t GetHashValue() const {TODO; return 0;}
	Index<int> GetRequiredRoles() const;
	Index<int> GetRequiredCategories() const;
	Vector<PhotoPromptLink> GetImageTypePrompts(String image_type);
	
	//BiographyCategory& GetAdd(Owner& o, int enum_);
	
};

// TODO easy remove?
#define USE_IMPROVED_ELEMENTS 0

#if USE_IMPROVED_ELEMENTS
	#define ELEMENTS_VAR	improved_elements
#else
	#define ELEMENTS_VAR	elements
#endif

struct ConceptStory : Moveable<ConceptStory> {
	struct Element : Moveable<Element> {
		String key, value;
		Color clr;
		byte scores[SCORE_COUNT] = {0,0,0,0,0, 0,0,0,0,0};
		void Visit(NodeVisitor& v) {
			v.Ver(1)
			(1)	("k",key)("v",value)("clr",clr);
			for(int i = 0; i < SCORE_COUNT; i++)
				v("s" + IntStr(i),scores[i]);
		}
		void ResetScore() {memset(scores, 0, sizeof(scores));}
		double GetAverageScore() const;
	};
	
	hash_t hash = 0;
	String desc;
	Vector<Element> elements;
	#if USE_IMPROVED_ELEMENTS
	Vector<Element> improved_elements;
	#endif
	int src = 0;
	int typeclass = -1;
	int content = -1;
	
	int FindElement(const String& key) const;
	int FindImprovedElement(const String& key) const;
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("hash", (int64&)hash)
			("desc", desc)
			.VisitVector("elements", elements)
			("src", src)
			("typeclass", typeclass)
			("content", content)
			;
		#if USE_IMPROVED_ELEMENTS
		v.VisitVector("improved_elements", improved_elements);
		#endif
	}
	
	double AvSingleScore(int i) const;
	Color GetAverageColor() const;
	double GetAverageScore() const;
	bool operator()(const ConceptStory& a, const ConceptStory& b) const;
	
};

struct Concept {
	int64 belief_uniq = 0;
	String name;
	int snap_rev = -1;
	Time created;
	Vector<ConceptStory> stories;
	
	int FindStory(hash_t h) const;
	ConceptStory& GetAddStory(hash_t h);
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("belief_uniq", belief_uniq)
			("name", name)
			("snap_rev", snap_rev)
			("created", created)
			.VisitVector("stories", stories)
			;
	}
};

struct BiographySnapshot : Component {
	int revision = 0;
	Time last_modified;
	//Biography data; // TODO use from ecs
	BiographyAnalysis analysis;
	Array<Concept> concepts;
	
	BiographySnapshot(MetaNode& o) : Component(o) {}
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1)	("revision", revision)
			("last_modified", last_modified)
			//("data", data)
			.Visit("analysis", analysis)
			.VisitVector("concepts", concepts)
		;
	}
	static int GetKind() {return METAKIND_ECS_COMPONENT_BIOGRAPHY_SNAPSHOT;}
	
};

INITIALIZE(BiographySnapshot)

END_UPP_NAMESPACE

#endif
