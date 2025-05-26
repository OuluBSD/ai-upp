#include "AICore.h"


NAMESPACE_UPP

BinaryWorldState::BinaryWorldState() {
	
	Clear();
}

void BinaryWorldState::Clear() {
	values.Clear();
	using_act.Clear();
}

bool BinaryWorldState::Set(int index, bool value) {
	if (index < 0) return false;
	if (using_act.GetCount() <= index) {
		using_act.SetCount(index+1, false);
		values.SetCount(index+1, false);
	}
	using_act[index] = true;
	values[index] = value;
	return true;
}

BinaryWorldState& BinaryWorldState::operator = (const BinaryWorldState& src) {
	values <<= src.values;
	using_act <<= src.using_act;
	return *this;
}


hash_t BinaryWorldState::GetHashValue() const {
	CombineHash c;
	for(int i = 0; i < values.GetCount(); i++) {
		c.Put(using_act[i]);
		c.Put(values[i]);
	}
	return c;
}






Action::Action() : cost(1.0) {
	
}






ActionPlanner::ActionPlanner() {
	
}


void ActionPlanner::Clear() {
	atom_count = 0;
	acts.Clear();
	search_cache.Clear();
}

void ActionPlanner::AddSize(int action_count, int atom_count) {
	ASSERT(action_count >= 0 && atom_count >= 0);
	this->atom_count += atom_count;
	int new_action_count = acts.GetCount() + action_count;
	acts.SetCount(new_action_count);
	if (wrapper)
		wrapper->OnResize();
}

void ActionPlanner::SetSize(int action_count, int atom_count) {
	this->atom_count = atom_count;
	acts.SetCount(action_count);
	if (wrapper)
		wrapper->OnResize();
}

void ActionPlanner::DoAction( int action_id, const BinaryWorldState& src, BinaryWorldState& dest) {
	const BinaryWorldState& post = acts[action_id].postcond;
	
	dest = src;
	
	for(int i = 0; i < post.using_act.GetCount(); i++) {
		if (post.using_act[i]) {
			dest.Set(i, post.values[i]);
		}
	}
}


void ActionPlanner::GetPossibleStateTransition(const BinaryWorldState& src, Array<BinaryWorldState*>& dest, Vector<int>& act_ids, Vector<double>& action_costs)
{
	for ( int i=0; i < acts.GetCount(); ++i )
	{
		// Check precondition
		Action& act = acts[i];
		const BinaryWorldState& pre = act.precond;
		
		// Check that precondition is not using something outside of src values
		int count = UPP::min(pre.using_act.GetCount(), src.using_act.GetCount());
		bool fail = false;
		for(int j = count; j < pre.using_act.GetCount(); j++)
			if (pre.using_act[j] && pre.values[j])
				{fail = true; break;}
		if (fail)
			continue;
		
		bool met = true;
		for(int j = 0; j < count; j++) {
			int a = pre.using_act[j];
			if (a && src.values[j] != pre.values[j]) {
				met = false;
				break;
			}
		}
		
		if (met) {
			act_ids.Add(i);
			action_costs.Add(act.cost);
			BinaryWorldState& tmp = search_cache.Add();
			DoAction( i, src, tmp );
			dest.Add(&tmp);
		}
	}
}

bool ActionPlanner::SetPreCondition(int act_idx, int atm_idx, bool value)
{
	if ( act_idx == -1 || atm_idx == -1 ) return false;
	acts[act_idx].precond.Set(atm_idx, value);
	return true;
}


bool ActionPlanner::SetPostCondition(int act_idx, int atm_idx, bool value)
{
	if ( act_idx == -1 || atm_idx == -1 ) return false;
	acts[act_idx].postcond.Set(atm_idx, value);
	return true;
}


bool ActionPlanner::SetCost(int act_idx, int cost )
{
	if ( act_idx == -1 ) return false;
	acts[ act_idx ].cost = cost;
	return true;
}








ActionPlannerWrapper::ActionPlannerWrapper(ActionPlanner& planner) : ap(planner) {
	ap.wrapper = this;
	OnResize();
}

void ActionPlannerWrapper::OnResize() {
	acts.SetCount(ap.GetActionCount());
	atoms.SetCount(ap.GetAtomCount());
}

String ActionPlannerWrapper::GetWorldstateDescription( const BinaryWorldState& ws )
{
	String str;
	for(int i = 0; i < atoms.GetCount(); i++) {
		if (ws.using_act.GetCount() <= i) break;
		if (ws.using_act[i]) {
			bool set = ws.values[i];
			if (set)
				str += ToUpper(atoms[i]) + ",";
			else
				str += atoms[i] + ",";
		}
	}
	return str;
}


