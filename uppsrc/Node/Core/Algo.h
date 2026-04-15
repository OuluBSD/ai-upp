#ifndef _Node_Core_Algo_h_
#define _Node_Core_Algo_h_

#include "Core.h"

namespace Upp {

namespace Node {

// Topological Sort
Vector<EntityId> TopologicalSort(const Graph& graph);

// Dijkstra Shortest Path
struct PathNode : Moveable<PathNode> {
	EntityId entity_id;
	double   distance = 1e300;
	EntityId predecessor;
};
Vector<PathNode> Dijkstra(const Graph& graph, const EntityId& start_node);

// Tree Layouts
void OrderedTreeLayout(const Graph& graph, Vector<NodeState>& states);
void TournamentTreeLayout(const Graph& graph, Vector<NodeState>& states);
void LayeredLayout(const Graph& graph, Vector<NodeState>& states);

} // namespace Node

} // namespace Upp

#endif
