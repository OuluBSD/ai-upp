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
	typedef enum : int { SCOPE_COMMENTS, FN_COUNT } FnType;
	FnType fn;
	VectorMap<String, String> data;
	Vector<String> code;
	String lang;

	void Clear() {data.Clear(); code.Clear(); lang="";}
	void Jsonize(JsonIO& json) { json("fn", (int&)fn)("data",data)("code",code)("lang",lang); }

	String Get() const { return StoreAsJson(*this); }
	void Put(const String& s) { LoadFromJson(*this, s); }
};

struct AiAnnotationItem : Moveable<AiAnnotationItem>, AnnotationItem {
	struct Comment : Moveable<Comment> {
		int rel_line = -1;
		hash_t line_hash = 0;
		String txt;
		void Jsonize(JsonIO& json);
		void Serialize(Stream& s);
		bool operator()(const Comment& a, const Comment& b) const {return a.rel_line != b.rel_line ? a.rel_line < b.rel_line : a.txt < b.txt;}
	};
	Vector<Comment> comments;

	void RemoveCommentLine(int rel_line);
	Comment* FindComment(int rel_line);
	void Jsonize(JsonIO& json);
	void Serialize(Stream& s);
	void Sort();
};

struct AiFileInfo : Moveable<AiFileInfo> {
	Vector<AiAnnotationItem> ai_items;

	void Jsonize(JsonIO& json);
	void Serialize(Stream& s);
	void UpdateLinks(FileAnnotation& ann);
	void Sort();
};

END_UPP_NAMESPACE

#endif
