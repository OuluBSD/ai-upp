#ifndef _AI_TextCore_SourceText_h_
#define _AI_TextCore_SourceText_h_

NAMESPACE_UPP

/*
	Notes:
		- SolverBase based classes can be merged to one, BUT:
			- implement some kind of group of phases to have same limited phases
			- prepare to exit from a group of phases on user request (needs to monitor AI usage)
				- or needs to monitor the estimated time remaining to be short enough
*/
class SourceDataImporter : public SolverBase {

public:
	enum {
		PHASE_TOKENIZE,

		PHASE_COUNT
	};

	bool filter_foreign = true;
	Atomic actual = 0, total = 0;
	TimeStop ts;

public:
	typedef SourceDataImporter CLASSNAME;
	SourceDataImporter();

	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;

	static SourceDataImporter& Get(DatasetPtrs p);

private:
	void Tokenize();
	void Structurize();
};

class SourceAnalysisProcess : public SolverBase {

public:
	enum {
		PHASE_ANALYZE_ARTISTS,
		PHASE_ANALYZE_ELEMENTS,

		PHASE_COUNT,

		PHASE_SUMMARIZE_CONTENT,
	};

	bool filter_foreign = true;
	Atomic actual = 0, total = 0;
	TimeStop ts;
	SourceDataAnalysisArgs args;

public:
	typedef SourceAnalysisProcess CLASSNAME;
	SourceAnalysisProcess();

	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;

	static SourceAnalysisProcess& Get(DatasetPtrs p);

private:
	void AnalyzeArtists();
	void AnalyzeElements();
	void SummarizeContent();
};

class TokenDataProcess : public SolverBase {

public:
	enum {
		PHASE_GET,

		PHASE_COUNT
	};

	int per_action_task = 100;
	int actual = 0, total = 0;
	TokenArgs token_args;

	void Get();

public:
	typedef TokenDataProcess CLASSNAME;
	TokenDataProcess();

	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;

	static TokenDataProcess& Get(DatasetPtrs p);
};

class WordDataProcess : public SolverBase {
	Vector<String> tmp_words;
	int total = 0, actual = 0;
	int lng_i = 0;

	// Params
	int per_batch = 30;

public:
	enum {
		PHASE_WORD_FIX,
		PHASE_WORD_PROCESS,
		PHASE_DETAILS,
		PHASE_SYLLABLES,
		PHASE_COPY_LINKED_DATA,

		PHASE_COUNT
	};

public:
	typedef WordDataProcess CLASSNAME;
	WordDataProcess();

	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;

	static WordDataProcess& Get(DatasetPtrs p);

	void WordFix();
	void WordProcess();
	void Details();
	void Syllables();
	void CopyLinkedData();
};

class TokenPhrasesProcess : public SolverBase {

public:
	enum {
		PHASE_UNKNOWN_PAIRS,

		PHASE_COUNT
	};

	void UnknownPairs();

public:
	typedef TokenPhrasesProcess CLASSNAME;
	TokenPhrasesProcess();

	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;

	static TokenPhrasesProcess& Get(DatasetPtrs p);
};

class AmbiguousWordPairsProcess : public SolverBase {
	TokenArgs token_args;
	Vector<void*> tmp_ptrs;

	// Params
	int per_action_task = 100;

public:
	enum {
		PHASE_GET,

		PHASE_COUNT
	};

	void Get();

public:
	typedef AmbiguousWordPairsProcess CLASSNAME;
	AmbiguousWordPairsProcess();

	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;

	static AmbiguousWordPairsProcess& Get(DatasetPtrs p);
};

class VirtualPhrasesProcess : public SolverBase {
	int actual = 0, total = 0;
	Vector<void*> tmp_ptrs;
	TokenArgs token_args;

	// Params
	int per_action_task = 40;

public:
	enum {
		PHASE_IMPORT_TOKEN_TEXTS,
		PHASE_GET_PARTS,

		PHASE_COUNT
	};

	void ImportTokenTexts();
	void GetParts();

public:
	typedef VirtualPhrasesProcess CLASSNAME;
	VirtualPhrasesProcess();

	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;

	static VirtualPhrasesProcess& Get(DatasetPtrs p);
};

bool GetTypePhrase(Vector<int>& types, const SrcTextData& da, int next_w_i, int w_i,
                   int prev_w_i);

