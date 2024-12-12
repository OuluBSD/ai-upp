#ifndef _AI_Ctrl_Perspective_h_
#define _AI_Ctrl_Perspective_h_

NAMESPACE_UPP


struct Perspective : Component
{
	
	COMPONENT_CONSTRUCTOR(Perspective)
	void Serialize(Stream& s) override {TODO}
	void Jsonize(JsonIO& json) override {TODO}
	hash_t GetHashValue() const override {TODO; return 0;}
	static int GetKind() {return METAKIND_ECS_COMPONENT_PERSPECTIVE;}
	
};

INITIALIZE(Perspective);

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
	BiographySnapshot* snap = 0;
	
public:
	typedef PerspectiveProcess CLASSNAME;
	PerspectiveProcess();
	
	int GetPhaseCount() const override;
	void DoPhase() override;
	
	static PerspectiveProcess& Get(Profile& p, BiographySnapshot& snap);
	
	
};



END_UPP_NAMESPACE

#endif
