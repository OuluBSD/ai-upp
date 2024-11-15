#ifndef _AI_TextCore_Data_h_
#define _AI_TextCore_Data_h_

NAMESPACE_UPP

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
	
	int GetActiveEntityIndex() const;
	int GetActiveSnapshotIndex() const;
	int GetActiveComponentIndex() const;
	
	static EditorPtrs& Single() {static EditorPtrs p; return p;}
};

struct Token : Moveable<Token> {
	mutable int word_ = -1;

	void Serialize(Stream& d) { d % word_; }
};

struct TokenText : Moveable<TokenText> {
	Vector<int> tokens;
	int virtual_phrase = -1;

	void Serialize(Stream& d) { d % tokens % virtual_phrase; }
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

	void Serialize(Stream& d)
	{
		d % spelling % phonetic % count % clr % link;
		for(int i = 0; i < MAX_CLASS_COUNT; i++)
			d % classes[i];
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

	void Serialize(Stream& d) { d % from % to % from_type % to_type; }

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

	void Serialize(Stream& d) { d % word_classes % virtual_phrase_struct; }

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

	void Serialize(Stream& d) { d % word_classes % struct_part_type % count; }

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

	void Serialize(Stream& d) { d % virtual_phrase_parts % struct_type; }

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
	void Serialize(Stream& d)
	{
		d % words % tt_i % virtual_phrase_part % attr % clr % actions % typecasts %
			contrasts % el_i;
		for(int i = 0; i < SCORE_COUNT; i++) d % scores[i];
	}
	hash_t GetHashValue() const
	{
		CombineHash c;
		for(int w_i : words)
			c.Do(w_i).Put(1);
		return c;
	}
};

struct ExportAttr : Moveable<ExportAttr> {
	int simple_attr = -1, unused = -1;
	int positive = -1, link = -1;
	int count = 0;

	void Serialize(Stream& d) { d % simple_attr % unused % positive % link % count; }
};

struct ExportAction : Moveable<ExportAction> {
	int attr = -1;
	Color clr;
	int count = 0;

	void Serialize(Stream& d) { d % attr % clr % count; }
};

struct ExportParallel : Moveable<ExportParallel> {
	int count = 0, score_sum = 0;

	void Serialize(Stream& d) { d % count % score_sum; }
};

struct ExportTransition : Moveable<ExportTransition> {
	int count = 0, score_sum = 0;

	void Serialize(Stream& d) { d % count % score_sum; }
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
		d % actions % next_phrases % next_scores % first_lines % attr % clr;
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

	void Serialize(Stream& d) { TODO /*d % words % word_clrs % scores;*/ } // TODO fix
};

struct ExportSimpleAttr : Moveable<ExportSimpleAttr> {
	int attr_i0 = -1, attr_i1 = -1;

	void Serialize(Stream& d) { d % attr_i0 % attr_i1; }
};


typedef enum : int {
	DBFIELD_NULL,
	DBFIELD_SRCTEXT,
	
	DBFIELD_COUNT
} DbField;

Vector<DbField> GetDependencies(DbField t);

struct DatasetField {
	DbField type = DBFIELD_NULL;
	String filepath;
	Vector<String> dep_filepaths; // dependencies
	
	virtual ~DatasetField() {}
};

ArrayMap<String, DatasetField>& DatasetIndex();

struct SrcTextData : DatasetField, Pte<SrcTextData> {
	VectorMap<uint64, ScriptStruct> scripts;
	VectorMap<String, Token> tokens;
	
	static DbField GetFieldType() {return DBFIELD_SRCTEXT;}
	
};

struct DatasetPtrs {
	Ptr<SrcTextData>		src;
	
};

struct DatasetAnalysis {
	
	VectorMap<hash_t, TokenText> token_texts;
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
	VectorMap<int, VectorMap<int, ExportParallel>> parallel; // TODO does these work again?
	VectorMap<int, VectorMap<int, ExportTransition>> trans;
	VectorMap<String, ExportDepActionPhrase> action_phrases;
	// VectorMap<String,String>			translations[LNG_COUNT];
	VectorMap<hash_t, ExportWordnet> wordnets;
	VectorMap<String, String> diagnostics;
	VectorMap<String, ExportSimpleAttr> simple_attrs;
	// VectorMap<hash_t,String>			phrase_translations[LNG_COUNT];
	Index<String> element_keys;

	DatasetAnalysis();
	void Serialize(Stream& s);
	void Jsonize(JsonIO& json) {TODO} // depreceated
	void Load();
	String GetTokenTextString(const TokenText& txt) const;
	String GetTokenTypeString(const TokenText& txt) const;
	String GetWordString(const Vector<int>& words) const;
	WString GetWordPronounciation(const Vector<int>& words) const;
	String GetTypeString(const Vector<int>& word_classes) const;
	String GetActionString(const Vector<int>& actions) const;
	String GetScriptDump(int i) const;
	String GetScriptDump(DatasetAnalysis& da, int i) const;
	VectorMap<int, int> GetSortedElements();
	VectorMap<int, int> GetSortedElementsOfPhraseParts();
};

struct EntityAnalysis : Moveable<EntityAnalysis> {
	Vector<String> genres;

	void Serialize(Stream& s) { s % genres; }
	void Jsonize(JsonIO& json) { json("genres", genres); }
};

struct ScriptDataset : Moveable<ScriptDataset> {
	String name;
	String text;
};

struct EntityDataset : Moveable<EntityDataset> {
	String name;
	Vector<ScriptDataset> scripts;
};

struct SourceDataAnalysis {
	DatasetAnalysis dataset;
	VectorMap<String, EntityAnalysis> entities;

	void Jsonize(JsonIO& json)
	{
		if(json.IsLoading()) {
			ArrayMap<String, DatasetAnalysis> datasets;
			json("datasets", datasets)("entities", entities);
			int i = datasets.Find("en");
			if(i >= 0) {
				byte tmp[sizeof(dataset)];
				memcpy(tmp, &datasets[i], sizeof(dataset));
				memcpy(&datasets[i], &dataset, sizeof(dataset));
				memcpy(&dataset, tmp, sizeof(dataset));
			}
			else {
				json("dataset", dataset);
			}
		}
		else {
			json("dataset", dataset)("entities", entities);
		}
	}

	void Serialize(Stream& s) { TODO s % dataset % entities; } // TODO separate files
	void StoreJson();
	void LoadJson();
	void Store();
	void Load();
};

class TextDatabase {

public:
	Array<Entity> entities;             // user-data
	Vector<EntityDataset> src_entities; // source-data for analysis
	// TODO rename 'a'
	SourceDataAnalysis a; // analysis of the source-data
	Vector<ContentType> contents;
	Vector<String> content_parts;
	Index<String> typeclasses;
	
	const Vector<ContentType>& GetContents() {return contents;}
	const Vector<String>& GetContentParts() {return content_parts;}
	const Index<String>& GetTypeclasses() {return typeclasses;}
	int GetContentCount() {return contents.GetCount();}
	int GetTypeclassCount() {return typeclasses.GetCount();}
	
	static TextDatabase& Single() {static TextDatabase db; return db;}
};

END_UPP_NAMESPACE

#endif
