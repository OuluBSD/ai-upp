#ifndef _AI_Core_ScriptText_h_
#define _AI_Core_ScriptText_h_

NAMESPACE_UPP

class ScriptTextProcess : public SolverBase {
	
public:
	enum {
		PHASE_FIND_NATURAL_PARTS,
		PHASE_TRANSCRIPT,
		
		PHASE_COUNT
	};
	
	hash_t hash = 0;
	Event<> WhenUserInput;
	Value params;
	Vector<String> lines;
	Vector<Vector<String>> input_coarse_parts, natural_parts;
	Vector<String> total_summarizations;
	Vector<String> summarizations;
	Vector<Vector<int>> input_splits;
	
	// Params
	int chars_per_coarse_part = 2000;
	
public:
	typedef ScriptTextProcess CLASSNAME;
	ScriptTextProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static ScriptTextProcess& Get(VfsPath path, Value params);
	
};

INITIALIZE_VALUECOMPONENT(ScriptText, METAKIND_ECS_COMPONENT_SCRIPT_TEXT);

END_UPP_NAMESPACE


#endif
