#ifndef _AI_Common_h_
#define _AI_Common_h_

struct CurrentFileClang;
struct CurrentFileContext;

NAMESPACE_UPP

struct VisionArgs {
	int fn = 0;

	void Jsonize(JsonIO& json) { json("fn", fn); }
	String Get() const { return StoreAsJson(*this); }
	void Put(const String& s) { LoadFromJson(*this, s); }
};

struct GenericPromptArgs {
	int fn = 0;
	VectorMap<String, Vector<String>> lists;
	String response_title;
	bool is_numbered_lines = false;

	void Jsonize(JsonIO& json)
	{
		json("fn", fn)("lists", lists)("t", response_title)("nl", is_numbered_lines);
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

struct AiFileInfo : Moveable<AiFileInfo> {
	Array<AiAnnotationItem> ai_items;
	mutable Mutex lock;
	
	AiFileInfo() {}
	AiFileInfo(const AiFileInfo& f) {*this = f;}
	AiFileInfo(AiFileInfo&& f) : ai_items(pick(f.ai_items)) {}
	void operator=(const AiFileInfo& s);
	void Jsonize(JsonIO& json);
	void Serialize(Stream& s);
	void UpdateLinks(FileAnnotation& ann);
};


String GetStringRange(String content, Point begin, Point end);
Vector<String> GetStringArea(const String& content, Point begin, Point end);
bool UpdateAiFileInfo(AiFileInfo& f, const String& path);
bool RangeContains(Point pos, Point begin, Point end);


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
	int lng_i = -1;
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
		String			attr_key;
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
					("attr_key", attr_key)
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
				("lng_i", lng_i)
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


END_UPP_NAMESPACE

#endif
