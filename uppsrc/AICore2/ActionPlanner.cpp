#include "AICore.h"


NAMESPACE_UPP

BinaryWorldState::BinaryWorldState() {
	
	Clear();
}

void BinaryWorldState::Clear() {
	atom_values.Clear();
	using_atom.Clear();
}

bool BinaryWorldState::Set(int index, bool value) {
	if (index < 0) return false;
	if (using_atom.GetCount() <= index) {
		using_atom.SetCount(index+1, false);
		atom_values.SetCount(index+1, false);
	}
	using_atom[index] = true;
	atom_values[index] = value;
	return true;
}

BinaryWorldState& BinaryWorldState::operator = (const BinaryWorldState& src) {
	atom_values <<= src.atom_values;
	using_atom <<= src.using_atom;
	session = src.session;
	return *this;
}

bool BinaryWorldState::operator==(const BinaryWorldState& src) const {
	if (session != src.session)
		return false;
	int count = min(src.using_atom.GetCount(), using_atom.GetCount());
	for(int i = count; i < src.using_atom.GetCount(); i++)
		if (src.using_atom[i])
			return false;
	for(int i = count; i < using_atom.GetCount(); i++)
		if (using_atom[i])
			return false;
	for(int i = 0; i < count; i++) {
		if (src.using_atom[i] != using_atom[i] ||
			src.atom_values[i] != atom_values[i])
			return false;
	}
	return true;
}

hash_t BinaryWorldState::GetHashValue() const {
	CombineHash c;
	int last_i = atom_values.GetCount()-1;
	while (last_i >= 0) {
		if (using_atom[last_i])
			break;
		last_i--;
	}
	for(int i = 0; i <= last_i; i++) {
		bool b = using_atom[last_i];
		c.Put(b);
		if (b)
			c.Put(atom_values[i]);
	}
	return c;
}

Value BinaryWorldState::ToValue() const {
	ValueMap map;
	int i = 0;
	for (bool b : this->using_atom) {
		if (b)
			map.Add(i, (int)this->atom_values[i]);
		i++;
	}
	return map;
}


bool BinaryWorldState::FromValue(Value v, Event<String> WhenError) {
	ASSERT(session);
	if (!session) {Clear(); return false;}
	ValueMap ws = v;
	if (v.Is<ValueMap>()) {
		ws = v;
	}
	else {
		WhenError("unexpected value type " + v.GetTypeName());
		return false;
	}
	
	if (ws.IsEmpty()) {
		WhenError("empty world state map");
		return false;
	}
	
	Value first_key = ws.GetKey(0);
	if (first_key.Is<int>()) {
		for(int i = 0; i < ws.GetCount(); i++) {
			int j = ws.GetKey(i);
			int v = ws.GetValue(i);
			Set(j, v);
		}
	}
	else if (first_key.Is<String>()) {
		for(int i = 0; i < ws.GetCount(); i++) {
			String atom_name = ws.GetKey(i);
			Value v = ws.GetValue(i);
			int j = session->atoms.Find(atom_name);
			if (j < 0) {WhenError("the goal atom can't be found: " + atom_name); return false;}
			auto& atom = session->atoms[j];
			atom.goal = v;
			Set(j, v);
		}
	}
	else {
		LOG(AsJSON(v, true));
		ASSERT(0);
		return false;
	}
	
	return true;
}

String BinaryWorldState::ToString() const {
	String s;
	for(int i = 0; i < this->using_atom.GetCount(); i++) {
		if (using_atom[i]) {
			if (session)
				s << session->atoms.GetKey(i) << ": ";
			else
				s << i << ": ";
			s << (this->atom_values[i] ? "true" : "false");
			s << "\n";
		}
	}
	if (s.IsEmpty())
		s = "<null worldstate>";
	return s;
}







PlannerEvent::PlannerEvent() : cost(1.0) {
	
}






ActionPlanner::ActionPlanner() {
	
}


