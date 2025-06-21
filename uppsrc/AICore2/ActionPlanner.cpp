#include "AICore.h"


NAMESPACE_UPP

#undef DLOG
#undef DDUMP
#define DLOG(x)  LOG(x)
#define DDUMP(x) DUMP(x)







PlannerEvent::PlannerEvent() : cost(1.0) {
	
}

String PlannerEvent::GetName() const {
	ASSERT(precond.mask && precond.mask->session);
	ASSERT(precond.mask == postcond.mask);
	if (!precond.mask || !precond.mask->session) return "<error>";
	auto& mask = *precond.mask;
	auto& ses = *precond.mask->session;
	String out;
	out << ses.GetKeyString(key.name);
	int len = 0;
	for(int i = 0; i < this->key.max_len; i++) {
		if (key.params[i].cls < 0)
			break;
		ASSERT(key.params[i].name >= 0);
		if (key.params[i].name < 0)
			break;
		if (i)
			out << ", ";
		else
			out << "(";
		if (key.params[i].shared)
			out.Cat('&');
		String name = ses.GetKeyString(key.params[i].name);
		out.Cat(name);
		if (key.params[i].cls > 0) {
			String cls = ses.GetKeyString(key.params[i].cls);
			out.Cat(": ");
			out.Cat(cls);
		}
		if (key.params[i].val > 0) {
			String val = ses.GetKeyString(key.params[i].val);
			out.Cat(" = ");
			out.Cat(val);
		}
		len++;
	}
	if (len > 0)
		out.Cat(')');
	return out;
}






ActionPlanner::ActionPlanner() {
	
}


void ActionPlanner::Clear() {
	atom_count = 0;
	actions.Clear();
	search_cache.Clear();
}

void ActionPlanner::AddSize(int action_count, int atom_count) {
	ASSERT(action_count >= 0 && atom_count >= 0);
	this->atom_count += atom_count;
	int new_action_count = actions.GetCount() + action_count;
	actions.SetCount(new_action_count);
	if (wrapper)
		wrapper->OnResize();
}

void ActionPlanner::SetSize(int action_count, int atom_count) {
	this->atom_count = atom_count;
	actions.SetCount(action_count);
	if (wrapper)
		wrapper->OnResize();
}

void ActionPlanner::DoAction( int action_id, const BinaryWorldState& src, BinaryWorldState& dest) {
	const BinaryWorldState& post = actions[action_id].postcond;
	dest = src;
	
	int i = 0;
	for(auto& a : post.atoms) {
		if (a.in_use) {
			dest.SetMasked(i, a.value);
		}
		i++;
	}
}


