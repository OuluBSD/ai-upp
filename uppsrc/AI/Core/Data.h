#ifndef _AI_TextCore_Data_h_
#define _AI_TextCore_Data_h_

NAMESPACE_UPP

#if 0
// TODO remove EditorPtrs & use DatasetPtrs
struct Script;
struct DynPart;
struct EditorPtrs {
	Entity*		entity = 0;
	Component*	component = 0;
	Script*		script = 0;
	DynPart*	part = 0;
	int			pkg_cursor = 0;
	int			node_cursor = 0;
	
	void Zero() {memset(this, 0, sizeof(EditorPtrs));}
	
	bool HasComponent() const {return component;}
	
	//int GetActiveTypeclassIndex() const;
	//int GetActiveContentIndex() const;
	int GetActiveScriptIndex() const;
	
	//int GetActiveEntityIndex() const;
	//int GetActiveSnapshotIndex() const;
	//int GetActiveComponentIndex() const;
	
	static EditorPtrs& Single() {static EditorPtrs p; return p;}
};
#endif

struct Token : Moveable<Token> {
	mutable int word_ = -1;

	void Serialize(Stream& d) { d / word_; }
	void Visit(NodeVisitor& v) {v.Ver(1)(1)("w", word_);}
};

struct TokenText : Moveable<TokenText> {
	Vector<int> tokens;
	int virtual_phrase = -1;

	void Serialize(Stream& d) { d % tokens / virtual_phrase; }
	void Visit(NodeVisitor& v) {v.Ver(1)(1)("t",tokens)("vp", virtual_phrase);}
};

struct ExportWord : Moveable<ExportWord> {
	static const int MAX_CLASS_COUNT = 8;

	String spelling;
	WString phonetic;
	int count = 0;
	Color clr;
	int class_count = 0;
	int classes[MAX_CLASS_COUNT];
	int link = -1;

	void Serialize(Stream& s)
	{
		s / spelling / phonetic / count / clr / class_count;
		for(int i = 0; i < MAX_CLASS_COUNT; i++) s / classes[i];
		s / link;
	}
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("spelling", spelling)
			("phonetic", phonetic)
			("count", count)
			("clr", clr)
			("class_count", class_count);
		for(int i = 0; i < MAX_CLASS_COUNT; i++) {
			v("cls" + IntStr(i), classes[i]);
		}
		v("link", link);
	}

	void CopyFrom(const ExportWord& wa, bool reset)
	{
		spelling.Clear();
		phonetic.Clear();
		spelling = wa.spelling;
		phonetic = wa.phonetic;
		clr = wa.clr;
		class_count = wa.class_count;
		for(int i = 0; i < class_count; i++)
			classes[i] = wa.classes[i];
		if(reset) {
			count = 0;
			link = -1;
		}
	}
};

struct WordPairType : Moveable<WordPairType> {
	int from = -1, to = -1;           // word index
	int from_type = -1, to_type = -1; // word class index

	void Serialize(Stream& d) { d / from / to / from_type / to_type;}
	void Visit(NodeVisitor& v) {v.Ver(1)(1)("f",from)("t",to)("ft",from_type)("tt",to_type);}
	
	hash_t GetHashValue() const
	{
		CombineHash c;
		c.Do(from).Put(1).Do(to);
		return c;
	}
};

struct VirtualPhrase : Moveable<VirtualPhrase> {
	Vector<int> word_classes;
	int virtual_phrase_struct = -1;

	void Serialize(Stream& d) { d % word_classes / virtual_phrase_struct; }
	void Visit(NodeVisitor& v) {v.Ver(1)(1)("wc",word_classes)("vps",virtual_phrase_struct);}
	
	hash_t GetHashValue() const
	{
		CombineHash c;
		for(int wc_i : word_classes)
			c.Do(wc_i).Put(1);
		return c;
	}
};

struct VirtualPhrasePart : Moveable<VirtualPhrasePart> {
	Vector<int> word_classes;
	int struct_part_type = -1;
	int count = 0;

