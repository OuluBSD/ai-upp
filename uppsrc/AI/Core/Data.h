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
};

struct TokenText : Moveable<TokenText> {
	Vector<int> tokens;
	int virtual_phrase = -1;

	void Serialize(Stream& d) { d % tokens / virtual_phrase; }
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
	
	void Jsonize(JsonIO& json) {json("phrase_parts", phrase_parts);}
};

struct TranslatedPhrasePart : Moveable<TranslatedPhrasePart> {
	String phrase;
	int scores[SCORE_COUNT] = {0,0,0,0,0,0,0,0,0,0};
	
	void Jsonize(JsonIO& json) {json("phrase", phrase); for(int i = 0; i < SCORE_COUNT; i++) json("score[" + IntStr(i) + "]", scores[i]);}
};

struct ExportAttr : Moveable<ExportAttr> {
	int simple_attr = -1, unused = -1;
	int positive = -1, link = -1;
	int count = 0;

	void Serialize(Stream& d) { d / simple_attr / unused / positive / link / count; }
};

struct ExportAction : Moveable<ExportAction> {
	int attr = -1;
	Color clr;
	int count = 0;

	void Serialize(Stream& d) { d / attr / clr / count; }
};

struct ExportParallel : Moveable<ExportParallel> {
	int count = 0, score_sum = 0;

	void Serialize(Stream& d) { d / count / score_sum; }
};

struct ExportTransition : Moveable<ExportTransition> {
	int count = 0, score_sum = 0;

	void Serialize(Stream& d) { d / count / score_sum; }
};

struct ExportDepActionPhrase : Moveable<ExportDepActionPhrase> {
	Vector<int> actions;
	Vector<int> next_phrases;
	Vector<Vector<int>> next_scores;
	int first_lines = 0;
	int attr = -1;
	Color clr = Black();

	void Serialize(Stream& d)
	{
		d % actions % next_phrases % next_scores / first_lines / attr / clr;
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
};

struct ExportSimpleAttr : Moveable<ExportSimpleAttr> {
	int attr_i0 = -1, attr_i1 = -1;

	void Serialize(Stream& d) { d / attr_i0 / attr_i1; }
};

ArrayMap<String, Ptr<MetaNodeExt>>& DatasetIndex();

struct ScriptDataset : Moveable<ScriptDataset> {
	String name;
	String text;
	void Serialize(Stream& s) {s / name / text;}
};

struct EntityDataset : Moveable<EntityDataset> {
	String name;
	Vector<ScriptDataset> scripts;
	Vector<String> genres;
	
	
	void Serialize(Stream& s) {s % name % scripts % genres;}
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

struct SrcTextData : Pte<SrcTextData> {
	String filepath;
	VectorMap<hash_t, ScriptStruct> scripts;
	VectorMap<String, Token> tokens;
	VectorMap<hash_t, TokenText> token_texts;
	Index<String> element_keys;
	Index<String> word_classes;
	VectorMap<String, ExportWord> words;
	VectorMap<hash_t, WordPairType> ambiguous_word_pairs;
	VectorMap<hash_t, VirtualPhrase> virtual_phrases;
	VectorMap<hash_t, VirtualPhrasePart> virtual_phrase_parts;
	VectorMap<hash_t, VirtualPhraseStruct> virtual_phrase_structs;
	VectorMap<hash_t, PhrasePart> phrase_parts;
	Index<String> struct_part_types;
	Index<String> struct_types;
	VectorMap<AttrHeader, ExportAttr> attrs;
	VectorMap<ActionHeader, ExportAction> actions;
	VectorMap<int, VectorMap<int, ExportParallel>> parallel; // TODO make these work again?
	VectorMap<int, VectorMap<int, ExportTransition>> trans;
	VectorMap<String, ExportDepActionPhrase> action_phrases;
	VectorMap<hash_t, ExportWordnet> wordnets;
	VectorMap<String, String> diagnostics;
	VectorMap<String, ExportSimpleAttr> simple_attrs;
	Vector<EntityDataset> entities; // TODO rename, remove src_ (source-data for analysis)
	Index<String> typeclasses;
	Vector<ContentType> contents;
	Vector<String> content_parts;
	dword lang = LNG_enUS;
	VectorMap<String,Vector<String>> typeclass_entities[TCENT_COUNT];
	
	const Vector<ContentType>& GetContents() {return contents;}
	const Vector<String>& GetContentParts() {return content_parts;}
	const Index<String>& GetTypeclasses() {return typeclasses;}
	int GetContentCount() {return contents.GetCount();}
	int GetTypeclassCount() {return typeclasses.GetCount();}
	int GetLanguage() const {return lang;}
	
	String GetTokenTypeString(const TokenText& txt) const;
	String GetWordString(const Vector<int>& words) const;
	WString GetWordPronounciation(const Vector<int>& words) const;
	String GetTypeString(const Vector<int>& word_classes) const;
	String GetActionString(const Vector<int>& actions) const;
	String GetScriptDump(int i) const;
	VectorMap<int, int> GetSortedElements();
	VectorMap<int, int> GetSortedElementsOfPhraseParts();
	
	String GetTokenTextString(const TokenText& txt) const;
	void Serialize(Stream& s) {
		int v = 1; s % v;
		if (v >= 1) {
			s % scripts % tokens;
			s % token_texts;
			s % element_keys;
			s % word_classes;
			s % words;
			s % ambiguous_word_pairs;
			s % virtual_phrases;
			s % virtual_phrase_parts;
			s % virtual_phrase_structs;
			s % phrase_parts;
			s % struct_part_types;
			s % struct_types;
			s % attrs;
			s % actions;
			s % parallel;
			s % trans;
			s % action_phrases;
			s % wordnets;
			s % diagnostics;
			s % simple_attrs;
			s % entities;
			s % typeclasses;
			s % contents;
			s % content_parts;
			s % lang;
			for(int i = 0; i < TCENT_COUNT; i++)
				s % typeclass_entities[i];
		}
	}
	
};

struct SrcTxtHeader : MetaNodeExt {
	Time written;
	int64 size = 0;
	String sha1;
	Vector<String> files;
	void Visit(NodeVisitor& v) override {v.Ver(1)(1)("written",written)("size",size)("sha1",sha1)("files",files);}
	String GetName() const override {return "Source Database";}
	
	String filepath;
	One<SrcTextData> data;
	
	SrcTxtHeader(MetaNode& owner) : MetaNodeExt(owner) {}
	SrcTextData& Data() {if (data.IsEmpty()) data.Create(); return *data;}
	void RealizeData();
	bool LoadData();
	String SaveData();
};


END_UPP_NAMESPACE

#endif
