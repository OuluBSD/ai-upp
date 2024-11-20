#ifndef _AI_TextCore_Browser_h_
#define _AI_TextCore_Browser_h_

NAMESPACE_UPP

class ToolAppCtrl;
class TextDatabase;

struct VMapSumSorter {
	bool operator()(const VectorMap<String, int>& a, const VectorMap<String, int>& b) const
	{
		int asum = 0;
		for(int i : a.GetValues())
			asum += i;
		int bsum = 0;
		for(int i : b.GetValues())
			bsum += i;
		return asum > bsum;
	}
};

class DatabaseBrowser {

public:
	typedef enum : int {
		INVALID = -1,

		ATTR_GROUP,
		ATTR_VALUE,
		COLOR,
		ACTION,
		ACTION_ARG,
		ELEMENT,
		TYPECLASS,
		CONTENT,

		TYPE_COUNT
	} ColumnType;
	using T = ColumnType;

	static String GetTypeString(ColumnType t);

	enum {
#define MODE(x) x,
		DBROWSER_MODE_LIST
#undef MODE

			MODE_COUNT
	};
	static String GetModeKey(int i);
	static String GetModeString(int i);

private:
	VectorMap<String, VectorMap<String, int>> uniq_acts;
	VectorMap<String, VectorMap<String, int>> uniq_attr;
	VectorMap<String, int> uniq_attr_values, uniq_act_args;
	Vector<int> color_counts, rm_list;

	int cursor[TYPE_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};
	ColumnType order[TYPE_COUNT] = {(T)0, (T)1, (T)2, (T)3, (T)4, (T)-1, (T)-1, (T)-1};
	// VectorMap<int,int> uniq_attrs;
	// Vector<int> color_counts;
	VectorMap<hash_t, int> history;
	DatasetPtrs p;

	void RemoveExcessData(int order_i);

public:
	struct Item : Moveable<Item> {
		String str;
		int count;
		int idx = -1;
		bool operator()(const Item& a, const Item& b) const { return a.count > b.count; }
	};

	Vector<Item> items[TYPE_COUNT];
	int mode = -1;
	Vector<int> phrase_parts;
	int sorting = 0;
	bool filter_mid_rhyme = false, filter_end_rhyme = false;
	WString mid_rhyme, end_rhyme;
	double mid_rhyme_distance_limit = 0.005;
	double end_rhyme_distance_limit = 0.005;
	int secondary_category_limit = 10000;

	bool FilterPronounciation(SrcTextData& da, const PhrasePart& pp);

	ToolAppCtrl* ctrl = 0;

public:
	DatabaseBrowser();
	Vector<Item>& Get(ColumnType t);
	bool IsFirstInOrder(ColumnType t) const;
	const Vector<Item>& Get(ColumnType t) const;
	int GetColumnCursor(ColumnType t) const;
	int GetColumnOrder(ColumnType t) const;
	int GetMode() const { return mode; }
	void SetColumnCursor(ColumnType t, int i);
	void FillItems(ColumnType t);
	void SetCtrl(ToolAppCtrl& c) { ctrl = &c; }
	void Init();
	void Update();
	void SetMode(const DatasetPtrs& p, int i);
	static int FindMode(hash_t h);
	static hash_t GetModeHash(int mode);
	void SetMidRhymeFilter(WString wrd, bool up = true);
	void SetEndRhymeFilter(WString wrd, bool up = true);
	void SetMidRhymingLimit(double d, bool up = true);
	void SetEndRhymingLimit(double d, bool up = true);
	void SetInitialData();
	void SetAttrGroup(int i);
	void SetAttrValue(int i);
	void SetColor(int i);
	void SetAction(int i);
	void SetActionArg(int i);
	void SetElement(int i);
	void SetTypeclass(int i);
	void SetContrast(int i);
	void ResetCursor();
	void ResetCursor(int c, ColumnType type);
	void SetCursor(int i, ColumnType t);
	void SetSecondaryCategoryLimit(int i) { secondary_category_limit = i; }
	void DataCursor(int cursor);
	void DataCursorTail(int cursor);
	ColumnType GetCur(int cursor_i) const;
	bool IsSub(int cur, int cursor_i) const;
	double GetMidRhymingLimit() const { return mid_rhyme_distance_limit; }
	double GetEndRhymingLimit() const { return end_rhyme_distance_limit; }
	void SetAll(const DatasetPtrs& p, hash_t sorter, const String& element, const AttrHeader& attr, int clr,
	            const ActionHeader& act, int tc_i, int con_i);
	int FindAction(const String& s);
	int FindArg(const String& s);
	void RealizeUniqueAttrs();
	void RealizeUniqueActions();
	void FilterData(ColumnType t);
	void FilterAll();
	void FilterNextFrom(ColumnType t);
	ColumnType GetOrder(int i)
	{
		ASSERT(i >= 0 && i < (int)TYPE_COUNT);
		return order[i];
	}

	// Mode 0: Attribute - Color - Action
	// Mode 1: Action - Color - Attribute
	// Mode 2: Color - Action - Attribute
	// Mode 3: Color - Action group - Attribute - Action value

	void SortBy(int i);
	void Serialize(Stream& s) { s % history; }
	void Store();
	void Load();

	hash_t GetHash(int columns) const;
	TextDatabase& GetDatabase();

	static DatabaseBrowser& Single(); // TODO rename to Lyric
};

END_UPP_NAMESPACE

#endif