void ActionPlanner::Clear() {
	atom_count = 0;
	events.Clear();
	search_cache.Clear();
}

void ActionPlanner::AddSize(int action_count, int atom_count) {
	ASSERT(action_count >= 0 && atom_count >= 0);
	this->atom_count += atom_count;
	int new_action_count = events.GetCount() + action_count;
	events.SetCount(new_action_count);
	if (wrapper)
		wrapper->OnResize();
}

void ActionPlanner::SetSize(int action_count, int atom_count) {
	this->atom_count = atom_count;
	events.SetCount(action_count);
	if (wrapper)
		wrapper->OnResize();
}

void ActionPlanner::DoAction( int action_id, const BinaryWorldState& src, BinaryWorldState& dest) {
	const BinaryWorldState& post = events[action_id].postcond;
	
	dest = src;
	
	for(int i = 0; i < post.using_atom.GetCount(); i++) {
		if (post.using_atom[i]) {
			dest.Set(i, post.atom_values[i]);
		}
	}
}


void ActionPlanner::GetPossibleStateTransition(const BinaryWorldState& src, Array<BinaryWorldState*>& dest, Vector<int>& act_ids, Vector<double>& action_costs)
{
	for ( int i=0; i < events.GetCount(); ++i )
	{
		// Check precondition
		auto& e = events[i];
		const BinaryWorldState& pre = e.precond;
		
		// Check that precondition is not using something outside of src values
		int count = UPP::min(pre.using_atom.GetCount(), src.using_atom.GetCount());
		bool fail = false;
		for(int j = count; j < pre.using_atom.GetCount(); j++)
			if (pre.using_atom[j] && pre.atom_values[j])
				{fail = true; break;}
		if (fail)
			continue;
		
		bool met = true;
		for(int j = 0; j < count; j++) {
			int a = pre.using_atom[j];
			if (a && src.atom_values[j] != pre.atom_values[j]) {
				met = false;
				break;
			}
		}
		
		if (met) {
			act_ids.Add(i);
			action_costs.Add(e.cost);
			BinaryWorldState& tmp = search_cache.Add();
			DoAction( i, src, tmp );
			dest.Add(&tmp);
		}
	}
}

bool ActionPlanner::SetPreCondition(int ev_idx, int atm_idx, bool value)
{
	if ( ev_idx == -1 || atm_idx == -1 ) return false;
	events[ev_idx].precond.Set(atm_idx, value);
	return true;
}


bool ActionPlanner::SetPostCondition(int ev_idx, int atm_idx, bool value)
{
	if ( ev_idx == -1 || atm_idx == -1 ) return false;
	events[ev_idx].postcond.Set(atm_idx, value);
	return true;
}


bool ActionPlanner::SetCost(int ev_idx, int cost )
{
	if ( ev_idx == -1 ) return false;
	events[ ev_idx ].cost = cost;
	return true;
}








ActionPlannerWrapper::ActionPlannerWrapper(ActionPlanner& planner) : ap(planner) {
	ap.wrapper = this;
	OnResize();
}

void ActionPlannerWrapper::OnResize() {
	acts.SetCount(ap.GetEventCount());
	atoms.SetCount(ap.GetAtomCount());
}