class VirtualPhrasePartsProcess : public SolverBase {
	TokenArgs token_args;
	Vector<void*> tmp_ptrs;

	// Params
	int per_action_task = 65;

public:
	enum {
		PHASE_GET,

		PHASE_COUNT
	};

	void Get();

public:
	typedef VirtualPhrasePartsProcess CLASSNAME;
	VirtualPhrasePartsProcess();

	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;

	static VirtualPhrasePartsProcess& Get(DatasetPtrs p);
};

class VirtualPhraseStructsProcess : public SolverBase {
	int actual = 0;
	
public:
	enum {
		PHASE_GET,
		
		PHASE_COUNT
	};
	
	void Get();

public:
	typedef VirtualPhraseStructsProcess CLASSNAME;
	VirtualPhraseStructsProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static VirtualPhraseStructsProcess& Get(DatasetPtrs p);
	
};

class PhrasePartAnalysisProcess : public SolverBase {
	PhraseArgs phrase_args;
	TokenArgs token_args;
	Vector<void*> tmp_ptrs;
	Vector<int> tmp, tmp_iters;
	VectorMap<int,int> vmap;
	
public:
	enum {
		PHASE_ELEMENT,
		PHASE_COLOR,
		PHASE_ATTR,
		PHASE_ACTIONS,
		PHASE_SCORES,
		PHASE_TYPECLASS,
		PHASE_CONTENT,
		
		PHASE_COUNT
	};
	
	void Do(int fn);
	void OnPhraseElements(String result);
	void OnPhraseColors(String result);
	void OnPhraseAttrs(String result);
	void OnPhraseActions(String result);
	void OnPhraseScores(String result);
	void OnPhraseTypeclasses(String result);
	void OnPhraseContrast(String result);
	void OnPhraseElement(String result);
	
	int BatchCount(int phase) const;
	
public:
	typedef PhrasePartAnalysisProcess CLASSNAME;
	PhrasePartAnalysisProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static PhrasePartAnalysisProcess& Get(DatasetPtrs p);
	
};

class ActionAttrsProcess : public SolverBase {
	VectorMap<String, VectorMap<String, int>> uniq_acts;
	int per_action_clrs = 60;
	int per_action_attrs = 40;
	int actual = 0, total = 0;
	ActionAnalysisArgs args;
	
public:
	enum {
		PHASE_COLORS,
		PHASE_ATTRS,
		
		PHASE_COUNT
	};
	
	void Prepare(int fn);
	void Colors();
	void Attrs();

	int BatchCount(int phase) const;
	
public:
	typedef ActionAttrsProcess CLASSNAME;
	ActionAttrsProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static ActionAttrsProcess& Get(DatasetPtrs p);
	
};

class AttributesProcess : public SolverBase {
	
	struct AttrExtremesBatch : Moveable<AttrExtremesBatch> {
		String group;
	};
	Vector<AttrExtremesBatch> attr_extremes_batches;
	
	struct Batch : Moveable<Batch> {
		AuthorDataset* artist;
		ScriptDataset* scripts;
		String txt;
		int lng_i;
		bool song_begins;
	};
	Vector<Batch> batches;
	
	struct AttrPolarBatch : Moveable<AttrPolarBatch> {
		String group, attr0, attr1;
		Vector<String> attrs;
	};
	Vector<AttrPolarBatch> attr_polar_batches;
	
	struct AttrJoinBatch : Moveable<AttrJoinBatch> {
		Vector<String> groups, values;
	};
	Vector<AttrJoinBatch> attr_join_batches;
	
	
	String tmp_str;
	VectorMap<String,Index<String>> uniq_attrs;
	Index<String> tmp_words, tmp_words2;
	
public:
	enum {
		PHASE_MAIN_GROUPS,
		PHASE_SIMPLIFY_ATTRS,
		PHASE_JOIN_ORPHANED,
		PHASE_FIX_DATA,
		
		PHASE_COUNT
	};
	
	void MainGroups();
	void SimplifyAttrs();
	void JoinOrphaned();
	void FixData();
	void RealizeBatch_AttrExtremesBatch();
	
public:
	typedef AttributesProcess CLASSNAME;
	AttributesProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static AttributesProcess& Get(DatasetPtrs p);
	
};

END_UPP_NAMESPACE

#endif
