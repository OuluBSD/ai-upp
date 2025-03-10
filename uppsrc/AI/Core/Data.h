#ifndef _AI_TextCore_Data_h_
#define _AI_TextCore_Data_h_

NAMESPACE_UPP

typedef int ContentIdx;
inline int GetContentMain(ContentIdx i) {return i / 3;}
inline int GetContentSub(ContentIdx i) {return i % 3;}

typedef enum : int {
	TXT_INVALID = -1,
	TXT_NORMAL,
	TXT_PRE_REPEAT,
	TXT_REPEAT,
	TXT_TWIST,
	TXT_NULL,

	TXT_COUNT
} TextPartType;

#define CONTEXTLIST \
	CTX(0,CREATIVITY, Creativity, "Is the context focused on originality and expression?") \
	CTX(1,TECHNICALITY, Technicality, "Is there an emphasis on technical precision or functionality?") \
	CTX(2,EMOTIONALITY, Emotionality, " Does the context evoke emotions or aim to connect on an emotional level?") \
	CTX(3,EFFICIENCY, Efficiency, "Is the context concerned with optimization and performance?") \
	CTX(4,COLLABORATIVE, Collaborative, "Does the context involve teamwork or community involvement?") \
	CTX(5,STABILITY, Stability, "Is there a focus on reliability and consistency?") \
	CTX(6,INNOVATIVE, Innovative, "Is there an intention to introduce new ideas or techniques?") \
	CTX(7,EXPERIMENTAL, Experimental, "Is the context about trial and error or trying unproven methods?")

struct ContextType : Moveable<ContextType> {
	byte value = 0;
	
	typedef enum {
		#define CTX(a,b,c,d) b,
		CONTEXTLIST
		#undef CTX
		CTX_COUNT
	} Feature;
	
	typedef enum {
		#define CTX(a,b,c,d) b##_BIT = 1 << a,
		CONTEXTLIST
		#undef CTX
	} Bit;
	
	static ContextType Lyrical();
	static ContextType Programming();
	static ContextType PublicShortMessage();
	static ContextType PersonalBlog();
	static ContextType CorporateBlog();
	static ContextType Dialog();
	static ContextType Storyboard();
	
	bool operator==(const ContextType& t) const {return value;}
	hash_t GetHashValue() const {return value;}
	void Visit(NodeVisitor& vis) {vis("value", value);}
};

struct ContextData : Moveable<ContextData> {
	struct Typeclass : Moveable<Typeclass> {
		String name;
		Vector<Vector<String>> entities;
		void Visit(NodeVisitor& v) {v.Ver(1)(1)("n",name)("e",entities);}
	};
	struct Content : Moveable<Content> {
		String name;
		Vector<String> parts;
		void Visit(NodeVisitor& v) {v.Ver(1)(1)("n",name)("p",parts);}
	};
	String name;
	Vector<Typeclass> typeclasses;
	Vector<Content> contents;
	Vector<String> part_names;
	Index<String> entity_groups;
	
	int FindAddEntityGroup(String s);
	void Visit(NodeVisitor& vis) {
		vis.Ver(1)
		(1)	("name", name)
			("typeclasses", typeclasses, VISIT_VECTOR)
			("contents", contents, VISIT_VECTOR)
			("part_names", part_names)
			("entity_groups", entity_groups)
			;
	}
};

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
	mutable int word_ = -1; // TODO remove

	void Serialize(Stream& d) { d / word_; }
	void Visit(NodeVisitor& v) {
		v.Ver(2)
		(1)("w", word_);
	}
};

struct TokenText : Moveable<TokenText> {
	Vector<int> tokens;
	int virtual_phrase = -1;
	int phrase_part = -1;

	void Serialize(Stream& d) { d % tokens / virtual_phrase; }
	void Visit(NodeVisitor& v) {
		v.Ver(2)
		(1)("t",tokens)("vp", virtual_phrase)
		(2)("pp",phrase_part);
	}
	static hash_t GetHash(const Vector<int>& tokens) {CombineHash c; for (int tk : tokens) c.Do(tk); return c;}
	hash_t GetHashValue() const {return GetHash(tokens);}
};

struct ExportWord : Moveable<ExportWord> {
	static const int MAX_CLASS_COUNT = 8;

	String spelling;
	WString phonetic;
	int count = 0;
	Color clr;
	int class_count = 0;
	int classes[MAX_CLASS_COUNT];
	int link = -1; // TODO Remove
	byte lang = 0xFF;
	
	void Serialize(Stream& s)
	{
		s / spelling / phonetic / count / clr / class_count;
		for(int i = 0; i < MAX_CLASS_COUNT; i++) s / classes[i];
		s / link;
	}
	void Visit(NodeVisitor& v) {
		v.Ver(2)
		(1)	("spelling", spelling)
			("phonetic", phonetic)
			("count", count)
			("clr", clr)
			("class_count", class_count);
		for(int i = 0; i < MAX_CLASS_COUNT; i++) {
			v("cls" + IntStr(i), classes[i]);
		}
		v("link", link);
		v(2)("lang",lang);
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
	
	static hash_t GetHash(const Vector<int>& word_classes)
	{
		CombineHash c;
		for(int wc_i : word_classes)
			c.Do(wc_i);
		return c;
	}
	hash_t GetHashValue() const {return GetHash(word_classes);}
};

struct VirtualPhrasePart : Moveable<VirtualPhrasePart> {
	Vector<int> word_classes;
	int struct_part_type = -1;
	int count = 0;

