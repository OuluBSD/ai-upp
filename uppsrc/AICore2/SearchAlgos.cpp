#include "AICore.h"

NAMESPACE_UPP


Searcher::Searcher() {
	
}

bool Searcher::TerminalTest(Val& n, NodeRoute& prev) {
	ASSERT(n.ext);
	return n.ext->TerminalTest(prev);
}

double Searcher::Utility(Val& n) {
	ASSERT(n.ext);
	return n.ext->GetUtility();
}

double Searcher::Estimate(Val& n) {
	ASSERT(n.ext);
	return n.ext->GetEstimate();
}

double Searcher::Distance(Val& n, Val& dest) {
	ASSERT(n.ext);
	return n.ext->GetDistance(dest);
}





MiniMax::MiniMax() {route.from_owner_only = true;}
	
double MiniMax::MaxValue(Val& n, int* decision_pos) {
	Val* p = 0;
	if (TerminalTest(n,route))
		return this->Utility(n);
	double v = -DBL_MAX;
	int pos = -1;
	for(int i = 0; i < n.GetCount(); i++) {
		// Find minimum...
		double d = MinValue(n[i]);
		// ..but use maximum
		if (d > v) {
			v = d;
			pos = i;
		}
	}
	if (decision_pos) *decision_pos = pos;
	return v;
}

double MiniMax::MinValue(Val& n, int* decision_pos) {
	Val* p = 0;
	if (TerminalTest(n,route))
		return Searcher::Utility(n);
	double v = DBL_MAX;
	int pos = -1;
	for(int i = 0; i < n.GetCount(); i++) {
		double d = MaxValue(n[i]);
		if (d < v) {
			v = d;
			pos = i;
		}
	}
	if (decision_pos) *decision_pos = pos;
	return v;
}

Vector<Val*> MiniMax::Search(Val& src) {
	Vector<Val*> out;
	Val* ptr = &src;
	Val* prev = 0;
	while (1) {
		Val& t = *out.Add(ptr);
		if (TerminalTest(*ptr,route)) break;
		int type = out.GetCount() % 2;
		int pos = -1;
		double v;
		if (type == 1)
			v = MinValue(*ptr, &pos);
		else
			v = MaxValue(*ptr, &pos);
		if (pos == -1) break;
		ptr = &(*ptr)[pos];
		//LOG(pos << " " << v);
	}
	return out;
}

















AlphaBeta::AlphaBeta() {route.from_owner_only = true;}

double AlphaBeta::MaxValue(Val& n, double alpha, double beta, int* decision_pos) {
	if (TerminalTest(n,route))
		return this->Utility(n);
	int pos = -1;
	double v = -DBL_MAX;
	for(int i = 0; i < n.GetCount(); i++) {
		// Find minimum...
		double d = MinValue(n[i], alpha, beta);
		// ..but use maximum
		if (d > v) {
			v = d;
			pos = i;
		}
		if (v > alpha)
			alpha = v;
		if (beta <= alpha)
			break;
	}
	if (decision_pos) *decision_pos = pos;
	return v;
}

double AlphaBeta::MinValue(Val& n, double alpha, double beta, int* decision_pos) {
	if (TerminalTest(n,route))
		return this->Utility(n);
	int pos = -1;
	double v = DBL_MAX;
	for(int i = 0; i < n.GetCount(); i++) {
		double d = MaxValue(n[i], alpha, beta);
		if (d < v) {
			v = d;
			pos = i;
		}
		if (v < beta)
			beta = v;
		if (beta <= alpha)
			break;
	}
	if (decision_pos) *decision_pos = pos;
	return v;
}

Vector<Val*> AlphaBeta::Search(Val& src) {
	Vector<Val*> out;
	Val* ptr = &src;
	while (1) {
		Val& t = *out.Add((Val*)ptr);
		if (TerminalTest(*ptr,route)) break;
		int type = out.GetCount() % 2;
		int pos = -1;
		double v;
		double alpha = -DBL_MAX;
		double beta = DBL_MAX;
		if (type == 1)
			v = MinValue(*ptr, alpha, beta, &pos);
		else
			v = MaxValue(*ptr, alpha, beta, &pos);
		if (pos == -1) break;
		ptr = &(*ptr)[pos];
	}
	return out;
}