String ActionPlannerWrapper::GetWorldstateDescription( const BinaryWorldState& ws )
{
	String str;
	for(int i = 0; i < atoms.GetCount(); i++) {
		if (ws.using_atom.GetCount() <= i) break;
		if (ws.using_atom[i]) {
			bool set = ws.atom_values[i];
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
	for(int j = 0; j < ap.events.GetCount(); j++) {
		auto& e = ap.events[j];
		str += acts[j] + ":\n";
		
		BinaryWorldState& pre  = e.precond;
		BinaryWorldState& post = e.postcond;
		
		int count = UPP::min(atoms.GetCount(), pre.atom_values.GetCount());
		for(int i = 0; i < count; i++) {
			bool v = pre.atom_values[i];
			str += " " + atoms[i] + "==" + IntStr(v) + "\n";
		}
		
		count = UPP::min(atoms.GetCount(), post.atom_values.GetCount());
		for(int i = 0; i < count; i++) {
			bool v = post.atom_values[i];
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


int ActionPlannerWrapper::GetEventIndex(String event_name) {
	int i = VectorFind(acts, event_name);
	if (i != -1)
		return i;
	acts.Add(event_name);
	return acts.GetCount()-1;
}


bool ActionPlannerWrapper::SetPreCondition(String event_name, String atom_name, bool value)
{
	int ev_idx = GetEventIndex( event_name );
	int atm_idx = GetAtomIndex( atom_name );
	if ( ev_idx == -1 || atm_idx == -1 ) return false;
	ap.events[ev_idx].precond.Set(atm_idx, value);
	return true;
}


bool ActionPlannerWrapper::SetPostCondition(String event_name, String atom_name, bool value)
{
	int ev_idx = GetEventIndex( event_name );
	int atm_idx = GetAtomIndex( atom_name );
	if ( ev_idx == -1 || atm_idx == -1 ) return false;
	ap.events[ev_idx].postcond.Set(atm_idx, value);
	return true;
}


bool ActionPlannerWrapper::SetCost(String event_name, int cost )
{
	int ev_idx = GetEventIndex( event_name );
	if ( ev_idx == -1 ) return false;
	ap.events[ ev_idx ].cost = cost;
	return true;
}










ActionNode::ActionNode(VfsValue& n) : VfsValueExt(n) {
	cost = 0;
	act_id = -1;
}

bool ActionNode::GenerateSubValues(const Value& params, NodeRoute& route) {
	ASSERT(goal);
	BinaryWorldState& ws = this->GetWorldState();
	ActionNode* root = val.FindRoot<ActionNode>();
	ASSERT(root);
	if (!root) return false;
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
			APlanNode sub = root->val.Add<ActionNode>();
			sub->SetWorldState(ws_to);
			sub->SetCost(action_costs[i]);
			sub->SetActionId(act_ids[i]);
			sub->SetGoal(*goal);
			auto& link = val.sub.Add();
			link.symbolic_link = sub.n;
			root->tmp_sub.Add(hash, sub);
		} else {
			auto& link = val.sub.Add();
			link.symbolic_link = &root->tmp_sub[j]->val;
		}
	}
	return true;
}

bool ActionNode::TerminalTest() {
	if (this->GetEstimate() <= 0)
		return true;
	return !val.GetCount();
}

double ActionNode::GetDistance(VfsValue& n) {
	ActionNode& to = *CastPtr<ActionNode>(&*n.ext);
	double dist = 0;
	
	Vector<bool>& values = ws->atom_values;
	const Vector<bool>& to_values = to.ws->atom_values;
	
	Vector<bool>& using_atom = ws->using_atom;
	const Vector<bool>& to_using_atom = to.ws->using_atom;
	
	int count = UPP::min(values.GetCount(), to_values.GetCount());
	
	for(int j = count; j < to_using_atom.GetCount(); j++)
		if (to_using_atom[j] && to_values[j])
			dist += 1;
	
	for(int j = 0; j < count; j++)
		if (to_using_atom[j] && values[j] != to_values[j])
			dist += 1;
	
	return dist;
}

double ActionNode::GetEstimate() {
	ASSERT(goal);
	return GetDistance(goal->val);
}















OmniActionPlanner::OmniActionPlanner() {
	ws_initial.session = &this->ws_session;
	ws_goal.session = &this->ws_session;
	tmp0.session = &this->ws_session;
	tmp1.session = &this->ws_session;
	
}

bool OmniActionPlanner::SetParams(Value val) {
	this->params = val;
	
	ValueMap in_actions  = val("actions");
	ValueMap in_atoms    = val("atoms");
	if (in_actions.IsEmpty()) {WhenError("OmniActionPlanner requires actions list"); return false;}
	if (in_atoms.IsEmpty())   {WhenError("OmniActionPlanner requires atoms list");   return false;}
	
	actions.Clear();
	for(int i = 0; i < in_actions.GetCount(); i++) {
		String action_name = in_actions.GetKey(i);
		int j = actions.Find(action_name);
		if (j >= 0) {WhenError("duplicate action: " + action_name); return false;}
		actions.Add(action_name);
	}
	
	ws_session.atoms.Clear();
	ws_initial.Clear();
	ws_initial.atom_values.SetCount(in_atoms.GetCount(), false);
	ws_initial.using_atom.SetCount(in_atoms.GetCount(), true);
	for(int i = 0; i < in_atoms.GetCount(); i++) {
		String atom_name = in_atoms.GetKey(i);
		Value v = in_atoms.GetValue(i);
		
		int j = ws_session.atoms.Find(atom_name);
		if (j >= 0) {WhenError("duplicate atom: " + atom_name); return false;}
		auto& atom = ws_session.atoms.Add(atom_name);
		if (v.Is<ValueArray>()) {
			ValueArray atom_v = v;
			if (atom_v.GetCount() != 3) {WhenError("unexpected atom '" + atom_name + "' values: " + v.ToString()); return false;}
			atom.positive = atom_v[0];
			atom.negative = atom_v[1];
			atom.initial  = atom_v[2];
		}
		else {
			atom.initial = v;
		}
		ws_initial.atom_values[i] = atom.initial;
	}
	
	events.SetCount(in_actions.GetCount());
	for(int act_idx = 0; act_idx < in_actions.GetCount(); act_idx++) {
		String action = in_actions.GetKey(act_idx);
		ValueMap in_action = in_actions.GetValue(act_idx);
		ValueArray in_pre  = in_action("pre");
		ValueArray in_post = in_action("post");
		double cost = in_action.Get("cost", 1.0);
		if (in_pre.IsEmpty())  {WhenError("empty pre-condition in action '" + action + "'");  return false;}
		if (in_post.IsEmpty()) {WhenError("empty post-condition in action '" + action + "'"); return false;}
		PlannerEvent& ev = events[act_idx];
		ev.cost = cost;
		for (int m = 0; m < 2; m++) {
			auto& in  = m == 0 ? in_pre     : in_post;
			auto& out = m == 0 ? ev.precond : ev.postcond;
			for(int j = 0; j < in.GetCount(); j++) {
				ValueArray arr = in[j];
				if (arr.GetCount() != 2) {WhenError("expected vector of 2 in condition");  return false;}
				String atom_name = arr[0];
				int atom_value = arr[1];
				int atom_idx = ws_session.atoms.Find(atom_name);
				if (atom_idx < 0) {WhenError("atom '" + atom_name + "' not found");  return false;}
				out.Set(atom_idx, atom_value);
			}
		}
	}
	
	return true;
}

VfsValue& OmniActionPlanner::GetInitial(Val& fs) {
	return fs.GetAdd(".initial",0);
}

bool OmniActionPlanner::Run(Val& fs) {
	this->fs = &fs;
	tmp_sub.Clear();
	{
		initial = &fs.GetAdd(".initial",0);
		if (!Set(*initial, ws_initial, 0.0, -1)) return false;
		hash_t h = ws_initial.GetHashValue();
		tmp_sub.Add(h, initial);
	}
	
	
	{
		goal = &fs.GetAdd(".goal",0);
		ValueMap in_goal = params("goal");
		if (in_goal.IsEmpty()) {WhenError("OmniActionPlanner requires goal atom-list");   return false;}
		ws_goal.FromValue(in_goal, WhenError);
		if (!Set(*goal, ws_goal, 0.0, -1)) return false;
		hash_t h = ws_goal.GetHashValue();
		tmp_sub.Add(h, goal);
	}
	
	return true;
}

bool OmniActionPlanner::Set(VfsValue& v, const BinaryWorldState& ws, double cost, int act_i) {
	Val* p = &v;
	if (!p) return false;
	ValueArray arr;
	arr.Add(ws.ToValue());
	arr.Add(cost);
	arr.Add(act_i);
	p->value = arr;
	return true;
}

bool OmniActionPlanner::Get(const VfsValue& v, BinaryWorldState& ws, double& cost, int& act_i) {
	const Val* p = &v;
	if (!p) return false;
	if (p->value.Is<ValueMap>()) {
		ValueMap m = p->value;
		//DUMPM(m);
		WhenError("unexpected ValueMap");
		return false;
	}
	if (!p->value.Is<ValueArray>()) {
		//LOG(p->value.GetTypeName());
		WhenError("unexpected Value type");
		return false;
	}
	ValueArray arr = p->value;
	if (arr.GetCount() != 3) {WhenError("Could not read [ws,cost,act_i]. Unexpected value"); return false;}
	if (!ws.FromValue(arr[0], WhenError))
		return false;
	cost = arr[1];
	act_i = arr[2];
	return true;
}

bool OmniActionPlanner::GetCost(const VfsValue& v, double& cost) {
	ValueArray arr = v.value;
	if (arr.GetCount() != 3) {WhenError("Could not read [ws,cost,act_i]. Unexpected value"); return false;}
	cost = arr[1];
	return true;
}

bool OmniActionPlanner::GenerateSubValues(Val& v) {
	// this is equivalent of ActionNode::GenerateSubValues
	
	Val* p = v.Resolve();
	if (p->sub.GetCount())
		return true;
	
	ASSERT(goal);
	
	auto& ws = tmp0;
	double c;
	int a;
	if (!Get(*p, ws, c, a)) {
		ASSERT(0);
		return false;
	}
	
	Vector<int> act_ids;
	Vector<double> action_costs;
	GetPossibleStateTransition(ws, possibilities, act_ids, action_costs);
	for(int i = 0; i < possibilities.GetCount(); i++) {
		BinaryWorldState& ws_to = *possibilities[i];
		hash_t hash = ws_to.GetHashValue();
		int j = tmp_sub.Find(hash);
		Val* to;
		if (j == -1) {
			Val& sub = fs->Add();
			if (!Set(sub, ws_to, 0.0, -1)) return false;
			tmp_sub.Add(hash, &sub);
			to = &sub;
		}
		else {
			to = tmp_sub[j];
		}
		VfsValue& link = p->Add();
		link.symbolic_link = to;
		if (!Set(link, ws_to, action_costs[i], act_ids[i])) return false;
		
		/*double dist = Distance(*p, sub);
		if (!dist) {
			LOG("\nFROM (" << HexStrPtr(p) << "):\n" << ws.ToString() << "\nTO (" << HexStrPtr(&sub) << "):\n" << ws_to.ToString() << "\n" << "DISTANCE: " << dist);
		}*/
	}
	return true;
}

bool OmniActionPlanner::GetPossibleStateTransition(const BinaryWorldState& src, Vector<BinaryWorldState*>& dest, Vector<int>& act_ids, Vector<double>& action_costs) {
	dest.SetCount(0);
	for (int i=0; i < events.GetCount(); ++i )
	{
		// Check precondition
		auto& e = events[i];
		const BinaryWorldState& pre  = e.precond;
		const BinaryWorldState& post = e.postcond;
		
		// Check that precondition is not using something outside of src values
		int pre_count = UPP::min(pre.using_atom.GetCount(), src.using_atom.GetCount());
		bool fail = false;
		for(int j = pre_count; j < pre.using_atom.GetCount(); j++)
			if (pre.using_atom[j] && pre.atom_values[j])
				{fail = true; break;}
		if (fail)
			continue;
		
		bool met = true;
		int met_count = 0;
		for(int j = 0; j < pre_count; j++) {
			int a = pre.using_atom[j];
			if (a && src.atom_values[j] != pre.atom_values[j]) {
				met = false;
				break;
			}
			else met_count++;
		}
		if (!met)
			continue;
		ASSERT(met_count > 0);
		
		// Check that changes can be made
		int post_count = UPP::min(post.using_atom.GetCount(), src.using_atom.GetCount());
		bool changes = false;
		if (post_count < post.using_atom.GetCount()) {
			for(int j = post_count; j < post.using_atom.GetCount(); j++)
				if (post.using_atom[j] && post.atom_values[j])
					{changes = true; break;}
		}
		for(int j = 0; j < post_count; j++)
			if (post.using_atom[j] &&
				post.atom_values[j] != src.atom_values[j])
				{changes = true; break;}
		if (!changes)
			continue;
		
		act_ids.Add(i);
		action_costs.Add(e.cost);
		BinaryWorldState& tmp = search_cache.Add();
		DoAction( i, src, tmp );
		if (tmp == src) {LOG("ERROR: NO CHANGES IN:\n" << tmp.ToString());}
		ASSERT(!(tmp == src));
		dest.Add(&tmp);
	}
	return true;
}

void OmniActionPlanner::DoAction( int action_id, const BinaryWorldState& src, BinaryWorldState& dest) {
	const BinaryWorldState& post = events[action_id].postcond;
	
	dest = src;
	
	for(int i = 0; i < post.using_atom.GetCount(); i++) {
		if (post.using_atom[i]) {
			dest.Set(i, post.atom_values[i]);
		}
	}
}

bool OmniActionPlanner::TerminalTest(Val& v) {
	if (1) {
		LOG("OmniActionPlanner::TerminalTest: " << HexStrPtr(&v) << ": " << v.value.ToString());
	}
	if (Estimate(v) <= 0)
		return true;
	
	Val* p = v.Resolve();
	return !p->GetCount();
}

double OmniActionPlanner::Utility(Val& val) {
	TODO
	return 0;
}

double OmniActionPlanner::Estimate(Val& n) {
	ASSERT(goal);
	if (!goal) return DBL_MAX;
	return Distance(n, *goal);
}

double OmniActionPlanner::Distance(Val& v, Val& dest) {
	
	// Find the real cost (== distance)
	double cost_multiplier = 1.0;
	if (dest.owner && dest.owner == &v) {
		for(int i = 0; i < v.sub.GetCount(); i++) {
			Val* c = &v.sub[i];
			if (c == &dest) {
				double cost = 1.0;
				if (!GetCost(*c, cost))
					break;
				ASSERT(cost != 0.0);
				cost_multiplier = cost;
				break;
			}
		}
	}
	
	// this is equivalent of ActionNode::GetDistance
	Val* a = &v; //v.Resolve();
	if (!a) return DBL_MAX;
	Val* b = &dest; //dest.Resolve();
	if (!b) return DBL_MAX;
	
	double dist = 0;
	double c0, c1;
	int a0, a1;
	
	if (!Get(*a, tmp0, c0, a0))
		return DBL_MAX;
	if (!Get(*b, tmp1, c1, a1))
		return DBL_MAX;
	
	
	int count = UPP::min(tmp0.atom_values.GetCount(), tmp1.atom_values.GetCount());
	
	// When the size is different, the tail is considered be all 'false'
	// So if there's any 'true' values, they are different of 'false'
	for(int j = count; j < tmp1.atom_values.GetCount(); j++)
		if (tmp1.using_atom[j] && tmp1.atom_values[j])
			dist += 1;
	
	for(int j = 0; j < count; j++)
		if (tmp1.using_atom[j] && tmp0.atom_values[j] != tmp1.atom_values[j])
			dist += 1;
	
	double total_dist = cost_multiplier * dist;
	
	LOG("\nFROM (" << HexStrPtr(a) << "):\n" << tmp0.ToString() << "\nTO (" << HexStrPtr(b) << "):\n" << tmp1.ToString() << "\n" << "DISTANCE: " << total_dist);
	
	return total_dist;
}


END_UPP_NAMESPACE
