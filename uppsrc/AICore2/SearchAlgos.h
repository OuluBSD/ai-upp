#ifndef _AICore2_SearchAlgos_h_
#define _AICore2_SearchAlgos_h_


double GetSearcherUtility(Nod& n);
double GetSearcherEstimate(Nod& n);
double GetSearcherDistance(Nod& n, Nod& dest);
bool TerminalTest(Nod& n, Nod** prev);



class Searcher {
	
public:
	
	Searcher() {}
	
	bool TerminalTest(Nod& n, Nod** prev=NULL) {return Upp::TerminalTest(n, prev);}
	double Utility(Nod& n) {return Upp::GetSearcherUtility(n);}
	double Estimate(Nod& n) {return Upp::GetSearcherEstimate(n);}
	double Distance(Nod& n, Nod& dest) {return Upp::GetSearcherDistance(n, dest);}
	
	virtual Vector<Nod*> Search(Nod& src) = 0;
	
	
};


class MiniMax : public Searcher {
	
public:
	MiniMax();
	double MaxValue(Nod& n, int* decision_pos=0);
	double MinValue(Nod& n, int* decision_pos=0);
	Vector<Nod*> Search(Nod& src) override;
};


class AlphaBeta : public Searcher {
	
public:
	AlphaBeta() {}
	
