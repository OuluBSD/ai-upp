#ifndef _AICore2_ActionPlanner_h_
#define _AICore2_ActionPlanner_h_


class ActionPlanner;
class ActionNode;

class WorldState : public Moveable<WorldState> {

protected:
	friend class ActionPlanner;
	friend class ActionPlannerWrapper;
	friend class ActionNode;
	
	Vector<bool> values, using_act;
	
public:
	
	WorldState();
	void Clear();
	
	bool Set(int index, bool value);
	
	WorldState& operator = (const WorldState& src);
	
	hash_t GetHashValue();
	
};


class Action : public Moveable<Action> {
	
protected:
	friend class ActionPlanner;
	friend class ActionNode;
	friend class ActionPlannerWrapper;
	
	WorldState precond, postcond;
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
	
	Array<WorldState> search_cache;
	
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
	
	
	void DoAction( int action_id, const WorldState& src, WorldState& dest);
	void GetPossibleStateTransition(const WorldState& src, Array<WorldState*>& dest, Vector<int>& act_ids, Vector<double>& action_costs);
	
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
	String GetWorldstateDescription( const WorldState& ws );
	String GetDescription();
	
	void SetAction(int act_i, String s) {acts[act_i] = s;}
	void SetAtom(int atom_i, String s) {atoms[atom_i] = s;}
	bool SetPreCondition(String action_name, String atom_name, bool value);
	bool SetPostCondition(String action_name, String atom_name, bool value);
	bool SetCost(String action_name, int cost );
	
};

class ActionNode : public MetaNodeExt {
	WorldState* ws;
	double cost;
	int act_id;
	hash_t hash = 0;
	
	ActionPlanner* ap = 0;
	ActionNode* goal = 0;
	ArrayMap<hash_t, ActionNode*> tmp_sub;
	
public:
	CLASSTYPE(ActionNode)
	void Visit(Vis& v) override {}
	
	
	ActionNode(MetaNode& n);
	
	ActionPlanner& GetActionPlanner() {return *ap;}
	WorldState& GetWorldState() {return *ws;}
	
	void SetActionPlanner(ActionPlanner& ap_) {ap = &ap_;}
	void SetGoal(ActionNode& ws) {goal = &ws;}
	
	void SetWorldState(WorldState& ws) {this->ws = &ws; hash = ws.GetHashValue();}
	inline void SetCost(double d) {cost = d;}
	inline void SetActionId(int i) {act_id = i;}
	
	double GetDistance(MetaNode& to) override;
	double GetEstimate() override;
	bool TerminalTest(NodeRoute& route) override;
	inline double GetCost() const {return cost;}
	inline int GetActionId() const {return act_id;}
};

typedef Node<ActionNode> APlanNode;




#endif
