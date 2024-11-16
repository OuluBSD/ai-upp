#ifndef _AI_TextCore_Lyric_h_
#define _AI_TextCore_Lyric_h_

NAMESPACE_UPP

// TODO rename LYRICPART_
typedef enum : int {
	TXT_NORMAL,
	TXT_PRE_REPEAT,
	TXT_REPEAT,
	TXT_TWIST,
	TXT_NULL,

	TXT_COUNT
} TextPartType;

String GetTextTypeString(int i);
void ParseTextPartType(String part_name, TextPartType& text_type, int& text_num);

typedef enum : int {
	VOICE_SINGING,
	VOICE_RAPPING,
	VOICE_POETRY,
	VOICE_DIALOG,
	VOICE_SKIP,

	VOICE_TYPE_COUNT
} VoiceType;

struct LineScore : Moveable<LineScore> {
	Vector<String> lines;
	Vector<int> scores;
	int line_n = 0;

	String Get(int i, int j) const;
	int GetScore(int i, int j) const;
	void SetCount(int i, int line_n);
	void Set(int i, int j, const String& s);
	void SetScore(int i, int j, int value);
	int GetCount() const;

	void Jsonize(JsonIO& json) { json("lines", lines)("scores", scores)("line_n", line_n); }
};

struct LineElement {
	String element;
	AttrHeader attr;
	ActionHeader act;
	int clr_i = -1;
	int typeclass_i = -1;
	int con_i = -1;
	hash_t sorter = 0;

	void Jsonize(JsonIO& json)
	{
		json("e", element)("at", attr)("clr", clr_i)("a", act)("t", typeclass_i)("c", con_i)(
			"s", (int64&)sorter);
	}
	void Overlay(const LineElement& le);
};

struct DynLine : Moveable<DynLine> {
	String text;
	String alt_text;
	String edit_text;
	String user_text;
	String expanded;
	Vector<String> suggs;
	int pp_i = -1;
	int end_pp_i = -1;
	LineElement el;

	// Suggestion variables
	int style_type = 0;
	int style_entity = 0;
	int safety = 0;
	int line_len = 0;
	int connector = 0;
	String line_begin;

	void Jsonize(JsonIO& json)
	{
		json("text", text)("alt_text", alt_text)("edit_text", edit_text)(
			"user_text", user_text)("ex", expanded)("el", el)("suggs", suggs)("pp_i", pp_i)(
			"end_pp_i", end_pp_i)("style_type", style_type)("style_entity", style_entity)(
			"safety", safety)("line_len", line_len)("connector", connector)("line_begin",
		                                                                    line_begin);
		if(json.IsLoading()) {
			String element;
			json("element", element);
			if(!element.IsEmpty() && el.element.IsEmpty())
				el.element = element;
		}
	}
	void CopySuggestionVars(const DynLine& dl)
	{
		style_type = dl.style_type;
		style_entity = dl.style_entity;
		safety = dl.safety;
		line_len = dl.line_len;
		connector = dl.connector;
		line_begin = dl.line_begin;
	}
};

struct DynSub : Moveable<DynSub> {
	LineElement el;
	Vector<DynLine> lines;
	String story;

	void Jsonize(JsonIO& json)
	{
		json("lines", lines)("el", el)("story", story);
		if(json.IsLoading()) {
			String e0, e1;
			json("element0", e0);
			json("element1", e1);
			if(!e0.IsEmpty() && el.element.IsEmpty())
				el.element = e0;
			if(!e1.IsEmpty() && el.element.IsEmpty())
				el.element = e1;
		}
	}
};

struct DynPart {
	VoiceType voice_type = VOICE_SINGING;
	TextPartType text_type = TXT_NORMAL;
	int text_num = -1;
	int text_lines = 0;
	int text_lines_per_sub = 0;
	String person;
	LineElement el;
	Vector<DynSub> sub;
	Vector<int> phrase_parts;
	String story;

	void Jsonize(JsonIO& json)
	{
		json("voice_type", (int&)voice_type)("text_type", (int&)text_type)(
			"text_num", text_num)("text_lines", text_lines)(
			"text_lines_per_sub", text_lines_per_sub)("person", person)("el", el)("sub", sub)(
			"phrase_parts", phrase_parts)("story", story);
		if(json.IsLoading()) {
			String element;
			json("element", element);
			if(!element.IsEmpty() && el.element.IsEmpty())
				el.element = element;
		}
	}
	String GetName() const;
	int GetExpectedLineCount() const;
	int GetContrastIndex() const;
	String GetLineElementString(int line) const;
};

struct ScriptStruct : Moveable<ScriptStruct> {
	ScriptStruct() {}
	ScriptStruct(const ScriptStruct& s) { *this = s; }
	ScriptStruct(ScriptStruct&& s) { *this = s; }
	void operator=(const ScriptStruct& s) { parts <<= s.parts; }

