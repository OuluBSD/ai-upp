#ifndef _AICore2_SearchAlgos_h_
#define _AICore2_SearchAlgos_h_


/*double GetSearcherUtility(Nod& n);
double GetSearcherEstimate(Nod& n);
double GetSearcherDistance(Nod& n, Nod& dest);
bool TerminalTest(Nod& n, Nod** prev);
*/


class Searcher {
	
public:
	
	Searcher();
	
	bool TerminalTest(Nod& n, NodeRoute& prev);
	double Utility(Nod& n);
	double Estimate(Nod& n);
	double Distance(Nod& n, Nod& dest);
	
	virtual Vector<Nod*> Search(Nod& src) = 0;
	
	
};


class MiniMax : public Searcher {
	NodeRoute route;
public:
	MiniMax();
	double MaxValue(Nod& n, int* decision_pos=0);
	double MinValue(Nod& n, int* decision_pos=0);
	Vector<Nod*> Search(Nod& src) override;
};


class AlphaBeta : public Searcher {
	NodeRoute route;
public:
	AlphaBeta();
	double MaxValue(Nod& n, double alpha, double beta, int* decision_pos=0);
	double MinValue(Nod& n, double alpha, double beta, int* decision_pos=0);
	Vector<Nod*> Search(Nod& src) override;
};


// UNINFORMED SEARCH STRATEGIES

class BreadthFirst : public Searcher {
	NodeRoute route;
public:
	BreadthFirst();
	virtual Vector<Nod*> Search(Nod& src);
};


class UniformCost : public Searcher {
	NodeRoute route;
public:
	UniformCost();
	virtual Vector<Nod*> Search(Nod& src);
};

class DepthFirst : public Searcher {
	NodeRoute route;
public:
	DepthFirst();
	virtual Vector<Nod*> Search(Nod& src);
};

class DepthLimited : public Searcher {
	int limit;
	NodeRoute route;
	
public:
	DepthLimited();
	void SetLimit(int lim);
	virtual Vector<Nod*> Search(Nod& src);
};




// INFORMED (HEURISTIC) SEARCH STRATEGIES


class BestFirst : public Searcher {
	NodeRoute route;
public:
	BestFirst();
	virtual Vector<Nod*> Search(Nod& src);
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
	
	
	static int FindNode(const Vector<NodePtr*>& vec, const Nod* ptr);
	
public:
	AStar();
	void operator=(const AStar& as);
	void SetLimit(int i);
	void Stop();
	void TrimWorst(int limit, int count);
	Vector<Nod*> GetBestKnownPath();
	Vector<Nod*> ReconstructPath(Nod& current, Vector<NodePtr*>& closed_set, Vector<NodePtr*>& open_set);
	Vector<Nod*> ReconstructPath(Nod& current);
	Vector<Nod*> GetBest();
	Vector<Nod*> Search(Nod& src) override;
	Vector<Nod*> ContinueSearch(Nod& src);
	Vector<Nod*> SearchMain();
};



#endif
