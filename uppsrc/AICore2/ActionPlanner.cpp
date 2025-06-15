#include "AICore.h"


NAMESPACE_UPP









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
	Val* p = n.Resolve();
	ActionNode& to = *CastPtr<ActionNode>(&*p->ext);
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
	
}

bool OmniActionPlanner::SetParams(Value val) {
	this->params = val;
	cost_multiplier = params.Get("cost_multiplier", 1.5);
	use_params = params.Get("use_params", false);
	use_resolver = params.Get("use_resolver", false);
	
	if (use_resolver) TODO;
	
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
		Key atom_key = RealizeKey(atom_name);
		
		int j = ws_session.atoms.Find(atom_key);
		if (j >= 0) {WhenError("duplicate atom: " + atom_name); return false;}
		auto& atom = ws_session.atoms.Add(atom_key);
		if (v.Is<bool>() || v.Is<int>()) {
			atom.initial = v;
		}
		else {
			WhenError("unexpected value type '" + v.GetTypeName() + "'"); return false;
		}
		ws_initial.atom_values[i] = atom.initial;
	}
	
	events.Clear();
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
				Key atom_key = RealizeKey(atom_name);
				int atom_value = arr[1];
				int atom_idx = ws_session.atoms.Find(atom_key);
				if (atom_idx < 0) {WhenError("atom '" + atom_name + "' not found");  return false;}
				out.Set(atom_idx, atom_value);
			}
		}
	}
	
	return true;
}

WorldStateKey OmniActionPlanner::RealizeKey(const String& str) {
	WorldStateKey key;
	auto& mask = GetMask();
	if (!mask.ParseKey(use_params, str, key)) {
		WhenError("parsing key '" + str + "' failed");
	}
	return key;
}

VfsValue& OmniActionPlanner::GetInitial(Val& fs) {
	return fs.GetAdd(".initial",0);
}

bool OmniActionPlanner::Run(Val& fs) {
	this->fs = &fs;
	
	ws_initial.mask = &GetMask();
	ws_goal.mask = &GetMask();
	tmp0.mask = &GetMask();
	tmp1.mask = &GetMask();
	
	tmp_sub.Clear();
	{
		initial = &fs.GetAdd(".initial",0);
		if (!Set(*initial, ws_initial)) return false;
		hash_t h = ws_initial.GetHashValue();
		tmp_sub.Add(h, initial);
	}
	
	
	{
		goal = &fs.GetAdd(".goal",0);
		ValueMap in_goal = params("goal");
		if (in_goal.IsEmpty()) {WhenError("OmniActionPlanner requires goal atom-list");   return false;}
		ws_goal.FromValue(use_params, in_goal, WhenError);
		if (!Set(*goal, ws_goal)) return false;
		hash_t h = ws_goal.GetHashValue();
		tmp_sub.Add(h, goal);
	}
	
	return true;
}

bool OmniActionPlanner::Set(VfsValue& v, const BinaryWorldState& ws) {
	ASSERT(!v.symbolic_link);
	if (v.symbolic_link)
		return false; // don't write all data to symbolic links
	v.value = ws.ToValue();
	return true;
}

bool OmniActionPlanner::Get(const VfsValue& v, BinaryWorldState& ws) const {
	if (!v.value.Is<ValueMap>()) {
		WhenError("unexpected Value type");
		return false;
	}
	return ws.FromValue(use_params, v.value, WhenError);
}

void OmniActionPlanner::SetAction(VfsValue& v, int action) {
	v.value = action;
}

bool OmniActionPlanner::GetAction(VfsValue& v, int& action) {
	if (v.value.Is<int>()) {
		action = v.value;
		return true;
	}
	return false;
}

bool OmniActionPlanner::GetCost(const VfsValue& v, double& cost) {
	ASSERT(v.value.Is<int>());
	if (!v.value.Is<int>())
		return false;
	int act = v.value;
	if (act < 0 || act >= events.GetCount())
		return false;
	cost = events[act].cost;
	return true;
}

