#ifndef _AICore2_SearchAlgos_h_
#define _AICore2_SearchAlgos_h_


/*double GetSearcherUtility(Val& n);
double GetSearcherEstimate(Val& n);
double GetSearcherDistance(Val& n, Val& dest);
bool TerminalTest(Val& n, Val** prev);
*/


class Generator {
protected:
	Event<Val&> set_value;
public:
	Generator() {}
	virtual ~Generator() {}
	
	void SetValueFunction(Event<Val&> e);
	virtual void SetParams(Value val) {}
	virtual bool Run(Val& fs) {return true;}
	//virtual void GenerateSubValues(NodeRoute& prev) {}
};

class GeneratorRandom : public Generator {
	int total = 0;
	int low = 0, high = 0;
public:
	GeneratorRandom();
	
	void SetParams(Value val) override;
	bool Run(Val& fs) override;
};

class Searcher {
public:
	Searcher();
	virtual ~Searcher() {}
	
	bool TerminalTest(Val& n, NodeRoute& prev);
	double Utility(Val& n);
	double Estimate(Val& n);
	double Distance(Val& n, Val& dest);
	
	Vector<Val*> Search(Val& src);
	
	virtual bool SearchBegin(Val& src) = 0;
	virtual bool SearchIteration() = 0;
	virtual Vector<Val*> SearchEnd() = 0;
};


class MiniMax : public Searcher {
	NodeRoute route;
	Vector<Val*> out;
	Val* ptr = 0;
	Val* prev = 0;
public:
	MiniMax();
	double MaxValue(Val& n, int* decision_pos=0);
	double MinValue(Val& n, int* decision_pos=0);
	bool SearchBegin(Val& src) override;
	bool SearchIteration() override;
	Vector<Val*> SearchEnd() override;
};


class AlphaBeta : public Searcher {
	NodeRoute route;
	Vector<Val*> out;
	Val* ptr = 0;
public:
	AlphaBeta();
	double MaxValue(Val& n, double alpha, double beta, int* decision_pos=0);
	double MinValue(Val& n, double alpha, double beta, int* decision_pos=0);
	bool SearchBegin(Val& src) override;
	bool SearchIteration() override;
	Vector<Val*> SearchEnd() override;
};


// UNINFORMED SEARCH STRATEGIES

class BreadthFirst : public Searcher {
	NodeRoute route;
	Vector<Val*> queue, next_queue;
	double v = 0;
	Val* ptr = 0;
public:
	BreadthFirst();
	bool SearchBegin(Val& src) override;
	bool SearchIteration() override;
	Vector<Val*> SearchEnd() override;
};


class UniformCost : public Searcher {
	NodeRoute route;
	Vector<Val*> frontier;
	double v = DBL_MAX;
	Val* ptr = 0;
public:
	UniformCost();
	bool SearchBegin(Val& src) override;
	bool SearchIteration() override;
	Vector<Val*> SearchEnd() override;
};

class DepthFirst : public Searcher {
	NodeRoute route;
	One<typename Val::IteratorDeep> it;
	Val* ptr = 0;
	Val* prev = 0;
	double v = 0;
public:
	DepthFirst();
	bool SearchBegin(Val& src) override;
	bool SearchIteration() override;
	Vector<Val*> SearchEnd() override;
};

class DepthLimited : public Searcher {
	int limit;
	NodeRoute route;
	One<typename Val::IteratorDeep> it;
	Val* ptr = 0;
	Val* prev = 0;
	double v = DBL_MAX;
public:
	DepthLimited();
	void SetLimit(int lim);
	bool SearchBegin(Val& src) override;
	bool SearchIteration() override;
	Vector<Val*> SearchEnd() override;
};




// INFORMED (HEURISTIC) SEARCH STRATEGIES


class BestFirst : public Searcher {
	NodeRoute route;
	Vector<Val*> out;
	Val* ptr = 0;
	Val* prev = 0;
public:
	BestFirst();
	bool SearchBegin(Val& src) override;
	bool SearchIteration() override;
	Vector<Val*> SearchEnd() override;
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
	Vector<Val*> out;
	Vector<double> worst_f_score;
	Vector<int> worst_id;
	NodeRoute route;
	
	// The set of nodes already evaluated.
	Array<NodePtr> nodes;
	Vector<NodePtr*> closed_set;
	
	// The set of currently discovered nodes still to be evaluated.
	// Initially, only the start node is known.
	Vector<NodePtr*> open_set;
	
	
	static int FindNode(const Vector<NodePtr*>& vec, const Val* ptr);
	
	bool Fail();
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
	Vector<Val*> ContinueSearch(Val& src);
	bool SearchBegin(Val& src) override;
	bool SearchIteration() override;
	Vector<Val*> SearchEnd() override;
};



#endif