	void Serialize(Stream& d) { d % word_classes / struct_part_type / count;}
	void Visit(NodeVisitor& v) {v.Ver(1)(1)("wc",word_classes)("spt",struct_part_type)("c",count);}

	static hash_t GetHash(const Vector<int>& word_classes)
	{
		CombineHash c;
		for(int wc_i : word_classes)
			c.Do(wc_i).Put(1);
		return c;
	}
	hash_t GetHashValue() const {return GetHash(word_classes);}
};

struct VirtualPhraseStruct : Moveable<VirtualPhraseStruct> {
	Vector<int> virtual_phrase_parts;
	int struct_type = -1;

	void Serialize(Stream& d) { d % virtual_phrase_parts / struct_type; }
	void Visit(NodeVisitor& v) {v.Ver(1)(1)("vps",virtual_phrase_parts)("st",struct_type);}

	static hash_t GetHash(const Vector<int>& virtual_phrase_parts)
	{
		CombineHash c;
		for(int part : virtual_phrase_parts)
			c.Do(part).Put(1);
		return c;
	}
	hash_t GetHashValue() const
	{
		return GetHash(virtual_phrase_parts);
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
	byte lang = 0xFF;
	byte ctx = 0;

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
		v(2)("lang", lang)("ctx", ctx);
	}
	void Serialize(Stream& s)
	{
		s % words / tt_i / virtual_phrase_part / attr / el_i / clr % actions % typecasts % contrasts;
		for(int i = 0; i < SCORE_COUNT; i++) s / scores[i];
	}
	static hash_t GetHash(const Vector<int>& words)
	{
		CombineHash c;
		for(int w_i : words)
			c.Do(w_i).Put(1);
		return c;
	}
	hash_t GetHashValue() const {return GetHash(words);}
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
	static hash_t GetHash(const Vector<int>& words)
	{
		CombineHash c;
		for(int w_i : words)
			c.Do(w_i).Put(1);
		return c;
	}
	hash_t GetHashValue() const {
		CombineHash c;
		for(int i = 0; i < word_count; i++)
			c.Do(words[i]).Put(1);
		return c;
	}
};

struct ExportSimpleAttr : Moveable<ExportSimpleAttr> {
	int attr_i0 = -1, attr_i1 = -1;

	void Serialize(Stream& d) { d / attr_i0 / attr_i1; } // Todo remove
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
	ContextType ctx;
	void Serialize(Stream& s) {s / title / text;} // Todo remove
	void Visit(NodeVisitor& v) {
		v.Ver(2)
		(1)	("name", title)
			("txt", text)
		(2)	("ctx", ctx, VISIT_NODE);
	}
};

struct AuthorDataset : Moveable<AuthorDataset> {
	String name;
	Vector<ScriptDataset> scripts;
	Vector<String> genres;
	ContextType ctx;
	
	ScriptDataset& GetAddScript(String title);
	void Serialize(Stream& s) {s % name % scripts % genres;}
	void Visit(NodeVisitor& v) {
		v.Ver(2)
		(1)	("name", name)
			("scripts", scripts, VISIT_VECTOR)
			("genres", genres)
		(2)	("ctx", ctx, VISIT_NODE);
	}
};

struct ScriptStruct : Moveable<ScriptStruct> {
	ScriptStruct() {}
	ScriptStruct(const ScriptStruct& s) { *this = s; }
	ScriptStruct(ScriptStruct&& s) { *this = s; }
	void operator=(const ScriptStruct& s) { parts <<= s.parts; }

	struct SubSubPart : Moveable<SubSubPart> {
		Vector<int> token_texts;
		int el_i = -1;

		SubSubPart() {}
		SubSubPart(const SubSubPart& s) { *this = s; }
		void Serialize(Stream& s) { s % token_texts % el_i; }
		void Visit(NodeVisitor& json) { json.Ver(1)(1)("token_texts", token_texts)("cls", el_i); }
		void operator=(const SubSubPart& s)
		{
			token_texts <<= s.token_texts;
			el_i = s.el_i;
		}
	};
	struct SubPart : Moveable<SubPart> {
		Vector<SubSubPart> sub;
		int el_i = -1;
		int repeat = 0; // TODO remove (see below)
		double repeat_ = 0;

