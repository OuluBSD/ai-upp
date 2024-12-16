#ifndef _AI_Ctrl_Perspective_h_
#define _AI_Ctrl_Perspective_h_

NAMESPACE_UPP


class PerspectiveCtrl : public ComponentCtrl {
	Splitter hsplit;
	ArrayCtrl beliefs;
	ArrayCtrl attrs, user;
	WithSocialBelief<Ctrl> info;
	
public:
	typedef PerspectiveCtrl CLASSNAME;
	PerspectiveCtrl();
	
	void Data() override;
	void DataBelief();
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	void AddBelief();
	void RemoveBelief();
	void OnValueChange();
};

INITIALIZE(PerspectiveCtrl)

// TODO Rename to PerspectiveSolver
class PerspectiveProcess : public SolverBase {
	
public:
	enum {
		PHASE_GET_POSITIVE_ATTRS,
		PHASE_GET_NEGATIVE_ATTRS,
		
		PHASE_COUNT
	};
	
	Owner* owner = 0;
	Profile* profile = 0;
	BiographyPerspectives* snap = 0;
	
public:
	typedef PerspectiveProcess CLASSNAME;
	PerspectiveProcess();
	
	int GetPhaseCount() const override;
	void DoPhase() override;
	
	static PerspectiveProcess& Get(Profile& p, BiographyPerspectives& snap);
	
	
};



END_UPP_NAMESPACE

#endif