	struct SubSubPart : Moveable<SubSubPart> {
		Vector<int> token_texts;
		int cls = -1;

		SubSubPart() {}
		SubSubPart(const SubSubPart& s) { *this = s; }
		void Serialize(Stream& s) { s % token_texts % cls; }
		void Jsonize(JsonIO& json) { json("token_texts", token_texts)("cls", cls); }
		void operator=(const SubSubPart& s)
		{
			token_texts <<= s.token_texts;
			cls = s.cls;
		}
	};
	struct SubPart : Moveable<SubPart> {
		Vector<SubSubPart> sub;
		int cls = -1;
		int repeat = 0;

		SubPart() {}
		SubPart(const SubPart& s) { *this = s; }
		void Serialize(Stream& s) { s % sub % cls % repeat; }
		void Jsonize(JsonIO& json) { json("sub", sub)("cls", cls)("repeat", repeat); }
		void operator=(const SubPart& s)
		{
			sub <<= s.sub;
			cls = s.cls;
			repeat = s.repeat;
		}
	};
	struct Part : Moveable<Part> {
		Vector<SubPart> sub;
		int type = -1;
		int num = -1;
		int cls = -1, typeclass = -1, content = -1;

		Part() {}
		Part(const Part& p) { *this = p; }
		void Serialize(Stream& s) { s % sub % type % num % cls % typeclass % content; }
		void Jsonize(JsonIO& json)
		{
			json("sub", sub)("type", type)("num", num)("cls", cls)("tc", typeclass)("c",
			                                                                        content);
		}
		void operator=(const Part& s)
		{
			sub <<= s.sub;
			type = s.type;
			num = s.num;
			cls = s.cls;
			typeclass = s.typeclass;
			content = s.content;
		}
	};
	Vector<Part> parts;

	void Serialize(Stream& s) { s % parts; }

	void Jsonize(JsonIO& json) { json("parts", parts); }
	bool HasAnyClasses() const
	{
		for(const auto& p : parts) {
			if(p.cls >= 0)
				return true;
			for(const auto& s : p.sub) {
				if(s.cls >= 0)
					return true;
				for(const auto& ss : s.sub) {
					if(ss.cls >= 0)
						return true;
				}
			}
		}
		return false;
	}
	double GetNormalScore() const;
};

struct ScriptPostFix {
	struct Weak : Moveable<Weak> {
		int line_i = -1;
		int altline_i = -1;
		String description;
		void Jsonize(JsonIO& json) { json("l0", line_i)("l1", altline_i)("d", description); }
	};
	struct Improvement : Moveable<Improvement> {
		int line_i = -1;
		String text;
		void Jsonize(JsonIO& json) { json("l", line_i)("txt", text); }
	};
	struct Variation : Moveable<Variation> {
		String text;
		Vector<int> scores;
		int ScoreSum() const
		{
			int s = 0;
			for(int i : scores)
				s += i;
			return s;
		}
		double ScoreAv() const
		{
			int s = 0;
			for(int i : scores)
				s += i;
			return (double)s / scores.GetCount();
		}
		void Jsonize(JsonIO& json)
		{
			json("txt", text)("scores", scores);
			if(json.IsLoading())
				scores.SetCount(10, 0);
		}
	};
	Vector<Vector<Weak>> weaks;
	Vector<String> src_lines;
	VectorMap<int, String> src_line_parts;
	Vector<Vector<Improvement>> improvements;
	Vector<Variation> variations;

	void Jsonize(JsonIO& json)
	{
		json("weaks", weaks)("src_lines", src_lines)("src_line_parts", src_line_parts)(
			"improvements", improvements)("variations", variations);
	}
};

struct Script : Component {
	String copyright;
	String description;
	String lang;
	Array<DynPart> parts;
	String __text;

	DynPart* FindPartByName(const String& name);
	int GetFirstPartPosition() const;
	String GetAnyTitle() const;
	String GetText() const;
	String GetUserText() const;
	String GetTextStructure(bool coarse) const;
	void LoadStructuredText(const String& s);
	void LoadStructuredTextExt(const String& s);
	void SetEditText(const String& s);

	Script() {}
	~Script();
	String GetTypename() const override {return "Lyric";}
	const std::type_info& GetType() const override {return typeid(*this);}
	void Store(Entity& a);
	void LoadTitle(Entity& a, String title);
	void Jsonize(JsonIO& json)
	{
		Component::Jsonize(json);
		json("copyright", copyright)("description", description)("lang", lang)(
			"parts", parts)("text", __text);
	}
};

void ReplaceWord(String& s, const String& orig_word, const String& replace_word);
void HotfixReplaceWord(WString& ws);
void HotfixReplaceWord(String& s);

END_UPP_NAMESPACE

#endif