void ActionPlanner::GetPossibleStateTransition(const BinaryWorldState& src, Array<BinaryWorldState*>& dest, Vector<int>& act_ids, Vector<double>& action_costs)
{
	// todo: callback for new actions
	
	for ( int i=0; i < actions.GetCount(); ++i )
	{
		// todo: callback for param resolver
		
		// Check precondition
		auto& e = actions[i];
		const BinaryWorldState& pre = e.precond;
		
		// Check that precondition is not using something outside of src values
		int count = UPP::min(pre.atoms.GetCount(), src.atoms.GetCount());
		bool fail = false;
		for(int j = count; j < pre.atoms.GetCount(); j++)
			if (pre.atoms[j].in_use && pre.atoms[j].value)
				{fail = true; break;}
		if (fail)
			continue;
		
		bool met = true;
		for(int j = 0; j < count; j++) {
			int a = pre.atoms[j].in_use;
			if (a && src.atoms[j].value != pre.atoms[j].value) {
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

bool ActionPlanner::SetPreCondition(int act_idx, int atm_idx, bool value)
{
	if ( act_idx == -1 || atm_idx == -1 ) return false;
	auto& ws = actions[act_idx].precond;
	if (!ws.mask || !ws.mask->session) return false;
	auto& ses = *ws.mask->session;
	if (atm_idx < 0 || atm_idx >= ses.atoms.GetCount()) return false;
	const WorldStateKey& key = ws.mask->session->atoms[atm_idx].key;
	ws.SetKey(key, value);
	return true;
}


bool ActionPlanner::SetPostCondition(int act_idx, int atm_idx, bool value)
{
	if ( act_idx == -1 || atm_idx == -1 ) return false;
	auto& ws = actions[act_idx].postcond;
	if (!ws.mask || !ws.mask->session) return false;
	auto& ses = *ws.mask->session;
	if (atm_idx < 0 || atm_idx >= ses.atoms.GetCount()) return false;
	const WorldStateKey& key = ws.mask->session->atoms[atm_idx].key;
	ws.SetKey(key, value);
	return true;
}


bool ActionPlanner::SetCost(int act_idx, int cost )
{
	if ( act_idx == -1 ) return false;
	actions[ act_idx ].cost = cost;
	return true;
}








ActionPlannerWrapper::ActionPlannerWrapper(ActionPlanner& planner) : ap(planner) {
	ap.wrapper = this;
	OnResize();
}

void ActionPlannerWrapper::OnResize() {
	acts.SetCount(ap.GetEventCount());
	TODO //atoms.SetCount(ap.GetAtomCount());
}

String ActionPlannerWrapper::GetWorldstateDescription( const BinaryWorldState& ws )
{
	String str;
	TODO /*
	for(int i = 0; i < atoms.GetCount(); i++) {
		if (ws.using_atom.GetCount() <= i) break;
		if (ws.using_atom[i]) {
			bool set = ws.atom_values[i];
			if (set)
				str += ToUpper(atoms[i]) + ",";
			else
				str += atoms[i] + ",";
		}
	}*/
	return str;
}


String ActionPlannerWrapper::GetAtomName(int i) {
	TODO
	//return atoms[i];
	return "";
}

void ActionPlannerWrapper::SetAtom(int atom_i, String s) {
	TODO
	//atoms[atom_i] = s;
}

String ActionPlannerWrapper::GetDescription()
{
	String str;
	for(int j = 0; j < ap.actions.GetCount(); j++) {
		auto& e = ap.actions[j];
		str += acts[j] + ":\n";
		
		BinaryWorldState& pre  = e.precond;
		BinaryWorldState& post = e.postcond;
		
		TODO /*
		int count = UPP::min(atoms.GetCount(), pre.atom_values.GetCount());
		for(int i = 0; i < count; i++) {
			bool v = pre.atom_values[i];
			str += " " + atoms[i] + "==" + IntStr(v) + "\n";
		}
		
		count = UPP::min(atoms.GetCount(), post.atom_values.GetCount());
		for(int i = 0; i < count; i++) {
			bool v = post.atom_values[i];
			str += " " + atoms[i] + "==" + IntStr(v) + "\n";
		}*/
	}
	return str;
}


int ActionPlannerWrapper::GetAtomIndex(String atom_name) {
	TODO /*
	int i = VectorFind(atoms, atom_name);
	if (i != -1)
		return i;
	atoms.Add(atom_name);
	return atoms.GetCount()-1;
	*/
	return -1;
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
	TODO //ap.actions[ev_idx].precond.Set(atm_idx, value);
	return true;
}


bool ActionPlannerWrapper::SetPostCondition(String event_name, String atom_name, bool value)
{
	int ev_idx = GetEventIndex( event_name );
	int atm_idx = GetAtomIndex( atom_name );
	if ( ev_idx == -1 || atm_idx == -1 ) return false;
	TODO //ap.actions[ev_idx].postcond.Set(atm_idx, value);
	return true;
}


bool ActionPlannerWrapper::SetCost(String event_name, int cost )
{
	int ev_idx = GetEventIndex( event_name );
	if ( ev_idx == -1 ) return false;
	ap.actions[ ev_idx ].cost = cost;
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
	
	auto& src_atoms = ws->atoms;
	const auto& to_atoms = to.ws->atoms;
	
	int count = UPP::min(src_atoms.GetCount(), to_atoms.GetCount());
	
	for(int j = count; j < to_atoms.GetCount(); j++)
		if (to_atoms[j].in_use && to_atoms[j].value)
			dist += 1;
	
	for(int j = 0; j < count; j++)
		if (to_atoms[j].in_use && src_atoms[j].value != to_atoms[j].value)
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
	
	ValueMap in_initial  = val("initial");
	run_initial = in_initial.GetCount();
	
	ValueMap in_actions  = val("actions");
	ValueMap in_atoms    = val("atoms");
	if (in_actions.IsEmpty()) {WhenError("OmniActionPlanner requires actions list"); return false;}
	if (in_atoms.IsEmpty())   {WhenError("OmniActionPlanner requires atoms list");   return false;}
	
	actions.Clear();
	for(int i = 0; i < in_actions.GetCount(); i++) {
		String action_name = in_actions.GetKey(i);
		Key action_key;
		if (!this->ParseDecl(action_name, action_key)) return false;
		int j = FindAction(action_key);
		if (j >= 0) {WhenError("duplicate action: " + action_name); return false;}
		if (HasDuplicateParams(action_key)) {WhenError("duplicate params in action: " + action_name); return false;}
		auto& action = actions.Add();
		action.key = action_key;
		action.postcond.mask = &GetMask();
		action.precond.mask = &GetMask();
	}
	
	ws_session.atoms.Clear();
	ws_initial.Clear();
	ws_initial.atoms.SetCount(in_atoms.GetCount());
	for(int i = 0; i < in_atoms.GetCount(); i++) {
		String atom_name = in_atoms.GetKey(i);
		Value v = in_atoms.GetValue(i);
		Key atom_key;
		if (!ParseCall(atom_name, atom_key)) return false;
		
		int j = ws_session.atoms.Find(atom_key);
		if (j >= 0) {WhenError("duplicate atom: " + atom_name); return false;}
		auto& atom = ws_session.GetAddAtom(atom_key);
		if (v.Is<bool>() || v.Is<int>()) {
			atom.initial = v;
		}
		else {
			WhenError("unexpected value type '" + v.GetTypeName() + "'"); return false;
		}
		ws_initial.atoms[i].value = atom.initial;
	}
	
	for(int act_idx = 0; act_idx < actions.GetCount(); act_idx++) {
		PlannerEvent& a = actions[act_idx];
		DLOG("ACTION " << a.precond.mask->ToString(a.key));
		String action = in_actions.GetKey(act_idx);
		ValueMap in_action = in_actions.GetValue(act_idx);
		ValueArray in_pre  = in_action("pre");
		ValueArray in_post = in_action("post");
		double cost = in_action.Get("cost", 1.0);
		if (in_pre.IsEmpty())  {WhenError("empty pre-condition in action '" + action + "'");  return false;}
		if (in_post.IsEmpty()) {WhenError("empty post-condition in action '" + action + "'"); return false;}
		a.cost = cost;
		int shared_count = 0;
		DLOG("");
		for (int m = 0; m < 2; m++) {
			auto& in  = m == 0 ? in_pre    : in_post;
			auto& out = m == 0 ? a.precond : a.postcond;
			out.mask = &GetMask();
			DLOG((String)(m == 0 ? "PRECOND:" : "POSTCOND:"));
			for(int j = 0; j < in.GetCount(); j++) {
				ValueArray arr = in[j];
				if (arr.GetCount() != 2) {WhenError("expected vector of 2 in condition");  return false;}
				String atom_name = arr[0];
				Key atom_key;
				if (!ParseCondParam(a.key, atom_name, atom_key)) return false;
				if (HasDuplicateParams(atom_key)) {WhenError("duplicate params in atom: " + a.precond.mask->ToString(atom_key)); return false;}
				DLOG("\tPARAM " << a.precond.mask->ToString(atom_key));
				int atom_value = arr[1];
				int atom_idx = ws_session.atoms.FindAdd(atom_key);
				/*if (atom_idx < 0) {
					WhenError("atom '" + atom_name + "' not found");  return false;}*/
				out.SetKey(atom_key, atom_value);
				if (use_resolver)
					for(int k = 0; k < WSKEY_MAX_PARAMS; k++)
						if (atom_key.params[k].shared)
							shared_count++;
			}
			DLOG("WS:\n" << out.ToString() << "\n");
		}
		if (a.key.GetLength() > 0 && !shared_count) {
			WhenError("the action '" + a.GetName() + "' has params, but no pre nor post condition uses it");
			return false;
		}
	}
	
	return true;
}

bool OmniActionPlanner::ParseDecl(const String& str, WorldStateKey& key) {
	if (!use_params && !ws_session.ParseRaw (str, key)) {WhenError("parsing key '" + str + "' failed"); return false;}
	if ( use_params && !ws_session.ParseDecl(str, key)) {WhenError("parsing key '" + str + "' failed"); return false;}
	return true;
}

bool OmniActionPlanner::ParseCall(const String& str, WorldStateKey& key) {
	if (!use_params && !ws_session.ParseRaw (str, key)) {WhenError("parsing key '" + str + "' failed"); return false;}
	if ( use_params && !ws_session.ParseCall(str, key)) {WhenError("parsing key '" + str + "' failed"); return false;}
	return true;
}

bool OmniActionPlanner::ParseCondParam(const Key& action, const String& str, WorldStateKey& key) {
	if (!use_params && !ws_session.ParseRaw (str, key)) {WhenError("parsing key '" + str + "' failed"); return false;}
	if ( use_params && !ws_session.ParseCondParam(action, str, key)) {WhenError("parsing key '" + str + "' failed"); return false;}
	return true;
}

bool OmniActionPlanner::HasDuplicateParams(const Key& key) {
	for(int i = 0; i < Key::max_len; i++) {
		if (key.params[i].cls < 0) break;
		int p0 = key.params[i].name;
		for(int j = i+1; j < Key::max_len; j++) {
			if (key.params[j].cls < 0) break;
			int p1 = key.params[j].name;
			if (p0 == p1)
				return true;
		}
	}
	return false;
}

int OmniActionPlanner::FindAction(const WorldStateKey& key) const {
	int i = 0;
	for (auto& a : actions) {
		if (a.key == key)
			return i;
		i++;
	}
	return -1;
}

VfsValue& OmniActionPlanner::GetInitial(Val& fs) {
	return fs.GetAdd(".initial",0);
}

bool OmniActionPlanner::Run(Val& fs) {
	this->fs = &fs;
	
	auto& mask = GetMask();
	ws_initial.mask = &mask;
	ws_goal.mask = &mask;
	tmp0.mask = &mask;
	tmp1.mask = &mask;
	
	tmp_sub.Clear();
	{
		if (run_initial) {
			ValueMap in_initial = params("initial");
			ASSERT(in_initial.GetCount());
			for(int i = 0; i < in_initial.GetCount(); i++) {
				String call = in_initial.GetKey(i);
				bool initial_value = in_initial.GetValue(i);
				Key call_key;
				if (!ParseCall(call, call_key)) return false;
				int decl_atom_idx = -1;
				if (!ResolveCall(call_key, decl_atom_idx)) {WhenError("could not resolve '" + call + "' atom"); return false;}
				int call_atom_idx = ws_session.FindAddAtom(call_key);
				DLOG("INITIAL " << i << ": " << ws_session.GetKeyString(call_key));
				auto& atom = ws_session.atoms[call_atom_idx];
				atom.initial = initial_value;
				ws_initial.SetAtomIndex(call_atom_idx, initial_value);
			}
		}
		
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

bool OmniActionPlanner::ResolveCall(const Key& call_key, int& atom_idx) {
	ASSERT(!call_key.IsEmpty());
	
	int key_len = call_key.GetLength();
	int i = 0;
	for (auto& atom : ws_session.atoms) {
		if (atom.key_len  == key_len &&
			atom.key.name == call_key.name) {
			atom_idx = i;
			return true;
		}
		i++;
	}
	
	WhenError("could not resolve call: " + ws_session.GetKeyString(call_key));
	return false;
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
	if (act < 0 || act >= actions.GetCount())
		return false;
	cost = actions[act].cost;
	return true;
}

String chk_str;
void* chk_ptr;

bool OmniActionPlanner::GenerateSubValues(Val& v) {
	DLOG(HexStrPtr(&v));
	
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
	DLOG(HexStrPtr(p) << ": " << ws.ToShortInlineString());
	
	if (&v == chk_ptr) {
		ASSERT(ws.ToShortInlineString() == chk_str);
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
	DLOG("SRC:\n" << src.ToShortInlineString());
	
	// TODO: AI exchange point: add actions
	//		 --> collect this to "failure points" and use if 'no route is found'
	
	for (int i=0; i < actions.GetCount(); ++i )
	{
		// Check precondition
		auto& e = actions[i];
		
		ActionParamResolver rs(ws_session);
		if (!rs.Resolve(e, src)) {
			if (rs.IsError()) {
				LOG("ActionParamResolver: error: " << rs.GetError());
				TODO
				return false;
			}
			else {
				// TODO: AI exchange point: create missing atoms
				//		 --> collect this to "failure points" and use if 'no route is found'
				continue;
			}
		}
		
		TODO
		
		// Check dynamic pre-conditions
		/*bool pre_req_resolve = false;
		for (auto& it : pre.atoms)
			pre_req_resolve = pre_req_resolve || it.req_resolve;
		if (pre_req_resolve) {
			TODO
			
			// resolve params
			
		}*/
		#if 0
		
		ASSERT(met_count > 0 || !pre_count);
		
		// Check that changes can be made
		int post_count = UPP::min(post.atoms.GetCount(), src.atoms.GetCount());
		bool changes = false;
		if (post_count < post.atoms.GetCount()) {
			for(int j = post_count; j < post.atoms.GetCount(); j++)
				if (post.atoms[j].in_use && post.atoms[j].value)
					{changes = true; break;}
		}
		for(int j = 0; j < post_count; j++)
			if (post.atoms[j].in_use &&
				post.atoms[j].value != src.atoms[j].value)
				{changes = true; break;}
				
		/*bool post_req_resolve = false;
		for (auto& it : post.atoms)
			post_req_resolve = post_req_resolve || it.req_resolve;
		if (!changes && post_req_resolve) {
			TODO
		}*/
		if (!changes)
			continue;
		
		/*if (post_req_resolve) {
			// resolve params from post
			
			TODO
		}*/
		
		act_ids.Add(i);
		action_costs.Add(e.cost);
		BinaryWorldState& tmp = search_cache.Add();
		DoAction( i, src, tmp );
		if (tmp == src) {LOG("ERROR: NO CHANGES IN:\n" << tmp.ToString());}
		ASSERT(!(tmp == src));
		dest.Add(&tmp);
		DLOG("ADDED");
		#endif
	}
	return true;
}

void OmniActionPlanner::DoAction( int action_id, const BinaryWorldState& src, BinaryWorldState& dest) const {
	const BinaryWorldState& post = actions[action_id].postcond;
	
	dest = src;
	ASSERT(post.mask == dest.mask);
	if (post.mask != dest.mask) return;
	
	DDUMP(post);
	int i = 0;
	auto k = post.mask->keys.Begin();
	for(auto& a : post.atoms) {
		if (a.in_use) {
			dest.SetMasked(i, a.value);
		}
		k++;
		i++;
	}
	
	DDUMP(dest);
}

bool OmniActionPlanner::TerminalTest(Val& v) {
	DLOG("OmniActionPlanner::TerminalTest: " << HexStrPtr(&v) << ": " << v.value.ToString());
	
	if (!Get(v, tmp0)) {
		ASSERT(0);
		return false;
	}
	if (tmp0.IsPartialMatch(ws_goal)) {
		DLOG("GOAL:\n" << ws_goal.ToString());
		DLOG("TERMINAL:\n" << tmp0.ToString());
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
		if (act >= 0 && act < actions.GetCount()) {
			total_dist += actions[act].cost;
		}
		else {ASSERT(0);}
	}
	else {
		ASSERT(!a.symbolic_link);
		// guesstimate distance using Hamming distance
		
		int count = UPP::min(tmp0.atoms.GetCount(), tmp1.atoms.GetCount());
		
		// When the size is different, the tail is considered be all 'false'
		// So if there's any 'true' values, they are different of 'false'
		int dist = 0;
		for(int j = count; j < tmp1.atoms.GetCount(); j++)
			if (tmp1.atoms[j].in_use && tmp1.atoms[j].value)
				dist += 1;
		
		for(int j = 0; j < count; j++)
			if (tmp1.atoms[j].in_use && tmp0.atoms[j].value != tmp1.atoms[j].value)
				dist += 1;
		
		total_dist += max(0.0, cost_multiplier * dist);
	}
	
	DLOG("\nFROM (" << HexStrPtr(&a) << "):\n" << tmp0.ToString() << "\nTO (" << HexStrPtr(&b) << "):\n" << tmp1.ToString() << "\nDIFF: \"" << BinaryWorldState::GetDifference(tmp0, tmp1).ToInlineString() << "\"\nDISTANCE: " << total_dist);
	
	return total_dist;
}

String OmniActionPlanner::GetTreeString() const {
	BinaryWorldState ws;
	if (!fs)
		return String();
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
			s << "\"" << actions[act].GetName() << "\"";
		}
		double cost = actions[act].cost;
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
			s << " " << actions[act].GetName() << ":";
			cost = actions[act].cost;
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














ActionParamResolver::ActionParamResolver(BinaryWorldStateSession& ses) : ses(&ses) {
	
}

bool ActionParamResolver::Resolve(const PlannerEvent& e, const BinaryWorldState& src) {
	this->ev = &e;
	this->src = &src;
	err.Clear();
	shared.Clear();
	
	pre_count  = UPP::min(ev->precond.atoms.GetCount(),  src.atoms.GetCount());
	post_count = UPP::min(ev->postcond.atoms.GetCount(), src.atoms.GetCount());
	
	// todo: 3-state behaviour is not completely solved yet...
	//       now: "used & false" in tail passes, but should it be "!used"???
	if (IsPreTailMismatch()) {
		// not an error, but fails
		return false;
	}
	
	if (!MakeKeys()) {
		err = "making keys failed";
		return false;
	}
	
	#if 0
	LOG(pre.mask->ToString(pre.mask->keys[j].key));
	
	const BinaryWorldState& pre  = e.precond;
	const BinaryWorldState& post = e.postcond;
	
	const auto& mkey0 = pre.mask->keys[j];
	bool found = false;
	for(int k = 0; k < src.atoms.GetCount(); k++) {
		const auto& mkey1 = src.mask->keys[k];
		if (mkey0.atom_idx == mkey1.atom_idx) {
			found = true;
		}
	}
	if (!found) {
		LOG("error: shared param not found in: " << pre.mask->ToString(pre.mask->keys[j].key));
		return false;
	}
	#endif
	
	return false;
}

bool ActionParamResolver::IsPreTailMismatch() {
	// Check that precondition is not using something outside of src values
	const BinaryWorldState& pre = ev->precond;
	bool fail = false;
	for(int j = pre_count; j < pre.atoms.GetCount(); j++)
		if (pre.atoms[j].in_use && pre.atoms[j].value)
			return true;
	return false;
}

bool ActionParamResolver::FindSharedVariables(int mask_idx, const Key& key, Source src) {
	int shared_count = 0;
	for(int i = 0; i < Key::max_len; i++) {
		if (key.params[i].cls < 0)
			break;
		if (key.params[i].shared) {
			int name = key.params[i].name;
			ASSERT(name >= 0);
			shared_count++;
			SharedParam& sp = shared.GetAdd(name);
			
			// Collect all indices of keys that use shared variables (per shared key)
			auto& atom = sp.atoms.Add();
			atom.mask_idx = mask_idx;
			atom.param_idx = i;
			atom.src = src;
			if (key.params[i].val >= 0) {
				if (sp.def_val >= 0) {
					err = "multiple default values set";
					return false;
				}
				sp.def_val = key.params[i].val;
			}
		}
	}
	return true;
}

bool ActionParamResolver::FindSharedVariables(int count, const BinaryWorldState& ws, Source src) {
	for(int j = 0; j < count; j++) {
		auto& a = ws.atoms[j];
		if (!a.in_use)
			continue;
		const auto& akey = ws.mask->keys[j].key;
		if (!FindSharedVariables(j, akey, src))
			return false;
	}
	return true;
}

bool ActionParamResolver::TestBasic() {
	const BinaryWorldState& pre = ev->precond;
	const auto& src_atoms = src->atoms;
	bool met = true;
	if (shared.IsEmpty()) {
		for(int j = 0; j < pre_count; j++) {
			auto& a = pre.atoms[j];
			if (!a.in_use)
				continue;
			if (src_atoms[j].value != a.value) {
				met = false;
				break;
			}
		}
		if (!met)
			return false; // Fixed-value atoms didn't match (and no errors)
	}
	return true;
}

bool ActionParamResolver::SolveShared() {
	BinaryWorldStateMask* mask = ev->precond.mask;
	auto mask_begin = mask->keys.Begin();
	for (auto shr : ~shared) {
		for (auto& shr_atom : shr.value.atoms) {
			auto mask0 = mask_begin + shr_atom.mask_idx;
			auto mask1 = mask_begin;
			int len0 = mask0->key.GetLength();
			ASSERT(shr_atom.mask_idx >= 0 && shr_atom.mask_idx < mask->keys.GetCount());
			ASSERT(shr_atom.param_idx >= 0 && shr_atom.param_idx < Key::max_len);
			int predicate_name0 = mask0->key.name;
			ASSERT(mask->keys.GetCount() >= src->atoms.GetCount());
			for(const auto& ws_atom : src->atoms) {
				if (mask0 != mask1 && ws_atom.in_use) {
					ASSERT(mask0->atom_idx != mask1->atom_idx);
					int predicate_name1 = mask1->key.name;
					if (predicate_name0 == predicate_name1 &&
						len0 == mask1->key.GetLength()) {
						const auto& param1 = mask1->key.params[shr_atom.param_idx];
						if (param1.val >= 0) {
							if (shr.value.val >= 0 && shr.value.val != param1.val) {
								err = "duplicate value";
								return false;
							}
							shr.value.val = param1.val;
						}
					}
				}
				mask1++;
			}
		}
		
		// If shared values wasn't found
		if (shr.value.val < 0) {
			
			// Use default value if set
			if (shr.value.def_val >= 0) {
				shr.value.val = shr.value.def_val;
			}
			else {
				String shr_name = mask->session->key_values[shr.key].ToString();
				err = "could not resolve shared variable '" + shr_name + "'";
				return false;
			}
		}
	}
	
	return true;
}

bool ActionParamResolver::MakeSharedPreCondition() {
	
	TODO
	
	return true;
}

bool ActionParamResolver::TestSharedPreCondition() {
	
	TODO
	
	return true;
}

bool ActionParamResolver::MakeDestination() {
	const BinaryWorldState& pre  = ev->precond;
	const BinaryWorldState& post = ev->postcond;
	const BinaryWorldState& src  = *this->src;
	
	dest = src;
	
	auto mask_begin = post.mask->keys.Begin();
	for(int i = 0; i < post.atoms.GetCount(); i++) {
		const auto& pa = post.atoms[i];
		if (!pa.in_use)
			continue;
		
		auto mask_it = mask_begin + i;
		const Key& src_key = mask_it->key;
		Key dst_key;
		dst_key.name = dst_key;
		for(int j = 0; j < Key::max_len; j++) {
			if (src_key.params[j].cls < 0) break;
			auto& dst_param = dst_key.params[j];
			if (!src_key.params[j].shared) {
				dst_param = src_key.params[j];
			}
			else {
				TODO
			}
		}
		
		dest.SetKey(dst_key, pa.value);
	}
	
	return true;
}

bool ActionParamResolver::MakeKeys() {
	const BinaryWorldState& pre = ev->precond;
	const BinaryWorldState& post = ev->postcond;
	
	ASSERT(pre.mask == post.mask && pre.mask == src->mask);
	if (!(pre.mask == post.mask && pre.mask == src->mask)) Panic("fatal internal error");
	
	DLOG("ACTION: " << ev->GetName());
	DLOG("<<<\n" << pre.ToString());
	DLOG(">>>\n" << post.ToString());
	
	// Find shared variables
	if (!FindSharedVariables(pre_count, pre, PRE))
		return false;
	
	// Basic case: pre-condition with fixed-param atoms
	if (!TestBasic())
		return false;
	
	// Find more shared variables
	if (!FindSharedVariables(post_count, post, POST))
		return false;
	if (!FindSharedVariables(-1, ev->key, ACTION))
		return false;
	
	// Collect all partial matching keys by "classpath" (== name of atom + param-count)
	// note: pass for performance reasons
	
	// Try to solve shared variables:
	// - from partially matching keys
	// - TODO: (possibly, but not likely) AI exchange point: create unsolved arguments
	if (!SolveShared())
		return false;
	
	// Check pre-condition with shared-param atoms
	if (!MakeSharedPreCondition())
		return false;
	if (!TestSharedPreCondition())
		return false;
	
	// Create new world-state with src&post world-states (with solved shared params)
	if (!MakeDestination())
		return false;
	
	// Assert that new world-state doesn't have atoms with shared params
	TODO
	
	return true;
}

END_UPP_NAMESPACE