	void Serialize(Stream& d) { d % word_classes / struct_part_type / count;}
	void Visit(NodeVisitor& v) {v.Ver(1)(1)("wc",word_classes)("spt",struct_part_type)("c",count);}

	hash_t GetHashValue() const
	{
		CombineHash c;
		for(int wc_i : word_classes)
			c.Do(wc_i).Put(1);
		return c;
	}
};

struct VirtualPhraseStruct : Moveable<VirtualPhraseStruct> {
	Vector<int> virtual_phrase_parts;
	int struct_type = -1;

	void Serialize(Stream& d) { d % virtual_phrase_parts / struct_type; }
	void Visit(NodeVisitor& v) {v.Ver(1)(1)("vps",virtual_phrase_parts)("st",struct_type);}

	hash_t GetHashValue() const
	{
		CombineHash c;
		for(int part : virtual_phrase_parts)
			c.Do(part).Put(1);
		return c;
	}
};

struct PhrasePart : Moveable<PhrasePart> {
	Vector<int> words;
	int tt_i = -1;
	int virtual_phrase_part = -1;
	int attr = -1;
	int el_i = -1;
	Color clr = Black();
	Vector<int> actions;
	Vector<int> typecasts;
	Vector<int> contrasts;
	int scores[SCORE_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	bool HasScores() const
	{
		for(int i = 0; i < SCORE_COUNT; i++)
			if(scores[i] != 0)
				return true;
		return false;
	}
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("words", words)
			("tt_i", tt_i)
			("virtual_phrase_part", virtual_phrase_part)
			("attr", attr)
			("el_i", el_i)
			("clr", clr)
			("actions", actions)
			("typecasts", typecasts)
			("contrasts", contrasts);
		for(int i = 0; i < SCORE_COUNT; i++)
			v("s" + IntStr(i), scores[i]);
	}
	void Serialize(Stream& s)
	{
		s % words / tt_i / virtual_phrase_part / attr / el_i / clr % actions % typecasts % contrasts;
		for(int i = 0; i < SCORE_COUNT; i++) s / scores[i];
	}
	hash_t GetHashValue() const
	{
		CombineHash c;
		for(int w_i : words)
			c.Do(w_i).Put(1);
		return c;
	}
};

struct ScriptSuggestion : Moveable<ScriptSuggestion> {
	struct Part : Moveable<Part> {
		String name;
		Vector<String> lines;
		
	};
	Vector<Part> parts;
	int rank = -1;
	Vector<Vector<Vector<int>>> transfers;
	int scores[2] = {0,0};
	
	String GetText() const;
};

struct PhraseComb : Moveable<PhraseComb> {
	Vector<int> phrase_parts;
	
	void Visit(NodeVisitor& json) {json.Ver(1)(1)("phrase_parts", phrase_parts);}
};

struct TranslatedPhrasePart : Moveable<TranslatedPhrasePart> {
	String phrase;
	int scores[SCORE_COUNT] = {0,0,0,0,0,0,0,0,0,0};
	
	void Visit(NodeVisitor& json) {json.Ver(1)(1)("phrase", phrase); for(int i = 0; i < SCORE_COUNT; i++) json("score[" + IntStr(i) + "]", scores[i]);}
};

struct ExportAttr : Moveable<ExportAttr> {
	int simple_attr = -1, unused = -1;
	int positive = -1, link = -1;
	int count = 0;

	void Serialize(Stream& d) { d / simple_attr / unused / positive / link / count; }
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("s", simple_attr)
			("u", unused)
			("p", positive)
			("l", link)
			("c", count);
	}
};

struct ExportAction : Moveable<ExportAction> {
	int attr = -1;
	Color clr;
	int count = 0;

	void Serialize(Stream& d) { d / attr / clr / count; }
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("a", attr)
			("clr", clr)
			("c", count);
	}
};

struct ExportParallel : Moveable<ExportParallel> {
	int count = 0, score_sum = 0;

	void Serialize(Stream& d) { d / count / score_sum; }
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("c", count)
			("ss", score_sum);
	}
};

struct ExportTransition : Moveable<ExportTransition> {
	int count = 0, score_sum = 0;

