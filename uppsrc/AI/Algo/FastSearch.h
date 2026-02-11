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
	struct AStarNode {
		Node node;
		double g = DBL_MAX;
		double f = DBL_MAX;
		
		bool operator<(const AStarNode& other) const { return f > other.f; } // Min-heap
	};

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
	
	static bool AStar(Node start, FastSearchGenerator<Node>& generator, int limit = 1000) {
		Vector<AStarNode> open_set;
		AStarNode s;
		s.node = start;
		s.g = 0;
		s.f = generator.GetEstimate(start);
		open_set.Add(s);
		
		Index<Node> visited;
		
		int count = 0;
		while(open_set.GetCount() > 0 && count++ < limit) {
			// Get node with smallest f
			int best_idx = 0;
			for(int i = 1; i < open_set.GetCount(); i++)
				if (open_set[i].f < open_set[best_idx].f)
					best_idx = i;
			
			AStarNode current = open_set[best_idx];
			open_set.Remove(best_idx);
			
			if (generator.IsGoal(current.node))
				return true;
			
			if (visited.Find(current.node) != -1) continue;
			visited.Add(current.node);
			
			Vector<Node> next;
			generator.Generate(current.node, next);
			
			for(int i = 0; i < next.GetCount(); i++) {
				const Node& n = next[i];
				if (visited.Find(n) != -1) continue;
				
				AStarNode next_node;
				next_node.node = n;
				next_node.g = current.g + generator.GetCost(current.node, n);
				next_node.f = next_node.g + generator.GetEstimate(n);
				open_set.Add(next_node);
			}
		}
		
		return false;
	}
};

}

#endif