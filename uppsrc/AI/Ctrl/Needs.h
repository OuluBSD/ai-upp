#ifndef _AI_Ctrl_Needs_h_
#define _AI_Ctrl_Needs_h_

NAMESPACE_UPP


struct SocialNeeds : Component
{
	
	COMPONENT_CONSTRUCTOR(SocialNeeds)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_SOCIAL_NEEDS;}
	
};

INITIALIZE(SocialNeeds)

class SocialNeedsCtrl : public ComponentCtrl {
	Splitter hsplit, rolesplit, platsplit, eventsplit;
	ArrayCtrl roles, needs, causes, messages;
	ArrayCtrl platforms, actions, action_causes;
	ArrayCtrl events, entries;
	DocEdit event, entry;
	
public:
	typedef SocialNeedsCtrl CLASSNAME;
	SocialNeedsCtrl();
	
	void Data() override;
	void DataRole();
	void DataNeed();
	void DataAction();
	void DataEvent();
	void DataEntry();
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	
};

INITIALIZE(SocialNeedsCtrl)

class SocialNeedsProcess : public SolverBase {
	enum {
		PHASE_PACK_ROLE_REACTIONS,
		PHASE_PACK_PLATFORM_REACTIONS,
		PHASE_PLATFORM_DESCRIPTIONS,
		PHASE_PLATFORM_DESCRIPTION_REFINEMENTS,
		PHASE_PLATFORM_DESCRIPTION_TRANSLATED,
		
		PHASE_COUNT
	};
	Owner* owner = 0;
	Profile* profile = 0;
	BiographyAnalysis* analysis = 0;
	Biography* biography = 0;
	BiographySnapshot* snap = 0;
	
	
	struct Range : Moveable<Range> {
		int off = 0, len = 0;
		int input[2] = {0,0};
		String ToString() const {return Format("%d, %d: %d, %d", off, len, input[0], input[1]);}
	};
	Vector<Range> ranges;
	
	
	int CreateRange(int off, int len);
	void ProcessRoleReactions();
	void ProcessPlatformReactions();
	void ProcessPlatformDescriptions();
	void ProcessPlatformDescriptionRefinements();
	void ProcessPlatformDescriptionTranslated();
	void OnProcessRoleReactions(String res);
	void OnProcessPlatformReactions(String res);
	void OnProcessPlatformDescriptions(String res);
	void OnProcessPlatformDescriptionRefinements(String res);
	void OnProcessPlatformDescriptionTranslated(String res);
	
public:
	typedef SocialNeedsProcess CLASSNAME;
	SocialNeedsProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	void OnBatchError() override;
	
	static SocialNeedsProcess& Get(Profile& p, BiographySnapshot& snap);
	
	Callback2<int,int> WhenProgress;
	
};


END_UPP_NAMESPACE

#endif