String ActionPlannerWrapper::GetDescription()
{
	String str;
	for(int j = 0; j < ap.acts.GetCount(); j++) {
		Action& act = ap.acts[j];
		str += acts[j] + ":\n";
		
		BinaryWorldState& pre  = act.precond;
		BinaryWorldState& post = act.postcond;
		
		int count = UPP::min(atoms.GetCount(), pre.values.GetCount());
		for(int i = 0; i < count; i++) {
			bool v = pre.values[i];
			str += " " + atoms[i] + "==" + IntStr(v) + "\n";
		}
		
		count = UPP::min(atoms.GetCount(), post.values.GetCount());
		for(int i = 0; i < count; i++) {
			bool v = post.values[i];
			str += " " + atoms[i] + "==" + IntStr(v) + "\n";
		}
	}
	return str;
}


int ActionPlannerWrapper::GetAtomIndex(String atom_name) {
	int i = VectorFind(atoms, atom_name);
	if (i != -1)
		return i;
	atoms.Add(atom_name);
	return atoms.GetCount()-1;
}


int ActionPlannerWrapper::GetActionIndex(String action_name) {
	int i = VectorFind(acts, action_name);
	if (i != -1)
		return i;
	acts.Add(action_name);
	return acts.GetCount()-1;
}


bool ActionPlannerWrapper::SetPreCondition(String action_name, String atom_name, bool value)
{
	int act_idx = GetActionIndex( action_name );
	int atm_idx = GetAtomIndex( atom_name );
	if ( act_idx == -1 || atm_idx == -1 ) return false;
	ap.acts[act_idx].precond.Set(atm_idx, value);
	return true;
}


bool ActionPlannerWrapper::SetPostCondition(String action_name, String atom_name, bool value)
{
	int act_idx = GetActionIndex( action_name );
	int atm_idx = GetAtomIndex( atom_name );
	if ( act_idx == -1 || atm_idx == -1 ) return false;
	ap.acts[act_idx].postcond.Set(atm_idx, value);
	return true;
}


bool ActionPlannerWrapper::SetCost(String action_name, int cost )
{
	int act_idx = GetActionIndex( action_name );
	if ( act_idx == -1 ) return false;
	ap.acts[ act_idx ].cost = cost;
	return true;
}










ActionNode::ActionNode(VfsValue& n) : VfsValueExt(n) {
	cost = 0;
	act_id = -1;
}

bool ActionNode::TerminalTest(NodeRoute& route) {
	if (this->GetEstimate() <= 0)
		return true;
	ASSERT(goal);
	BinaryWorldState& ws = this->GetWorldState();
	ActionNode* root = node.FindRoot<ActionNode>();
	ASSERT(root);
	if (!root) return true;
	ActionPlanner& ap = root->GetActionPlanner();
	Array<BinaryWorldState*> to;
	Vector<int> act_ids;
	Vector<double> action_costs;
	ap.GetPossibleStateTransition(ws, to, act_ids, action_costs);
	for(int i = 0; i < to.GetCount(); i++) {
		BinaryWorldState& ws_to = *to[i];
		hash_t hash = ws_to.GetHashValue();
		int j = root->tmp_sub.Find(hash);
		if (j == -1) {
			APlanNode sub = root->val.add<ActionNode>();
			sub->SetWorldState(ws_to);
			sub->SetCost(action_costs[i]);
			sub->SetActionId(act_ids[i]);
			sub->SetGoal(*goal);
			auto& link = val.sub.Add();
			link.symbolic_link = sub.n;
			root->tmp_sub.Add(hash, sub);
		} else {
			auto& link = val.sub.Add();
			link.symbolic_link = &root->tmp_sub[j]->node;
		}
	}
	return !node.GetCount();
}

double ActionNode::GetDistance(VfsValue& n) {
	ActionNode& to = *CastPtr<ActionNode>(&*n.ext);
	double dist = 0;
	
	Vector<bool>& values = ws->values;
	const Vector<bool>& to_values = to.ws->values;
	
	Vector<bool>& using_act = ws->using_act;
	const Vector<bool>& to_using_act = to.ws->using_act;
	
	int count = UPP::min(values.GetCount(), to_values.GetCount());
	
	for(int j = count; j < to_using_act.GetCount(); j++)
		if (to_using_act[j] && to_values[j])
			dist += 1;
	
	for(int j = 0; j < count; j++) {
		int b = to_using_act[j];
		if (b &&  values[j] != to_values[j]) {
			dist += 1; continue;}
	}
	return dist;
}

double ActionNode::GetEstimate() {
	ASSERT(goal);
	return GetDistance(goal->node);
}

END_UPP_NAMESPACE
