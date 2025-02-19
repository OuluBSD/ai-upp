#ifndef _AI_Core_ScriptText_h_
#define _AI_Core_ScriptText_h_

NAMESPACE_UPP

class ScriptTextProcess : public SolverBase {
	
public:
	enum {
		PHASE_INPUT,
		PHASE_TOKENIZE,
		
		PHASE_COUNT
	};
	
	hash_t hash = 0;
	Event<> WhenUserInput;
	Value params;
	SrcTextData* data = 0;
	One<NaturalTokenizer> tk;
	
	// Params
	
	// Temp (per phase)
	int total = 0, actual = 0;
	TimeStop ts;
	
	void Tokenize();
	
public:
	typedef ScriptTextProcess CLASSNAME;
	ScriptTextProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static ScriptTextProcess& Get(DatasetPtrs p, VfsPath path, Value params, SrcTextData& data, Event<> WhenReady);
	
};

INITIALIZE_VALUECOMPONENT(ScriptText, METAKIND_ECS_COMPONENT_SCRIPT_TEXT);

END_UPP_NAMESPACE


#endif