BreadthFirst::BreadthFirst() {route.from_owner_only = true;}

Vector<Val*> BreadthFirst::Search(Val& src) {
	Vector<Val*> out;
	Vector<Val*> queue, next_queue;
	next_queue.Add(&src);
	double v = DBL_MAX;
	Val* ptr = 0;
	while (1) {
		queue <<= next_queue;
		next_queue.Clear();
		
		bool all_terminals = true;
		for(int i = 0; i < queue.GetCount(); i++) {
			Val& t = *queue[i];
			
			if (TerminalTest(t,route)) {
				ptr = &t;
				all_terminals = true;
				break;
			}
			else all_terminals = false;
			
			for(int j = 0; j < t.GetCount(); j++) {
				next_queue.Add(&t[j]);
			}
		}
		if (all_terminals) break;
	}
	
	while (ptr) {
		out.Insert(0, ptr);
		ptr = ptr->owner;
	}
	return out;
}








UniformCost::UniformCost() {route.from_owner_only = true;}

Vector<Val*> UniformCost::Search(Val& src) {
	Vector<Val*> out;
	Vector<Val*> frontier;
	frontier.Add(&src);
	double v = DBL_MAX;
	Val* ptr = 0;
	
	for(; frontier.GetCount();) {
		bool all_terminals = true;
		Val& t = *frontier[0];
		if (TerminalTest(t,route)) {
			ptr = &t;
			all_terminals = true;
			break;
		}
		else all_terminals = false;
		
		frontier.Remove(0);
		
		//TODO: change this to search like in AStar, because it is faster than insert, which moves huge block of memory always. Or do something completely different and better.
		for(int j = 0; j < t.GetCount(); j++) {
			Val& sub = t[j];
			double utility = this->Utility(sub);
			bool inserted = false;
			for(int k = 0; k < frontier.GetCount(); k++) {
				double f_utility = this->Utility(*frontier[k]);
				if (utility <= f_utility) {
					frontier.Insert(k, &sub);
					inserted = true;
					break;
				}
			}
			// Worst utility
			if (!inserted) {
				frontier.Add(&sub);
			}
		}
		if (all_terminals) break;
	}
	
	while (ptr) {
		out.Insert(0, ptr);
		ptr = ptr->owner;
	}
	return out;
}











DepthFirst::DepthFirst() {route.from_owner_only = true;}

Vector<Val*> DepthFirst::Search(Val& src) {
	Vector<Val*> out;
	
	typename Val::IteratorDeep it = src.BeginDeep();
	Val* ptr = 0;
	Val* prev = 0;
	double v = DBL_MAX;
	
	while (!it.IsEnd()) {
		if (TerminalTest(*it,route)) {
			ptr = it;
			break;
		}
		it++;
	}
	
	while (ptr) {
		out.Insert(0, ptr);
		ptr = ptr->owner;
	}
	
	return out;
}















DepthLimited::DepthLimited() : limit(-1) {route.from_owner_only = true;}

void DepthLimited::SetLimit(int lim) {limit = lim;}

Vector<Val*> DepthLimited::Search(Val& src) {
	Vector<Val*> out;
	
	typename Val::IteratorDeep it = src.BeginDeep();
	Val* ptr = 0;
	Val* prev = 0;
	double v = DBL_MAX;
	
	while (!it.IsEnd()) {
		if (TerminalTest(*it,route)) {
			ptr = it;
			break;
		}
		int depth = it.GetDepth();
		if (depth >= limit)
			it.IncNotDeep();
		else
			it++;
	}
	
	while (ptr) {
		out.Insert(0, ptr);
		ptr = ptr->owner;
	}
	
	return out;
}
















BestFirst::BestFirst() {route.from_owner_only = true;}