	void Serialize(Stream& d) { d / count / score_sum; }
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("c", count)
			("ss", score_sum);
	}
};

struct ExportDepActionPhrase : Moveable<ExportDepActionPhrase> {
	Vector<int> actions;
	Vector<int> next_phrases;
	Vector<Vector<int>> next_scores;
	int first_lines = 0;
	int attr = -1;
	Color clr = Black();

	void Serialize(Stream& d) {
		d % actions % next_phrases % next_scores / first_lines / attr / clr;
	}
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("ac", actions)
			("np", next_phrases)
			("ns", next_scores)
			("fl", first_lines)
			("at", attr)
			("cl", clr);
	}
};

struct ExportWordnet : Moveable<ExportWordnet> {
	static const int MAX_WORDS = 64;
	int word_count = 0;
	int words[MAX_WORDS] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	int word_clr_count = 0;
	Color word_clrs[MAX_WORDS];
	int main_class = -1;
	int attr = -1;
	Color clr;
	int scores[SCORE_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	void Serialize(Stream& s) {
		s / word_count;
		for(int i = 0; i < MAX_WORDS; i++) s / words[i] / word_clrs[i];
		s / word_clr_count / main_class / attr / clr;
		for(int i = 0; i < SCORE_COUNT; i++) s / scores[i];
	}
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("wc", word_count);
		for(int i = 0; i < MAX_WORDS; i++)
			v	("w" + IntStr(i), words[i])
				("wc" + IntStr(i), word_clrs[i]);
		v	("wcc", word_clr_count)
			("mc", main_class)
			("at", attr)
			("clr", clr);
		for(int i = 0; i < SCORE_COUNT; i++)
			v("sc" + IntStr(i), scores[i]);
	}
};

struct ExportSimpleAttr : Moveable<ExportSimpleAttr> {
	int attr_i0 = -1, attr_i1 = -1;

	void Serialize(Stream& d) { d / attr_i0 / attr_i1; }
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("ai0", attr_i0)
			("ai1", attr_i1);
	}
};

ArrayMap<String, Ptr<MetaNodeExt>>& DatasetIndex();

struct ScriptDataset : Moveable<ScriptDataset> {
	String title;
	String text;
	void Serialize(Stream& s) {s / title / text;}
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("name", title)
			("txt", text);
	}
};

struct AuthorDataset : Moveable<AuthorDataset> {
	String name;
	Vector<ScriptDataset> scripts;
	Vector<String> genres;
	