bool OmniActionPlanner::GenerateSubValues(Val& v) {
	// this is equivalent of ActionNode::GenerateSubValues
	
	Val* p = v.Resolve();
	if (p->sub.GetCount())
		return true;
	
	ASSERT(goal);
	
	auto& ws = tmp0;
	if (!Get(*p, ws)) {
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
			if (!Set(sub, ws_to)) return false;
			tmp_sub.Add(hash, &sub);
			to = &sub;
		}
		else {
			to = tmp_sub[j];
		}
		VfsValue& link = p->Add();
		link.symbolic_link = to;
		SetAction(link, act_ids[i]);
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

void OmniActionPlanner::DoAction( int action_id, const BinaryWorldState& src, BinaryWorldState& dest) const {
	const BinaryWorldState& post = events[action_id].postcond;
	
	dest = src;
	
	for(int i = 0; i < post.using_atom.GetCount(); i++) {
		if (post.using_atom[i]) {
			dest.Set(i, post.atom_values[i]);
		}
	}
}

bool OmniActionPlanner::TerminalTest(Val& v) {
	if (0) {
		LOG("OmniActionPlanner::TerminalTest: " << HexStrPtr(&v) << ": " << v.value.ToString());
	}
	
	if (!Get(v, tmp0)) {
		ASSERT(0);
		return false;
	}
	if (tmp0.IsPartialMatch(ws_goal)) {
		if (0) {
			LOG("GOAL:\n" << ws_goal.ToString());
			LOG("TERMINAL:\n" << tmp0.ToString());
		}
		return true;
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

double OmniActionPlanner::Distance(Val& a, Val& b) {
	Val* a_link = a.Resolve();
	Val* b_link = b.Resolve();
	if (!Get(*a_link, tmp0))
		return DBL_MAX;
	if (!Get(*b_link, tmp1))
		return DBL_MAX;
	
	double total_dist = 0.0;
	bool add_hamming_dist = false;
	if (b.symbolic_link) {
		int act = b.value;
		if (act >= 0 && act < events.GetCount()) {
			total_dist += events[act].cost;
		}
		else {ASSERT(0);}
	}
	else {
		ASSERT(!a.symbolic_link);
		// guesstimate distance using Hamming distance
		
		
		int count = UPP::min(tmp0.atom_values.GetCount(), tmp1.atom_values.GetCount());
		
		// When the size is different, the tail is considered be all 'false'
		// So if there's any 'true' values, they are different of 'false'
		int dist = 0;
		for(int j = count; j < tmp1.atom_values.GetCount(); j++)
			if (tmp1.using_atom[j] && tmp1.atom_values[j])
				dist += 1;
		
		for(int j = 0; j < count; j++)
			if (tmp1.using_atom[j] && tmp0.atom_values[j] != tmp1.atom_values[j])
				dist += 1;
		
		total_dist += max(0.0, cost_multiplier * dist);
	}
	
	if (0) {
		BinaryWorldState diff;
		diff.SetDifference(tmp0, tmp1);
		if (diff.ToInlineString() == "at_high_place") {
			LOG("");
		}
		LOG("\nFROM (" << HexStrPtr(&a) << "):\n" << tmp0.ToString() << "\nTO (" << HexStrPtr(&b) << "):\n" << tmp1.ToString() << "\nDIFF: \"" << diff.ToInlineString() << "\"\nDISTANCE: " << total_dist);
	}
	
	return total_dist;
}

String OmniActionPlanner::GetTreeString() const {
	BinaryWorldState ws;
	return GetTreeString(*fs, ws, 0);
}

String OmniActionPlanner::GetTreeString(VfsValue& v, BinaryWorldState& parent, int indent) const {
	BinaryWorldState ws, intersection;
	ws.mask = ws_initial.mask;
	String s;
	s.Cat('\t', indent);
	if (v.id.GetCount())
		s << "\"" << v.id << "\" ";
	if (v.value.Is<ValueMap>()) {
		Get(v, ws);
		s << ws.ToShortInlineString() << " ";
	}
	else if (v.value.Is<int>()) {
		int act = v.value;
		DoAction(act, parent, ws);
		s << ws.ToShortInlineString() << " ";
		if (act >= 0 && act < actions.GetCount()) {
			s << "\"" << actions[act] << "\"";
		}
		double cost = events[act].cost;
		if (cost)
			s << " (cost: " << cost << ")";
		intersection.SetDifference(parent, ws);
		if (!intersection.IsEmpty())
			s << " \"" << intersection.ToInlineString() << "\"";
	}
	else {
		s << v.value.ToString();
	}
	s << " (" << HexStrPtr(&v) << ")";
	if (v.symbolic_link)
		s << " -> (" << HexStrPtr(&*v.symbolic_link) << ")";
	
	s << "\n";
	
	for(int i = 0; i < v.sub.GetCount(); i++) {
		Val& sub = v.sub[i];
		s << GetTreeString(sub, ws, indent+1);
	}
	
	return s;
}

String OmniActionPlanner::GetResultString(const Vector<Val*>& result) const {
	String s;
	BinaryWorldState ws, prev;
	prev.mask = ws.mask = &GetMask();
	int i = 0;
	for (Val* v : result) {
		s << i << ":";
		Val* r = v->Resolve();
		
		int act = -1;
		if (v->value.Is<int>())
			act = v->value;
		double cost = 0;
		if (act >= 0 && act < actions.GetCount()) {
			s << " " << actions[act] << ":";
			cost = events[act].cost;
		}
		else if (act >= 0) {
			s << " (error act " << act << "):";
		}
		
		if (r->value.Is<ValueMap>()) {
			if (Get(*r, ws)) {
				BinaryWorldState diff;
				diff.SetDifference(prev, ws);
				s << " \"" << diff.ToInlineString() << "\"";
				Swap(prev, ws);
			}
			else {
				s << "(error: no ws)";
				prev.Clear();
			}
		}
		
		if (cost)
			s << " (cost: " << cost << ")";
		
		s << "\n";
		i++;
	}
	return s;
}

BinaryWorldStateMask& OmniActionPlanner::GetMask() const {
	if (!ws_mask) {
		auto& ses = const_cast<OmniActionPlanner*>(this)->ws_session;
		ws_mask = &ses.masks.GetAdd(0);
		ws_mask->session = &ses;
	}
	return *ws_mask;
}

END_UPP_NAMESPACE
