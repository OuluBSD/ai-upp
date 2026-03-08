#ifndef _AI_Algo_FastSearch_h_
#define _AI_Algo_FastSearch_h_

#include <Core/Core.h>

namespace Upp {

template <class Node>
class FastSearchGenerator {
public:
	virtual void Generate(const Node& node, Vector<Node>& out) = 0;
	virtual bool IsGoal(const Node& node) = 0;
	virtual double GetCost(const Node& from, const Node& to) { return 1.0; }
	virtual double GetEstimate(const Node& node) { return 0.0; }
	virtual ~FastSearchGenerator() {}
};

template <class Node>
class FastSearch {
public:
	static bool BFS(Node start, FastSearchGenerator<Node>& generator, int limit = 1000) {
		Vector<Node> frontier;
		frontier.Add(start);
		
		Index<Node> visited;
		visited.Add(start);
		
		int head = 0;
		int count = 0;
		
		while(head < frontier.GetCount() && count++ < limit) {
			Node current = frontier[head++];
			
			if (generator.IsGoal(current))
				return true;
			
			Vector<Node> next;
			generator.Generate(current, next);
			
			for(int i = 0; i < next.GetCount(); i++) {
				const Node& n = next[i];
				if (visited.Find(n) == -1) {
					visited.Add(n);
					frontier.Add(n);
				}
			}
		}
		
		return false;
	}
};

}

#endif