	ScriptDataset& GetAddScript(String title);
	void Serialize(Stream& s) {s % name % scripts % genres;}
	void Visit(NodeVisitor& v) {
		v.Ver(1)
		(1)	("name", name)
			("scripts", scripts, VISIT_VECTOR)
			("genres", genres);
	}
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
		void Visit(NodeVisitor& json) { json.Ver(1)(1)("token_texts", token_texts)("cls", cls); }
		void operator=(const SubSubPart& s)
		{
			token_texts <<= s.token_texts;
			cls = s.cls;
		}
	};
	struct SubPart : Moveable<SubPart> {
		Vector<SubSubPart> sub;
		int cls = -1;
		int repeat = 0; // TODO: should be 'double', but old db from TextTool has int (rewrite dbs)

		SubPart() {}
		SubPart(const SubPart& s) { *this = s; }
		void Serialize(Stream& s) { s % sub % cls % repeat; }
		void Visit(NodeVisitor& json) { json.Ver(1)(1)("sub", sub, VISIT_VECTOR)("cls", cls)("repeat", repeat); }
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
		void Serialize(Stream& s) {
			s % sub;	ASSERT(!s.IsError());
			s % type;	ASSERT(!s.IsError());
			s % num;	ASSERT(!s.IsError());
			s % cls % typeclass % content;
			ASSERT(!s.IsError());
		}
		void Visit(NodeVisitor& json)
		{
			json.Ver(1)(1)("sub", sub, VISIT_VECTOR)("type", type)("num", num)("cls", cls)("tc", typeclass)("c",
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
	void Visit(NodeVisitor& json) { json.Ver(1)(1)("parts", parts, VISIT_VECTOR); }
	
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

struct SrcTextData : EntityData {
	String filepath;
	
	// INPUT
	Vector<AuthorDataset> authors;
	VectorMap<hash_t, ScriptStruct> scripts;
	
	// WORDS
	VectorMap<String, Token> tokens;
	Index<String> word_classes;
	VectorMap<String, ExportWord> words;
	VectorMap<hash_t, WordPairType> ambiguous_word_pairs;
	
	// PHRASES
	VectorMap<hash_t, TokenText> token_texts;
	VectorMap<hash_t, VirtualPhrase> virtual_phrases;
	VectorMap<hash_t, VirtualPhrasePart> virtual_phrase_parts;
	VectorMap<hash_t, VirtualPhraseStruct> virtual_phrase_structs;
	VectorMap<hash_t, PhrasePart> phrase_parts;
	Index<String> struct_part_types;
	Index<String> struct_types;
	VectorMap<String, ExportSimpleAttr> simple_attrs;
	
	// CLASSIFIERS
	Index<String> element_keys;
	VectorMap<AttrHeader, ExportAttr> attrs;
	VectorMap<ActionHeader, ExportAction> actions;
	
	// UNUSED ???
	VectorMap<int, VectorMap<int, ExportParallel>> parallel; // TODO make these work again?
	VectorMap<int, VectorMap<int, ExportTransition>> trans;
	VectorMap<String, ExportDepActionPhrase> action_phrases;
	VectorMap<hash_t, ExportWordnet> wordnets;
	
	// DIAGNOSTICS
	VectorMap<String, String> diagnostics;
	
	struct {
		struct {
			Index<String> labels;
			String name;
		} typeclass;
		struct {
			Vector<ContentType> labels;
			String name;
			Vector<String> parts;
		} content;
		String name;
	} ctx;
	dword lang = LNG_enUS;
	VectorMap<String,Vector<String>> typeclass_entities[TCENT_COUNT];
	
	const Vector<ContentType>& GetContents() {return ctx.content.labels;}
	const Vector<String>& GetContentParts() {return ctx.content.parts;}
	//const Index<String>& GetTypeclasses() {return typeclasses;}
	int GetContentCount() {return ctx.content.labels.GetCount();}
	//int GetTypeclassCount() {return typeclasses.GetCount();}
	int GetLanguage() const {return lang;}
	
	AuthorDataset& GetAddAuthor(String name);
	String GetTokenTypeString(const TokenText& txt) const;
	String GetWordString(const Vector<int>& words) const;
	WString GetWordPronounciation(const Vector<int>& words) const;
	String GetTypeString(const Vector<int>& word_classes) const;
	String GetActionString(const Vector<int>& actions) const;
	String GetScriptDump(int i) const;
	Value GetScriptValue(int i) const;
	VectorMap<int, int> GetSortedElements();
	VectorMap<int, int> GetSortedElementsOfPhraseParts();
	
	String GetTokenTextString(const TokenText& txt) const;
	String GetTokenTextString(const Vector<int>& tokens) const;
	void Serialize(Stream& s);
	int GetKind() const override {return METAKIND_ECS_VIRTUAL_VALUE_SRCTEXT;}
	void Visit(NodeVisitor& s) override;
};


// see SRC_TXT_HEADER_ENABLE

struct SrcTxtHeader : Component {
	Time written;
	int64 size = 0;
	String sha1;
	Vector<String> files;
	void Visit(NodeVisitor& v) override;
	String GetName() const override {return "Source Database";}
	
	String filepath;
	One<SrcTextData> data;
	
	SrcTxtHeader(MetaNode& owner) : Component(owner) {}
	~SrcTxtHeader();
	SrcTextData& Data() {if (data.IsEmpty()) data.Create(); return *data;}
	void RealizeData();
	bool LoadData();
	String SaveData();
	
	static int GetKind() {return METAKIND_DATABASE_SOURCE;}
	
};

INITIALIZE(SrcTxtHeader);

END_UPP_NAMESPACE

#endif
