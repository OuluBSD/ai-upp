#ifndef _AI_Common_h_
#define _AI_Common_h_

// TODO convert *Args classes to use Value based args file only (too many classes, when 1 is enough)

struct CurrentFileClang;
struct CurrentFileContext;

NAMESPACE_UPP


#define TASKFN_TYPE int
typedef enum : TASKFN_TYPE {
	FN_INVALID = -1,
	
	FN_VOICEOVER_1_FIND_NATURAL_PARTS,
	FN_VOICEOVER_2A_SUMMARIZE,
	FN_VOICEOVER_2B_SUMMARIZE_TOTAL,
	
} TaskFn;

struct TaskArgs : Moveable<TaskArgs> {
	TaskFn fn = FN_INVALID;
	Value params;
	
	void Jsonize(JsonIO& json) {json("fn", (TASKFN_TYPE&)fn)("params", params);}
	String Get() const { return StoreAsJson(*this); }
	void Put(const String& s) { LoadFromJson(*this, s); }
};

struct VisionArgs {
	int fn = 0;

	void Jsonize(JsonIO& json) { json("fn", fn); }
	String Get() const { return StoreAsJson(*this); }
	void Put(const String& s) { LoadFromJson(*this, s); }
};

struct TranscriptionArgs {
	int fn = 0;
	String file;
	String language;
	int ai_provider_idx = -1;

	void Jsonize(JsonIO& json) { json("fn", fn)("ai_provider_idx", ai_provider_idx)("file", file)("language", language); }
	String Get() const { return StoreAsJson(*this); }
	void Put(const String& s) { LoadFromJson(*this, s); }
};

struct GenericPromptArgs {
	int fn = 0;
	VectorMap<String, Vector<String>> lists;
	String response_path;
	String response_title;
	bool is_numbered_lines = false;

	void Jsonize(JsonIO& json) {
		json("lists", lists)("rp", response_path)("t", response_title)("nl", is_numbered_lines);
	}

	String Get() const { return StoreAsJson(*this); }
	void Put(const String& s) { LoadFromJson(*this, s); }
};

struct CodeArgs {
	typedef enum : int {
		SCOPE_COMMENTS,
		FUNCTIONALITY,
		
		FN_COUNT
	} Fn;
	typedef std::underlying_type<Fn>::type FnType;
	Fn fn;
	VectorMap<String, String> data;
	Vector<String> code;
	String lang;

	void Clear() {data.Clear(); code.Clear(); lang="";}
	void Jsonize(JsonIO& json) { json("fn", reinterpret_cast<FnType&>(fn))("data",data)("code",code)("lang",lang); }

	String Get() const { return StoreAsJson(*this); }
	void Put(const String& s) { LoadFromJson(*this, s); }
};

#if 0

struct AiAnnotationItem {
	struct Data : Moveable<Data> {
		String	txt;
		hash_t	tmp_hash = 0;
		
		Data() {}
		Data(const Data& f) {*this = f;}
		void operator=(const Data& f);
		void Jsonize(JsonIO& json);
		void Serialize(Stream& s);
	};
	struct SourceRange {
		struct Item : Moveable<Item> {
			typedef enum : int {INVALID=-1, COMMENT, OTHER, TYPE_COUNT} Kind;
			typedef std::underlying_type<Kind>::type KindType;
			Kind	kind = INVALID;
			int		rel_line = -1;
			int		data_i = -1;
			
			void Jsonize(JsonIO& json);
			void Serialize(Stream& s);
			bool operator()(const Item& a, const Item& b) const {return a.rel_line != b.rel_line ? a.rel_line < b.rel_line : a.data_i < b.data_i;}
		};
		Vector<Item> items;
		String	range_hash_sha1;
		
		// Keep this data temporary
		Point	pos = Null;
		Point	begin = Null;
		Point	end = Null;
		
		Mutex lock;
		
		SourceRange();
		SourceRange(const SourceRange& f);
		void operator=(const SourceRange& f);
		void Sort();
		void Jsonize(JsonIO& json);
		void Serialize(Stream& s);
		void RemoveLineItem(int rel_line);
		Item* FindItem(int rel_line);
		void RemoveAll(Item::Kind kind);
		bool IsLineAreaPartialMatch(const AnnotationItem& b) const;
	};
	String id; // Upp::Class::Method(Upp::Point p)
	String name; // Method
	String type; // for String x, Upp::String, surely valid for variables only
	String pretty; // void Class::Method(Point p)
	String nspace; // Upp
	String uname; // METHOD
	String nest; // Upp::Class
	String unest; // UPP::CLASS
	String bases; // base classes of struct/class
	int    kind = Null;
	bool   definition = false;
	bool   isvirtual = false;
	bool   isstatic = false;
	Array<SourceRange> source_ranges;
	mutable Mutex lock;
	
	
	AiAnnotationItem() {}
	AiAnnotationItem(const AiAnnotationItem& f) {*this = f;}
	AiAnnotationItem(AiAnnotationItem&& f) {*this = f;}
	bool IsSameContent(const AnnotationItem& b) const;
	void operator=(const AiAnnotationItem& s);
	void Set(const AnnotationItem& s, const String& range_hash_sha1);
	void Jsonize(JsonIO& json);
	void Serialize(Stream& s);
	void Sort();
	SourceRange& RealizeRangeByHashSha1(const String& sha1, bool invalidate_others);
	SourceRange* FindRangeByHashSha1(const String& sha1);
	int FindAddData(const String& txt);
	int GetDataCount() const;
	String GetDataString(int data_i) const;
	SourceRange* FindAnySourceRange();
	
private:
	Vector<Data> data;
	
};

