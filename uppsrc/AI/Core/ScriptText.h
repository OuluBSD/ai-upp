#ifndef _AI_Core_ScriptText_h_
#define _AI_Core_ScriptText_h_

NAMESPACE_UPP

class ScriptTextProcess : public SolverBase {
	
public:
	enum {
		// 0
		PHASE_INPUT,
		// 1
		PHASE_TOKENIZE,
		// 2
		PHASE_ANALYZE_PUBLIC_FIGURE,
		PHASE_ANALYZE_ELEMENTS,
		// 3
		PHASE_WORD_CLASSES,
		// 4
		PHASE_AMBIGUOUS_WORD_PAIRS,
		// 5
		PHASE_IMPORT_TOKEN_TEXTS,
		PHASE_CLASSIFY_SENTENCES,
		// 6
		PHASE_VIRTUAL_PHRASE_PARTS,
		// 7
		PHASE_VIRTUAL_PHRASE_PART_STRUCTS,
		// 8
		PHASE_ELEMENT,
		PHASE_COLOR,
		PHASE_ATTR,
		PHASE_ACTIONS,
		PHASE_SCORES,
		PHASE_TYPECLASS,
		PHASE_CONTENT,
		// 9
		PHASE_COLORS,
		PHASE_ATTRS,
		// 10
		PHASE_MAIN_GROUPS,
		PHASE_SIMPLIFY_ATTRS,
		PHASE_JOIN_ORPHANED,
		PHASE_FIX_DATA,
		
		PHASE_COUNT
	};
	
	hash_t hash = 0;
	Event<> WhenUserInput;
	Value params;
	SrcTextData* data = 0;
	One<NaturalTokenizer> tk;
	
	// Params
	TaskArgs args;
	
	// Configuration
	int words_per_action_task = 100;
	
	// Temp (per phase)
	int total = 0, actual = 0;
	TimeStop ts;
	Vector<const WordPairType*> tmp_wp_ptrs;
	Vector<VirtualPhrasePart*> tmp_vpp_ptrs;
	
	// 1
	void Tokenize();
	// 2
	void AnalyzePublicFigure();
	void AnalyzeElements();
	// 3
	void WordClasses();
	// 4
	void AmbiguousWordPairs();
	// 5
	void ImportTokenTexts();
	void ClassifySentences();
	// 6
	void VirtualPhraseParts();
	// 7
	void VirtualPhraseStructs();
	// 8
	void PhrasePartAnalysis();
	// 9
	void Prepare(int fn);
	void Colors();
	void Attrs();
	// 10
	void MainGroups();
	void SimplifyAttrs();
	void JoinOrphaned();
	void FixData();
	void RealizeBatch_AttrExtremesBatch();
	
public:
	typedef ScriptTextProcess CLASSNAME;
	ScriptTextProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static ScriptTextProcess& Get(DatasetPtrs p, VfsPath path, Value params, SrcTextData& data, Event<> WhenStopped);
	
};

INITIALIZE_VALUECOMPONENT(ScriptText, METAKIND_ECS_COMPONENT_SCRIPT_TEXT);

END_UPP_NAMESPACE


#endif
