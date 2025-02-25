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
		PHASE_ANALYZE_ARTISTS,
		PHASE_ANALYZE_ELEMENTS,
		// 3
		PHASE_GET,
		
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
	
	// 1
	void Tokenize();
	// 2
	void AnalyzeArtists();
	void AnalyzeElements();
	// 3
	void Get();
	
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