#endif

struct MetaSrcFile;

String GetStringRange(String content, Point begin, Point end);
Vector<String> GetStringArea(const String& content, Point begin, Point end);
bool UpdateMetaSrcFile(MetaSrcFile& f, const String& path);
bool RangeContains(const Point& pos, const Point& begin, const Point& end);


// TextTool classes
// TODO optimize & merge

struct SourceDataAnalysisArgs {
	int fn;
	String artist, song, text;
	Vector<String> words;
	Vector<String> phrases;
	
	void Jsonize(JsonIO& json) {
		json	("fn", fn)
				("artist", artist)
				("song", song)
				("text", text)
				("words", words)
				("phrases", phrases)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
	
};

struct TokenArgs {
	int fn;
	Vector<String> words;
	
	void Jsonize(JsonIO& json) {
		json	("fn", fn)
				("words", words)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
	
};

struct PhraseArgs {
	int fn;
	Vector<String> phrases;
	Vector<String> elements;
	Vector<String> typeclasses;
	Vector<String> contents;
	
	void Jsonize(JsonIO& json) {
		json	("fn", fn)
				("phrases", phrases)
				("elements", elements)
				("typeclasses", typeclasses)
				("contents", contents)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
	
};

struct ActionAnalysisArgs {
	int fn;
	Vector<String> actions;
	
	void Jsonize(JsonIO& json) {
		json	("fn", fn)
				("actions", actions)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
	
};

struct AttrArgs {
	int fn;
	String group;
	Vector<String> groups, values;
	String attr0, attr1;
	
	void Jsonize(JsonIO& json) {
		json	("fn", fn)
				("group", group)
				("groups", groups)
				("values", values)
				("attr0", attr0)
				("attr1", attr1)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
	
};

struct ScriptSolverArgs {
	int fn;
	int sub_fn = -1;
	String lng;
	VectorMap<String,String> artist, release, song;
	Vector<String> parts, attrs, phrases, scores, phrases2, styles;
	Vector<int> counts, offsets;
	String part, vision, ref;
	bool is_story = false;
	bool is_unsafe = false;
	bool is_self_centered = false;
	bool ret_fail = false;
	double factor = 0;
	Vector<String> elements;
	String rhyme_element;
	String previously;
	String peek;
	
	
	struct State {
		String			element;
		String			attr_group;
		String			attr_value;
		int				clr_i = -1;
		String			act_action;
		String			act_arg;
		String			typeclass;
		String			content, content_mod;
		String			style_type;
		String			style_entity;
		int				safety = 0;
		int				line_len = 0;
		int				connector = 0;
		String			line_begin;
		
		void Jsonize(JsonIO& json) {
			json	("element", element)
					("attr_group", attr_group)
					("attr_value", attr_value)
					("clr_i", clr_i)
					("act_action", act_action)
					("act_arg", act_arg)
					("typeclass", typeclass)
					("content", content)
					("content_mod", content_mod)
					("style_type", style_type)
					("style_entity", style_entity)
					("safety", safety)
					("line_len", line_len)
					("connector", connector)
					("line_begin", line_begin)
					;
		}
	};
	
	State state;
	Array<State> line_states;

	
	void Jsonize(JsonIO& json) {
		json	("fn", fn)
				("sub_fn", sub_fn)
				("lng", lng)
				("artist", artist)
				("release", release)
				("song", song)
				("parts", parts)
				("attrs", attrs)
				("phrases", phrases)
				("phrases2", phrases2)
				("styles", styles)
				("scores", scores)
				("counts", counts)
				("offsets", offsets)
				("part", part)
				("vision", vision)
				("ref", ref)
				("is_story", is_story)
				("is_unsafe", is_unsafe)
				("is_self_centered", is_self_centered)
				("ret_fail", ret_fail)
				("factor", factor)
				("elements", elements)
				("rhyme_element", rhyme_element)
				("state", state)
				("line_states", line_states)
				("previously", previously)
				("peek", peek)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
	
};

struct ConceptualFrameworkArgs {
	int fn = 0;
	VectorMap<String,String> elements;
	Vector<String> scores;
	String lyrics, genre;
	
	void Jsonize(JsonIO& json) {
		json	("fn", fn)
				("elements", elements)
				("scores", scores)
				("lyrics", lyrics)
				("genre", genre)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
};

struct SocialArgs {
	int fn = 0;
	String text, description, profile, photo_description;
	VectorMap<String,String> parts;
	int len = 0;
	
