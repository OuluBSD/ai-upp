#ifndef _AI_Core_Voiceover_h_
#define _AI_Core_Voiceover_h_


NAMESPACE_UPP

class VoiceoverProcess : public SolverBase {
	
public:
	enum {
		PHASE_FIND_NATURAL_PARTS,
		PHASE_CLASSIFY_NATURAL_PARTS,
		PHASE_MAKE_STORYLINE_SUGGESTIONS,
		PHASE_CLASSIFY_PARTS,
		PHASE_GENERATE,
		
		PHASE_COUNT
	};
	
	hash_t hash = 0;
	Event<> WhenUserInput;
	Value params;
	Vector<String> lines;
	Vector<Vector<String>> input_coarse_parts;
	
public:
	typedef VoiceoverProcess CLASSNAME;
	VoiceoverProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static VoiceoverProcess& Get(VfsPath path, Value params);
	
};

INITIALIZE_VALUECOMPONENT(VoiceoverText, METAKIND_ECS_COMPONENT_SCRIPT_VOICEOVER);

END_UPP_NAMESPACE


#endif