		SubPart() {}
		SubPart(const SubPart& s) { *this = s; }
		// TODO remove Serialize and int repeat
		void Serialize(Stream& s) { s % sub % el_i % repeat; if (s.IsLoading() && repeat && !repeat_) repeat_ = repeat;}
		void Visit(NodeVisitor& v) {
			v.Ver(2);
			if (v.file_ver == 1) {
				int repeat;
				v(1)("sub", sub, VISIT_VECTOR)("cls", el_i)("repeat", repeat);
				repeat_ = repeat;
			}
			else {
				v(2)("sub", sub, VISIT_VECTOR)("el_i", el_i)("repeat", repeat_);
				if (v.IsLoading()) repeat = repeat_;
			}
		}
		void operator=(const SubPart& s)
		{
			sub <<= s.sub;
			el_i = s.el_i;
			repeat = s.repeat;
			repeat_ = s.repeat_;
		}
	};
	struct Part : Moveable<Part> {
		Vector<SubPart> sub;
		TextPartType type = TXT_INVALID;
		int num = -1;
		int el_i = -1, typeclass = -1;
		ContentIdx content = -1;

		Part() {}
		Part(const Part& p) { *this = p; }
		void Serialize(Stream& s) {
			s % sub;	ASSERT(!s.IsError());
			s % (int&)type;	ASSERT(!s.IsError());
			s % num;	ASSERT(!s.IsError());
			s % el_i % typeclass % content;
			ASSERT(!s.IsError());
		}
		void Visit(NodeVisitor& v)
		{
			v.Ver(1)
			(1)	("sub", sub, VISIT_VECTOR)
				("type", (int&)type)
				("num", num)
				("cls", el_i)
				("tc", typeclass)
				("c", (int&)content);
		}
		void operator=(const Part& s)
		{
			sub <<= s.sub;
			type = s.type;
			num = s.num;
			el_i = s.el_i;
			typeclass = s.typeclass;
			content = s.content;
		}
	};
	Vector<Part> parts;
	String author, title;
	ContextType ctx;

	void Serialize(Stream& s) { s % parts; }
	void Visit(NodeVisitor& json) {
		json.Ver(3)
		(1)("parts", parts, VISIT_VECTOR)
		(2)("author", author)("title", title)
		(3)("ctx", ctx, VISIT_NODE);
	}
	
	bool HasAnyClasses() const
	{
		for(const auto& p : parts) {
			if(p.el_i >= 0)
				return true;
			for(const auto& s : p.sub) {
				if(s.el_i >= 0)
					return true;
				for(const auto& ss : s.sub) {
					if(ss.el_i >= 0)
						return true;
				}
			}
		}
		return false;
	}
	double GetNormalScore() const;
};

struct WordData : Moveable<WordData> {
	byte lang = 0xFF;
	String text;
	String spelling;
	int count = 0;
	Color clr;
	int word_class = -1;
	int translation = -1;
	
	static hash_t GetHash(byte lang, const String& text, int word_class) {
		CombineHash ch;
		ch.Put(lang).Do(text).Do(word_class);
		return ch;
	}
	hash_t GetHashValue() const {return GetHash(lang, text, word_class);}
	void Visit(NodeVisitor& vis) {
		vis.Ver(1)
		(1)	("lang", lang)
			("text", text)
			("spelling", spelling)
			("count", count)
			("clr", clr)
			("word_class", word_class)
			("translation", translation)
			;
	}
};

struct WordTranslation : Moveable<WordTranslation> {
	static const int MAX_CLASS_COUNT = 8;
	
	Vector<int> translations;
	
	void Visit(NodeVisitor& vis) {
		vis.Ver(1)
		(1)	("t", translations);
	}
};

struct SrcTextData : EntityData {
	String filepath;
	
	// INPUT
	Vector<AuthorDataset> authors;
	VectorMap<hash_t, ScriptStruct> scripts;
	
	// WORDS
	VectorMap<String, Token> tokens;
	Index<String> word_classes;
	VectorMap<String, ExportWord> words; // TODO remove
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
	
	// TRANSITION VALUES
	VectorMap<hash_t, ExportWordnet> wordnets;
	VectorMap<String, ExportDepActionPhrase> action_phrases;
	VectorMap<int, VectorMap<int, ExportTransition>> trans;
	VectorMap<int, VectorMap<int, ExportParallel>> parallel;
	
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
	} ctx; // TODO remove
	dword lang = LNG_enUS; // TODO remove
	
	// ????
	VectorMap<String,Vector<String>> typeclass_entities[TCENT_COUNT]; // TODO remove
	
	// NEXT VERSION
	/* note:
			make guis: parallel, trans, action_phrases, wordnets...
			remove:
					words
					ctx
					lang
					attrs, actions
					tokens
			convert:
					AttrHeader -> AttrIdx
						--> use new AttrText vector for attribute strings (bc clr etc.)
					ActionHeader -> ActionIdx
						--> use new ActionText vector for action strings (bc clr etc.)
					element_keys->
						---> use ElementText for element strings (bc clr etc.)
	*/
	ArrayMap<ContextType, ContextData> ctxs;
	Vector<WordData> words_;
	VectorMap<String, VectorMap<hash_t,VectorMap<int,int>>> langwords;
	Vector<WordTranslation> translations;
	
	
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
	int version = 1;
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
