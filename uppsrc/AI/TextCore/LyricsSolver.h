#ifndef _AI_TextCore_LyricsSolver_h_
#define _AI_TextCore_LyricsSolver_h_

NAMESPACE_UPP


class PartLineCtrl;

struct NavigatorState {
	PartLineCtrl* line = 0;
	int depth = -1;
	LineElement* el = 0;
	hash_t sorter = 0;
	String element;
	AttrHeader attr;
	int clr_i = -1;
	ActionHeader act;
	int typeclass_i = -1;
	int con_i = -1;
	void Clear();
	void RemoveDuplicate(const NavigatorState& s);
};

void ReadNavigatorState(Script& s, int part_i, int sub_i, int line_i, NavigatorState& state, int depth_limit);



class ScriptSolver : public SolverBase {
	enum {
		LS_FILL_LINES,
		LS_COMPARISON,
		LS_TITLE,
		
		LS_COUNT,
		
		LS_MATCH_REFERENCE,
		LS_SCORE_MATCH,
		LS_FILL_REFERENCE_MATCH,
		LS_SMOOTH_REFERENCE_MATCH,
		
	};
	Entity* artist = 0;
	Script* script = 0;
	
	// params
	double dist_limit = 0.005;
	int primary_count = 50;
	int rhyming_list_count = 5;
	int sugg_limit = 6;
	bool start_post_solver = false;
	
	// temp
	int per_batch = 0;
	//Vector<VectorMap<int,double>> phrase_parts;
	Vector<String> phrases;
	Vector<Tuple2<int,int>> matches;
	Index<int> remaining;
	VectorMap<String,int> part_sizes;
	//ComponentAnalysis* sa = 0;
	Vector<int> phrase_src;
	int active_part = -1;
	Index<hash_t> visited;
	Event<> WhenPartiallyReady;
	DynPart* tmp_part = 0;
	DynSub* tmp_sub = 0;
	DynLine* tmp_line = 0;
	Vector<const DynLine*> tmp_lines;
	
	struct ConvTask : Moveable<ConvTask> {
		Vector<String> from, ref;
		String part;
	};
	Vector<ConvTask> conv_tasks;
	Index<String> added_phrases;
	
	void Process();
	void ClearScript();
	void ProcessFillLines();
	void ProcessPrimary();
	void ProcessMakeHoles();
	void ProcessComparison();
	void ProcessReference();
	void ProcessScoreMatch();
	void ProcessTitle();
	void ProcessFillReferenceMatch();
	void ProcessSmoothReferenceMatch();
	
	void OnProcessPrimary(String res);
	void OnProcessFillLines(String res);
	void OnProcessMakeHoles(String res);
	void OnProcessComparison(String res);
	void OnProcessComparisonFail(int loser);
	void OnProcessReference(String res);
	void OnProcessScoreMatch(String res);
	void OnProcessFillReferenceMatch(String res);
	void OnProcessSmoothReferenceMatch(String res);
	
	//TextDatabase& GetDatabase() {return GetAppModeDatabase(appmode);}
	//int GetTypeclassCount() {return TextLib::GetTypeclassCount(appmode);}
	//int GetContentCount() {return TextLib::GetContentCount(appmode);}
	void CopyState(ScriptSolverArgs::State& to, const NavigatorState& from);
	
public:
	typedef ScriptSolver CLASSNAME;
	ScriptSolver();
	
	int GetPhaseCount() const override;
	void DoPhase() override;
	static ScriptSolver& Get(const DatasetPtrs& p, const String& ecs_path);
	
	void GetExpanded(int part_i, int sub_i, int line_i, Event<> WhenPartiallyReady);
	void GetSuggestions2(int part_i, int sub_i, const Vector<const DynLine*>& lines, Event<> WhenPartiallyReady);
	void GetStyleSuggestion(int part_i, int sub_i, const Vector<const DynLine*>& lines, Event<> WhenPartiallyReady);
	void GetSuggestions(const DynPart& part, const DynSub& sub, const Vector<const DynLine*>& lines, Event<> WhenPartiallyReady);
	void GetSubStory(int part_i, int sub_i, Event<> WhenPartiallyReady);
	void GetPartStory(int part_i, Event<> WhenPartiallyReady);
	
	
	void StartPostSolver(bool b=true) {start_post_solver = b;}
	
	Callback2<int,int> WhenProgress;
	
};


END_UPP_NAMESPACE

#endif
