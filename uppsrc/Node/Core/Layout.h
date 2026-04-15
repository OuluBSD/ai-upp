#ifndef _Node_Core_Layout_h_
#define _Node_Core_Layout_h_

#include "Core.h"

namespace Upp {

namespace Node {

class Layout {
public:
	virtual ~Layout() {}
	virtual void Run(Graph& graph, Vector<NodeState>& states) = 0;
};

class SpringLayout : public Layout {
	int iterations = 500;
	double max_repulsive_dist = 6.0;
	double k = 2.0;
	double c = 0.01;
	double max_vertex_move = 0.5;
	
	uint64 seed = 0;
	
public:
	SpringLayout& Iterations(int n) { iterations = n; return *this; }
	SpringLayout& Seed(uint64 s)    { seed = s; return *this; }
	
	virtual void Run(Graph& graph, Vector<NodeState>& states) override;
};

} // namespace Node

} // namespace Upp

#endif