Vector<Val*> BestFirst::Search(Val& src) {
	Vector<Val*> out;
	Val* ptr = &src;
	Val* prev = &src;
	while (1) {
		out.Add((Val*)ptr);
		Val& t = *ptr;
		if (TerminalTest(*ptr,route))
			break;
		int pos = -1;
		double v = DBL_MAX;
		for(int i = 0; i < t.GetCount(); i++) {
			Val& sub = t[i];
			double estimate = this->Estimate(sub);
			if (estimate < v) {
				v = estimate;
				pos = i;
				if (v <= 0)
					break;
			}
		}
		if (pos == -1)
			break;
		ptr = &(*ptr)[pos];
	}
	return out;
}














int AStar::FindNode(const Vector<NodePtr*>& vec, const Val* ptr) {
	int i = 0;
	for (const NodePtr* p : vec) {
		if (p->ptr == ptr)
			return i;
		i++;
	}
	return -1;
}

AStar::AStar() : max_worst(0), limit(0) {}

void AStar::operator=(const AStar& as) {
	max_worst = as.max_worst;
	do_search = as.do_search;
	limit = as.limit;
	closed_set <<= as.closed_set;
	open_set <<= as.open_set;
}
void AStar::SetLimit(int i) {limit = i;}
void AStar::Stop() {do_search = false;}

void AStar::TrimWorst(int limit, int count) {rm_limit = limit; max_worst = count; ASSERT(count >= 0);}

Vector<Val*> AStar::GetBestKnownPath() {
	if (smallest_id < 0)
		return Vector<Val*>();
	
	NodePtr* t_ptr = open_set[smallest_id];
	Val& t = *t_ptr->ptr;
	return ReconstructPath(t, closed_set, open_set);
}

Vector<Val*> AStar::ReconstructPath(Val& current, Vector<NodePtr*>& closed_set, Vector<NodePtr*>& open_set) {
	Vector<Val*> path;
	Val* ptr = &current;
	while (1) {
		path.Add(ptr);
		int i = FindNode(open_set, ptr);
		if (i == -1) {
			i = FindNode(closed_set, ptr);
			if (i == -1) break;
			else {
				const NodePtr* cf = closed_set[i]->came_from;
				ptr = cf ? cf->ptr : 0;
			}
		}
		else ptr = open_set[i]->came_from->ptr;
		if (!ptr) break;
	}
	Vector<Val*> out;
	out.SetCount(path.GetCount());
	for(int i = 0; i < path.GetCount(); i++) {
		out[path.GetCount()-1-i] = path[i];
	}
	return out;
}

Vector<Val*> AStar::ReconstructPath(Val& current) {
	return ReconstructPath(current, closed_set, open_set);
}

Vector<Val*> AStar::GetBest() {
	NodePtr* n = 0;
	double best_score = DBL_MAX;
	for (const NodePtr* p : closed_set) {
		double score = p->g_score + p->f_score;
		if (score < best_score) {
			best_score = score;
			n = const_cast<NodePtr*>(p);
		}
	}
	if (n)
		return ReconstructPath(*n->ptr, closed_set, open_set);
	
	return Vector<Val*>();
}

Vector<Val*> AStar::Search(Val& src) {
	do_search = true;
	
	closed_set.Clear();
	open_set.Clear();
	
	// For the first node, that value is completely heuristic.
	NodePtr& np = nodes.Add();
	np.ptr = &src;
	np.g_score = 0;
	np.f_score = this->Estimate(src);
	open_set.Add(&np);
	
	return SearchMain();
}

Vector<Val*> AStar::ContinueSearch(Val& src) {
	do_search = true;
	
	NodePtr* copy = 0;
	int i = 0;
	for (NodePtr* np : open_set) {
		if (np->ptr == &src) {
			copy = np;
			open_set.Remove(i);
			break;
		}
		++i;
	}
	if (!copy || !copy->ptr)
		return Vector<Val*>();
	
	closed_set.Append(open_set);
	open_set.Clear();
	
	open_set.Add(copy);
	
	return SearchMain();
}

