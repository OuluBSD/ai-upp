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


END_UPP_NAMESPACE

#endif