	double MaxValue(Nod& n, double alpha, double beta, int* decision_pos=0) {
		Nod* prev = 0;
		if (TerminalTest(n,&prev))
			return this->Utility(n);
		int pos = -1;
		double v = -DBL_MAX;
		for(int i = 0; i < n.GetCount(); i++) {
			// Find minimum...
			double d = MinValue(n.sub[i], alpha, beta);
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
	
	double MinValue(Nod& n, double alpha, double beta, int* decision_pos=0) {
		Nod* prev = 0;
		if (TerminalTest(n,&prev))
			return this->Utility(n);
		int pos = -1;
		double v = DBL_MAX;
		for(int i = 0; i < n.GetCount(); i++) {
			double d = MaxValue(n.sub[i], alpha, beta);
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
	
	Vector<Nod*> Search(Nod& src) override {
		Vector<Nod*> out;
		Nod* ptr = &src;
		Nod* prev = 0;
		while (1) {
			Nod& t = *out.Add((Nod*)ptr);
			if (TerminalTest(*ptr,&prev)) break;
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
			ptr = &(*ptr).sub[pos];
		}
		return out;
	}
};


// UNINFORMED SEARCH STRATEGIES

class BreadthFirst : public Searcher {
	
public:
	BreadthFirst() {}
	
	virtual Vector<Nod*> Search(Nod& src) {
		Vector<Nod*> out;
		Vector<Nod*> queue, next_queue;
		next_queue.Add(&src);
		double v = DBL_MAX;
		Nod* ptr = 0;
		Nod* prev = 0;
		while (1) {
			queue <<= next_queue;
			next_queue.Clear();
			
			bool all_terminals = true;
			for(int i = 0; i < queue.GetCount(); i++) {
				Nod& t = *queue[i];
				
				if (TerminalTest(t,&prev)) {
					ptr = &t;
					all_terminals = true;
					break;
				}
				else all_terminals = false;
				
				for(int j = 0; j < t.GetCount(); j++) {
					next_queue.Add(&t.sub[j]);
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
};


class UniformCost : public Searcher {
	
public:
	UniformCost() {}
	
	virtual Vector<Nod*> Search(Nod& src) {
		Vector<Nod*> out;
		Vector<Nod*> frontier;
		frontier.Add(&src);
		double v = DBL_MAX;
		Nod* ptr = 0;
		Nod* prev = 0;
		
		for(; frontier.GetCount();) {
			bool all_terminals = true;
			Nod& t = *frontier[0];
			if (TerminalTest(t,&prev)) {
				ptr = &t;
				all_terminals = true;
				break;
			}
			else all_terminals = false;
			
			frontier.Remove(0);
			
			//TODO: change this to search like in AStar, because it is faster than insert, which moves huge block of memory always. Or do something completely different and better.
			for(int j = 0; j < t.GetCount(); j++) {
				Nod& sub = t.sub[j];
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
};

class DepthFirst : public Searcher {
	
public:
	DepthFirst() {}
	
	virtual Vector<Nod*> Search(Nod& src) {
		Vector<Nod*> out;
		
		typename Nod::IteratorDeep it = src.BeginDeep();
		Nod* ptr = 0;
		Nod* prev = 0;
		double v = DBL_MAX;
		
		while (!it.IsEnd()) {
			if (TerminalTest(*it,&prev)) {
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
};

class DepthLimited : public Searcher {
	int limit;
	
public:
	DepthLimited() : limit(-1) {}
	
	void SetLimit(int lim) {limit = lim;}
	
	virtual Vector<Nod*> Search(Nod& src) {
		Vector<Nod*> out;
		
		typename Nod::IteratorDeep it = src.BeginDeep();
		Nod* ptr = 0;
		Nod* prev = 0;
		double v = DBL_MAX;
		
		while (!it.IsEnd()) {
			if (TerminalTest(*it,&prev)) {
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
};




// INFORMED (HEURISTIC) SEARCH STRATEGIES


class BestFirst : public Searcher {
	
public:
	BestFirst() {}
	
	virtual Vector<Nod*> Search(Nod& src) {
		Vector<Nod*> out;
		Nod* ptr = &src;
		Nod* prev = &src;
		while (1) {
			out.Add((Nod*)ptr);
			Nod& t = *ptr;
			if (TerminalTest(*ptr,&prev))
				break;
			int pos = -1;
			double v = DBL_MAX;
			for(int i = 0; i < t.GetCount(); i++) {
				Nod& sub = t.sub[i];
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
			ptr = &(*ptr).sub[pos];
		}
		return out;
	}
};


    
class AStar : public Searcher {
	
	struct NodePtr {
		Nod* ptr;
		NodePtr() : ptr(0), g_score(0), f_score(0), came_from(0) {g_score = DBL_MAX; f_score = DBL_MAX;}
		
		hash_t GetHashValue() const {return UPP::GetHashValue((size_t)ptr);}
		bool operator == (const NodePtr& np) const {return np.ptr == ptr;}
		// For each node, the cost of getting from the start node to that node.
		double g_score;
		// For each node, the total cost of getting from the start node to the goal
		// by passing by that node. That value is partly known, partly heuristic.
		double f_score;
		// For each node, which node it can most efficiently be reached from.
		// If a node can be reached from many nodes, came_from will eventually contain the
		// most efficient previous step.
		const NodePtr* came_from = 0;
	};
	
	int max_worst;
	bool do_search;
	int limit;
	int rm_limit = 1000;
	int smallest_id = -1;
	
	// The set of nodes already evaluated.
	Array<NodePtr> nodes;
	Vector<NodePtr*> closed_set;
	
	// The set of currently discovered nodes still to be evaluated.
	// Initially, only the start node is known.
	Vector<NodePtr*> open_set;
	
	
	static int FindNode(const Vector<NodePtr*>& vec, const Nod* ptr) {
		int i = 0;
		for (const NodePtr* p : vec) {
			if (p->ptr == ptr)
				return i;
			i++;
		}
		return -1;
	}
	
public:
	AStar() : max_worst(0), limit(0) {}
	
	void operator=(const AStar& as) {
		max_worst = as.max_worst;
		do_search = as.do_search;
		limit = as.limit;
		closed_set <<= as.closed_set;
		open_set <<= as.open_set;
	}
	void SetLimit(int i) {limit = i;}
	void Stop() {do_search = false;}
	
	void TrimWorst(int limit, int count) {rm_limit = limit; max_worst = count; ASSERT(count >= 0);}
	
	Vector<Nod*> GetBestKnownPath() {
		if (smallest_id < 0)
			return Vector<Nod*>();
		
		NodePtr* t_ptr = open_set[smallest_id];
		Nod& t = *t_ptr->ptr;
		return ReconstructPath(t, closed_set, open_set);
	}
	
	Vector<Nod*> ReconstructPath(Nod& current, Vector<NodePtr*>& closed_set, Vector<NodePtr*>& open_set) {
		Vector<Nod*> path;
		Nod* ptr = &current;
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
		Vector<Nod*> out;
		out.SetCount(path.GetCount());
		for(int i = 0; i < path.GetCount(); i++) {
			out[path.GetCount()-1-i] = path[i];
		}
		return out;
	}
	
	Vector<Nod*> ReconstructPath(Nod& current) {
		return ReconstructPath(current, closed_set, open_set);
	}
	
	Vector<Nod*> GetBest() {
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
		
		return Vector<Nod*>();
	}
	
	virtual Vector<Nod*> Search(Nod& src) {
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
	
	Vector<Nod*> ContinueSearch(Nod& src) {
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
			return Vector<Nod*>();
		
		closed_set.Append(open_set);
		open_set.Clear();
		
		open_set.Add(copy);
		
		return SearchMain();
	}
	
	Vector<Nod*> SearchMain() {
		Vector<double> worst_f_score;
		Vector<int> worst_id;
		worst_f_score.SetCount(max_worst);
		worst_id.SetCount(max_worst);
		
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
			
			
			Nod* prevs[5];
			NodePtr* t_ptr = open_set[smallest_id];
			Nod& t = *t_ptr->ptr;
			const NodePtr* prev = t_ptr->came_from;
			for(int i = 0; i < 4; i++) {
				if (prev) {
					prevs[i] = prev->ptr;
					prev = prev->came_from;
				}
				else prevs[i] = 0;
			}
			prevs[4] = 0;
			double current_g_score = t_ptr->g_score;
			
			if (TerminalTest(t, prevs))
				return ReconstructPath(t, closed_set, open_set);
			
			if (!do_search)
				break;
			
			open_set.Remove(smallest_id);
			smallest_id = -1;
			closed_set.Add(t_ptr);
			
			
			for(int j = 0; j < t.GetCount(); j++) {
				Nod& sub = t.sub[j];
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
		return Vector<Nod*>();
	}
};



#endif