Vector<Val*> AStar::SearchMain() {
	Vector<double> worst_f_score;
	Vector<int> worst_id;
	worst_f_score.SetCount(max_worst);
	worst_id.SetCount(max_worst);
	NodeRoute route;
	
	// while open_set is not empty
	for(; open_set.GetCount() && do_search;) {
		
		if (limit) {
			limit--;
			if (limit <= 0)
				do_search = 0;
		}
		
		bool rm = open_set.GetCount() > rm_limit;
		if (rm) {
			for(int i = 0; i < max_worst; i++) {
				worst_f_score[i] = -DBL_MAX;
				worst_id[i] = -1;
			}

			for(int i = 0; i < open_set.GetCount(); i++) {
				const NodePtr& nptr = *open_set[i];
				double f_score = nptr.f_score;
				
				for(int j = 0; j < max_worst; j++) {
					if (f_score > worst_f_score[j]) {
						for(int k = max_worst-1; k > j; k--) {
							worst_f_score[k] = worst_f_score[k-1];
							worst_id[k] = worst_id[k-1];
						}
						worst_f_score[j] = f_score;
						worst_id[j] = i;
						break;
					}
				}
			}
			
			int count = 0;
			Vector<int> rm_list;
			for(int i = 0; i < max_worst; i++) {
				int id = worst_id[i];
				if (id == -1) break;
				closed_set.Add(open_set[id]);
				rm_list.Add(id);
			}
			
			if (rm_list.GetCount()) {
				if (rm_list.GetCount() > 1) {
					Sort(rm_list, StdLess<int>());
				}
				open_set.Remove(rm_list);
			}
		}
		
		
		double smallest_f_score = DBL_MAX;
		smallest_id = -1;
		
		for(int i = 0; i < open_set.GetCount(); i++) {
			const NodePtr& nptr = *open_set[i];
			double f_score = nptr.f_score;
			if (f_score < smallest_f_score) {
				smallest_f_score = f_score;
				smallest_id = i;
			}
		}
		
		
		route.route.SetCount(0);
		NodePtr* t_ptr = open_set[smallest_id];
		Val& t = *t_ptr->ptr;
		const NodePtr* prev = t_ptr->came_from;
		for(int i = 0; i < 50; i++) {
			if (prev) {
				route.route.Add(prev->ptr);
				prev = prev->came_from;
			}
			else break;
		}
		double current_g_score = t_ptr->g_score;
		
		if (TerminalTest(t, route))
			return ReconstructPath(t, closed_set, open_set);
		
		if (!do_search)
			break;
		
		open_set.Remove(smallest_id);
		smallest_id = -1;
		closed_set.Add(t_ptr);
		
		
		for(int j = 0; j < t.GetCount(); j++) {
			Val& sub = t[j];
			if (FindNode(closed_set, &sub) != -1)
				continue; // Ignore the neighbor which is already evaluated.
			// The distance from start to a neighbor
			double sub_g_score = current_g_score + this->Distance(t, sub);
			double sub_f_score = sub_g_score + this->Estimate(sub);
			int k = FindNode(open_set, &sub);
			if (k == -1) {
				// Discover a new node
				k = open_set.GetCount();
				NodePtr& subptr = nodes.Add();
				subptr.ptr = &sub;
				subptr.came_from = t_ptr;
				subptr.f_score = sub_f_score;
				subptr.g_score = sub_g_score;
				ASSERT(subptr.ptr);
				open_set.Add(&subptr);
			}
			else if (sub_g_score >= current_g_score)
				continue; // This is not a better path.
			else {
				NodePtr& subptr = *open_set[k];
				ASSERT(subptr.ptr == &sub);
				ASSERT(subptr.came_from == t_ptr);
				ASSERT(subptr.ptr);
				subptr.f_score = sub_f_score;
				subptr.g_score = sub_g_score;
				open_set.Remove(k);
			}
		}
	}
	return Vector<Val*>();
}

END_UPP_NAMESPACE
