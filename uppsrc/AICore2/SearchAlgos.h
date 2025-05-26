#ifndef _AICore2_SearchAlgos_h_
#define _AICore2_SearchAlgos_h_


/*double GetSearcherUtility(Val& n);
double GetSearcherEstimate(Val& n);
double GetSearcherDistance(Val& n, Val& dest);
bool TerminalTest(Val& n, Val** prev);
*/


class Searcher {
	
public:
	
	Searcher();
	
	bool TerminalTest(Val& n, NodeRoute& prev);
	double Utility(Val& n);
	double Estimate(Val& n);
	double Distance(Val& n, Val& dest);
	
	virtual Vector<Val*> Search(Val& src) = 0;
	
	
};


class MiniMax : public Searcher {
	NodeRoute route;
public:
	MiniMax();
	double MaxValue(Val& n, int* decision_pos=0);
	double MinValue(Val& n, int* decision_pos=0);
	Vector<Val*> Search(Val& src) override;
};


class AlphaBeta : public Searcher {
	NodeRoute route;
public:
	AlphaBeta();
	double MaxValue(Val& n, double alpha, double beta, int* decision_pos=0);
	double MinValue(Val& n, double alpha, double beta, int* decision_pos=0);
	Vector<Val*> Search(Val& src) override;
};


// UNINFORMED SEARCH STRATEGIES

class BreadthFirst : public Searcher {
	NodeRoute route;
public:
	BreadthFirst();
	virtual Vector<Val*> Search(Val& src);
};


class UniformCost : public Searcher {
	NodeRoute route;
public:
	UniformCost();
	virtual Vector<Val*> Search(Val& src);
};

class DepthFirst : public Searcher {
	NodeRoute route;
public:
	DepthFirst();
	virtual Vector<Val*> Search(Val& src);
};

class DepthLimited : public Searcher {
	int limit;
	NodeRoute route;
	
public:
	DepthLimited();
	void SetLimit(int lim);
	virtual Vector<Val*> Search(Val& src);
};




// INFORMED (HEURISTIC) SEARCH STRATEGIES


class BestFirst : public Searcher {
	NodeRoute route;
public:
	BestFirst();
	virtual Vector<Val*> Search(Val& src);
};


    
class AStar : public Searcher {
	
	struct NodePtr {
		Val* ptr;
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
	
	
	static int FindNode(const Vector<NodePtr*>& vec, const Val* ptr);
	
public:
	AStar();
	void operator=(const AStar& as);
	void SetLimit(int i);
	void Stop();
	void TrimWorst(int limit, int count);
	Vector<Val*> GetBestKnownPath();
	Vector<Val*> ReconstructPath(Val& current, Vector<NodePtr*>& closed_set, Vector<NodePtr*>& open_set);
	Vector<Val*> ReconstructPath(Val& current);
	Vector<Val*> GetBest();
	Vector<Val*> Search(Val& src) override;
	Vector<Val*> ContinueSearch(Val& src);
	Vector<Val*> SearchMain();
};



#endif