	void Jsonize(JsonIO& json) {
		json	("text", text)
				("desc", description)
				("fn", fn)
				("parts", parts)
				("len", len)
				("profile", profile)
				("photo_description", photo_description)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
	
};

struct BiographySummaryProcessArgs {
	int fn = 0;
	VectorMap<String,String> parts;
	
	void Jsonize(JsonIO& json) {
		json	("fn", fn)
				("parts", parts)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
};

struct SongHeaderArgs {
	int fn = 0;
	int tc_i = -1;
	int con_i = -1;
	String lyrics_idea;
	String music_style;
	
	void Jsonize(JsonIO& json) {
		json	("tc_i", tc_i)
				("con_i", con_i)
				("lyrics_idea", lyrics_idea)
				("music_style", music_style)
				("fn", fn)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
	
};

struct LeadSolverArgs {
	int fn = 0;
	int opp_i = 0;
	
	void Jsonize(JsonIO& json) {
		json	("opp_i", opp_i)
				("fn", fn)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
	
};

//TODO: rename to PerspectiveArgs
struct BeliefArgs {
	int fn = 0;
	Vector<String> user;
	Vector<String> pos, neg;
	
	void Jsonize(JsonIO& json) {
		json	("fn", fn)
				("user", user)
				("pos", pos)
				("neg", neg)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
	
};

struct PkgFile : Moveable<PkgFile> {
	int pkg = -1, file = -1;
	
	PkgFile() {}
	PkgFile(PkgFile&& k) {*this = k;}
	PkgFile(const PkgFile& k) {*this = k;}
	PkgFile(int pkg, int file) : pkg(pkg), file(file) {}
	void operator=(const PkgFile& k) {file = k.file; pkg = k.pkg;}
	bool operator()(const PkgFile& a, const PkgFile& b) const {
		if (a.pkg != b.pkg) return a.pkg < b.pkg;
		return a.file < b.file;
	}
	hash_t GetHashValue() const {return CombineHash(pkg, file);}
	bool operator==(const PkgFile& p) const {return pkg == p.pkg && file == p.file;}
	String ToString() const {return Format("[%d:%d]", pkg, file);}
};

inline bool IsFilePosLess(const Point& a, const Point& b) {
	if (a.y != b.y) return a.y < b.y;
	else return a.x < b.x;
}

inline bool IsFilePosLessOrEqual(const Point& a, const Point& b) {
	if (a.y != b.y) return a.y <= b.y;
	else return a.x <= b.x;
}

inline bool IsFilePosGreater(const Point& a, const Point& b) {
	if (a.y != b.y) return a.y > b.y;
	else return a.x > b.x;
}

inline bool IsRangesOverlapping(const Point& begin0, const Point& end0, const Point& begin1, const Point& end1) {
	bool begin_less = IsFilePosLessOrEqual(begin0, begin1);
	bool end_greater = IsFilePosLessOrEqual(end1, end0);
	bool begin1_less = IsFilePosLess(begin1, end0);
	bool end1_greater = IsFilePosLess(begin0, end1);
	bool begin1_in_between = begin_less && begin1_less;
	bool end1_in_between = end_greater && end1_greater;
	bool is_1_subset = begin_less && end_greater;
	bool is_0_subset = !begin_less && !end_greater;
	return begin1_in_between || end1_in_between || is_1_subset || is_0_subset;
}

inline bool IsSubset(const Point& begin0, const Point& end0, const Point& begin1, const Point& end1) {
	// Is range 1 subset of range 0
	bool begin_less = IsFilePosLessOrEqual(begin0, begin1);
	bool end_greater = IsFilePosLessOrEqual(end1, end0);
	bool is_1_subset = begin_less && end_greater;
	return is_1_subset;
}

struct TextRange : Moveable<TextRange> {
	Point begin, end;
	
	TextRange() {}
	TextRange(TextRange&& k) {*this = k;}
	TextRange(const TextRange& k) {*this = k;}
	TextRange(Point begin, Point end) : begin(begin), end(end) {}
	void operator=(const TextRange& k) {begin = k.begin; end = k.end;}
	bool operator()(const TextRange& a, const TextRange& b) const {
		if (a.begin != b.begin) return IsFilePosLess(a.begin, b.begin);
		return IsFilePosLess(a.end, b.end);
	}
	hash_t GetHashValue() const {return CombineHash(begin, end);}
	bool operator==(const TextRange& p) const {return begin == p.begin && end == p.end;}
	String ToString() const {return Format("[%s -> %s]", begin.ToString(), end.ToString());}
	bool Contains(const Point& pt) const {return RangeContains(pt, begin, end);}
};

struct MarketplaceArgs {
	int fn = 0;
	VectorMap<String,String> map;
	
	void Jsonize(JsonIO& json) {
		json	("fn", fn)
				("map", map)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
};

bool IsAllDigit(const String& s);
String AppendUnixFileName(String a, String b);
ValueMap& ValueToMap(Value& val);
ValueArray& ValueToArray(Value& val);
void RemoveColonTrail(String& s);
void RemoveCommentTrail(String& s);
double FractionDbl(const String& s);
String GetDurationString(double seconds);
String GetSizeString(uint64 bytes);
Size GetAspectRatio(Size sz);

END_UPP_NAMESPACE

#endif
