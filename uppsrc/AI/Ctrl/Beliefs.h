#ifndef _AI_Ctrl_Beliefs_h_
#define _AI_Ctrl_Beliefs_h_

NAMESPACE_UPP


class SocialBeliefsCtrl : public ToolAppCtrl {
	Splitter hsplit;
	ArrayCtrl beliefs;
	ArrayCtrl attrs, user;
	WithSocialBelief<Ctrl> info;
	
public:
	typedef SocialBeliefsCtrl CLASSNAME;
	SocialBeliefsCtrl();
	
	void Data() override;
	void DataBelief();
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	void AddBelief();
	void RemoveBelief();
	void OnValueChange();
};

// TODO Rename to SocialBeliefsSolver
class SocialBeliefsProcess : public SolverBase {
	
public:
	enum {
		PHASE_GET_POSITIVE_ATTRS,
		PHASE_GET_NEGATIVE_ATTRS,
		
		PHASE_COUNT
	};
	
	Owner* owner = 0;
	Profile* profile = 0;
	BiographySnapshot* snap = 0;
	
public:
	typedef SocialBeliefsProcess CLASSNAME;
	SocialBeliefsProcess();
	
	int GetPhaseCount() const override;
	void DoPhase() override;
	
	static SocialBeliefsProcess& Get(Profile& p, BiographySnapshot& snap);
	
	
};


END_UPP_NAMESPACE

#endif
