#include "Graph.h"

namespace GraphLib {

Pin& Node::AddPin(String id, PinKind kind, int type) {
	Pin& p = pins.Add();
	p.id = id;
	p.kind = kind;
	p.type = type;
	p.label = id;
	// Calculate default position relative to node
	if (kind == PinKind::Input)
		p.position = Pointf(0, sz.cy / 2);
	else
		p.position = Pointf(sz.cx, sz.cy / 2);
	return p;
}

Pin* Node::FindPin(String id) {
	for(int i = 0; i < pins.GetCount(); i++)
		if(pins[i].id == id) return &pins[i];
	return NULL;
}

Node& Graph::AddNode(const String& id) {
	int i = FindNode(id);
	if (i >= 0) return nodes[i];
	
	Node& n = nodes.Add();
	n.id = id;
	n.SetLabel(id);
	return n;
}

void Graph::RemoveNode(const String& id) {
	int i = FindNode(id);
	if (i < 0) return;
	
	// Remove connected edges
	Node* n = &nodes[i];
	for(int j = edges.GetCount()-1; j >= 0; j--) {
		if (edges[j].source == n || edges[j].target == n)
			edges.Remove(j);
	}
	
	nodes.Remove(i);
}

int Graph::FindNode(const String& id) {
	for(int i = 0; i < nodes.GetCount(); i++)
		if (nodes[i].id == id) return i;
	return -1;
}

Edge& Graph::AddEdge(const String& n1, const String& n2, double weight) {
	Node& node1 = AddNode(n1);
	Node& node2 = AddNode(n2);
	
	Edge& e = edges.Add();
	e.source = &node1;
	e.target = &node2;
	e.SetWeight(weight);
	
	node1.edges.Add(&e);
	node2.edges.Add(&e);
	
	return e;
}

Edge& Graph::AddEdge(const String& n1, const String& p1, const String& n2, const String& p2, double weight) {
	Node& node1 = AddNode(n1);
	Node& node2 = AddNode(n2);
	
	Edge& e = edges.Add();
	e.source = &node1;
	e.target = &node2;
	e.SetWeight(weight);
	
	// Connect pins if they exist
	Pin* pin1 = node1.FindPin(p1);
	Pin* pin2 = node2.FindPin(p2);
	if (pin1) pin1->connections.Add(&e);
	if (pin2) pin2->connections.Add(&e);
	
	node1.edges.Add(&e);
	node2.edges.Add(&e);
	
	return e;
}

void Graph::RemoveEdge(Edge& e) {
	for(int i = 0; i < edges.GetCount(); i++) {
		if (&edges[i] == &e) {
			// Remove from node edge lists
			if (e.source) {
				for(int j = 0; j < e.source->edges.GetCount(); j++)
					if (e.source->edges[j] == &e) { e.source->edges.Remove(j); break; }
			}
			if (e.target) {
				for(int j = 0; j < e.target->edges.GetCount(); j++)
					if (e.target->edges[j] == &e) { e.target->edges.Remove(j); break; }
			}
			edges.Remove(i);
			return;
		}
	}
}

GroupNode& Graph::AddGroup(const String& id) {
	int i = FindGroup(id);
	if (i >= 0) return groups[i];
	
	GroupNode& g = groups.Add();
	g.id = id;
	g.label = id;
	return g;
}

void Graph::RemoveGroup(const String& id) {
	int i = FindGroup(id);
	if (i >= 0) groups.Remove(i);
}

int Graph::FindGroup(const String& id) {
	for(int i = 0; i < groups.GetCount(); i++)
		if (groups[i].id == id) return i;
	return -1;
}

void Graph::MoveNodeToGroup(const String& nodeId, const String& groupId) {
	// First remove node from any existing group
	for(int i = 0; i < groups.GetCount(); i++) {
		int q = -1;
		for(int j = 0; j < groups[i].node_ids.GetCount(); j++) {
			if(groups[i].node_ids[j] == nodeId) {
				q = j;
				break;
			}
		}
		if(q >= 0) groups[i].node_ids.Remove(q);
	}
	
	// Add to new group if specified
	if(!groupId.IsEmpty()) {
		int i = FindGroup(groupId);
		if(i >= 0) {
			groups[i].node_ids.Add(nodeId);
		}
	}
}

void Graph::Clear() {
	nodes.Clear();
	edges.Clear();
	groups.Clear();
}

}
