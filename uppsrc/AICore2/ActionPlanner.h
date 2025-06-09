#ifndef _AICore2_ActionPlanner_h_
#define _AICore2_ActionPlanner_h_


class ActionPlanner;
class ActionNode;


class Action : public Moveable<Action> {
	
protected:
	friend class ActionPlanner;
	friend class ActionNode;
	friend class ActionPlannerWrapper;
	
	BinaryWorldState precond, postcond;
	double cost;
	
public:

	Action();
};


class ActionPlannerWrapper;

class ActionPlanner {
	
protected:
	friend class ActionNode;
	friend class ActionPlannerWrapper;
	
	int atom_count = 0;
	Vector<Action> acts;
	ActionPlannerWrapper* wrapper = 0;
	
	Array<BinaryWorldState> search_cache;
	
public:
	ActionPlanner();
	
	
	void Clear();
	
	int GetActionCount() const {return acts.GetCount();}
	int GetAtomCount() const {return atom_count;}
	
	void AddSize(int action_count, int atom_count);
	void SetSize(int action_count, int atom_count);
	bool SetPreCondition(int action_id, int atom_id, bool value);
	bool SetPostCondition(int action_id, int atom_id, bool value);
	bool SetCost(int action_id, int cost );
	
	
	void DoAction( int action_id, const BinaryWorldState& src, BinaryWorldState& dest);
	void GetPossibleStateTransition(const BinaryWorldState& src, Array<BinaryWorldState*>& dest, Vector<int>& act_ids, Vector<double>& action_costs);
	
};

class ActionPlannerWrapper {
	ActionPlanner& ap;
	Vector<String> atoms, acts;
	
protected:
	friend class ActionPlanner;
	
	void OnResize();
	
public:
	ActionPlannerWrapper(ActionPlanner& planner);
	
	int GetAtomIndex(String atom_name);
	int GetActionIndex(String action_name);
	String GetAtomName(int i) {return atoms[i];}
	String GetActionName(int i) {return acts[i];}
	String GetWorldstateDescription( const BinaryWorldState& ws );
	String GetDescription();
	
	void SetAction(int act_i, String s) {acts[act_i] = s;}
	void SetAtom(int atom_i, String s) {atoms[atom_i] = s;}
	bool SetPreCondition(String action_name, String atom_name, bool value);
	bool SetPostCondition(String action_name, String atom_name, bool value);
	bool SetCost(String action_name, int cost );
	
};

class ActionNode : public VfsValueExt {
	BinaryWorldState* ws;
	double cost;
	int act_id;
	hash_t hash = 0;
	
	ActionPlanner* ap = 0;
	ActionNode* goal = 0;
	ArrayMap<hash_t, ActionNode*> tmp_sub;
	
public:
	CLASSTYPE(ActionNode)
	void Visit(Vis& v) override {}
	
	
	ActionNode(VfsValue& n);
	
	ActionPlanner& GetActionPlanner() {return *ap;}
	BinaryWorldState& GetWorldState() {return *ws;}
	
	void SetActionPlanner(ActionPlanner& ap_) {ap = &ap_;}
	void SetGoal(ActionNode& ws) {goal = &ws;}
	
	void SetWorldState(BinaryWorldState& ws) {this->ws = &ws; hash = ws.GetHashValue();}
	inline void SetCost(double d) {cost = d;}
	inline void SetActionId(int i) {act_id = i;}
	
	double GetDistance(VfsValue& to) override;
	double GetEstimate() override;
	void GenerateSubValues(NodeRoute& route) override;
	bool TerminalTest() override;
	inline double GetCost() const {return cost;}
	inline int GetActionId() const {return act_id;}
};

typedef Node<ActionNode> APlanNode;




#endif
