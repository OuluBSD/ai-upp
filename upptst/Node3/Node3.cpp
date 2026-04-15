#include <Node/Core/Algo.h>
#include <Node/Core/Layout.h>

using namespace Upp;
using namespace Upp::Node;

#define TCHECK(cond) \
	if(!(cond)) { LOG("FAIL: " #cond " (" __FILE__ ":" + AsString(__LINE__) + ")"); failures++; }

static int failures = 0;

void TestTopSort()
{
	Graph g;
	g.AddNode("a");
	g.AddNode("b");
	g.AddNode("c");
	g.AddEdge("e1", "a", "", "b", "");
	g.AddEdge("e2", "b", "", "c", "");

	Vector<EntityId> order = TopologicalSort(g);
	TCHECK(order.GetCount() == 3);

	// a must come before b, b before c
	int ia = -1, ib = -1, ic = -1;
	for(int i = 0; i < order.GetCount(); i++) {
		if(order[i] == "a") ia = i;
		if(order[i] == "b") ib = i;
		if(order[i] == "c") ic = i;
	}
	TCHECK(ia >= 0 && ib >= 0 && ic >= 0);
	TCHECK(ia < ib);
	TCHECK(ib < ic);
}

void TestTopSortCycle()
{
	// Cyclic graph: TopologicalSort should not crash, returns partial result
	Graph g;
	g.AddNode("x");
	g.AddNode("y");
	g.AddEdge("e1", "x", "", "y", "");
	g.AddEdge("e2", "y", "", "x", "");

	Vector<EntityId> order = TopologicalSort(g);
	TCHECK(order.GetCount() <= 2); // not all nodes may appear in cycle
}

void TestDijkstra()
{
	Graph g;
	g.AddNode("s");
	g.AddNode("m");
	g.AddNode("t");
	EdgeDoc& e1 = g.AddEdge("e1", "s", "", "m", "");
	e1.weight = 2.0;
	e1.directed = false;
	EdgeDoc& e2 = g.AddEdge("e2", "m", "", "t", "");
	e2.weight = 3.0;
	e2.directed = false;

	Vector<PathNode> res = Dijkstra(g, "s");
	TCHECK(res.GetCount() == 3);

	double ds = 1e300, dm = 1e300, dt = 1e300;
	for(const auto& pn : res) {
		if(pn.entity_id == "s") ds = pn.distance;
		if(pn.entity_id == "m") dm = pn.distance;
		if(pn.entity_id == "t") dt = pn.distance;
	}
	TCHECK(ds == 0.0);
	TCHECK(dm == 2.0);
	TCHECK(dt == 5.0);
}

void TestDijkstraUnreachable()
{
	Graph g;
	g.AddNode("a");
	g.AddNode("b"); // isolated

	Vector<PathNode> res = Dijkstra(g, "a");
	for(const auto& pn : res)
		if(pn.entity_id == "b")
			TCHECK(pn.distance == 1e300); // unreachable
}

void TestSpringLayout()
{
	Graph g;
	for(int i = 0; i < 5; i++) {
		EntityId id = "n" + AsString(i);
		g.AddNode(id);
	}
	g.AddEdge("e01", "n0", "", "n1", "");
	g.AddEdge("e12", "n1", "", "n2", "");
	g.AddEdge("e23", "n2", "", "n3", "");
	g.AddEdge("e34", "n3", "", "n4", "");

	Vector<NodeState> states;
	SpringLayout layout;
	layout.Iterations(200).Seed(42);
	layout.Run(g, states);

	TCHECK(states.GetCount() == 5);
	// After spring layout, positions should differ (not all zero)
	bool any_nonzero = false;
	for(const auto& s : states)
		if(s.layout_pos.x != 0 || s.layout_pos.y != 0) { any_nonzero = true; break; }
	TCHECK(any_nonzero);
}

void TestLayeredLayout()
{
	// Linear chain: a→b→c should get ranks 0,1,2
	Graph g;
	g.AddNode("a");
	g.AddNode("b");
	g.AddNode("c");
	g.AddEdge("e1", "a", "", "b", "");
	g.AddEdge("e2", "b", "", "c", "");

	Vector<NodeState> states;
	LayeredLayout(g, states);
	TCHECK(states.GetCount() == 3);

	Index<EntityId> ids;
	for(const auto& n : g.GetDoc().nodes) ids.Add(n.id);

	Pointf pa = states[ids.Find("a")].layout_pos;
	Pointf pb = states[ids.Find("b")].layout_pos;
	Pointf pc = states[ids.Find("c")].layout_pos;

	// Each successive rank should be further right (increasing x)
	TCHECK(pb.x > pa.x);
	TCHECK(pc.x > pb.x);
}

void TestLayeredLayoutEmpty()
{
	Graph g;
	Vector<NodeState> states;
	LayeredLayout(g, states); // Should not crash on empty graph
	TCHECK(states.GetCount() == 0);
}

void TestOrderedTreeLayout()
{
	Graph g;
	g.AddNode("root");
	g.AddNode("left");
	g.AddNode("right");
	g.AddEdge("e1", "root", "", "left", "");
	g.AddEdge("e2", "root", "", "right", "");

	Vector<NodeState> states;
	OrderedTreeLayout(g, states);
	TCHECK(states.GetCount() == 3);
}

void TestApplyLayout()
{
	// Verify that ApplyLayout copies layout_pos into NodeDoc::pos and
	// marks only moved nodes dirty (incremental invalidation).
	Graph g;
	g.AddNode("n0");
	g.AddNode("n1");
	g.AddNode("n2");

	Vector<NodeState> states;
	LayeredLayout(g, states);
	TCHECK(states.GetCount() == 3);

	// Before apply, nodes sit at default pos (0,0)
	for(int i = 0; i < 3; i++)
		TCHECK(g.GetDoc().nodes[i].pos == Pointf(0, 0));

	uint64 serial_before = g.GetSerial();
	g.ApplyLayout(states);

	// Serial must have advanced (something moved)
	TCHECK(g.GetSerial() != serial_before);

	// After apply, NodeDoc::pos must match layout_pos
	for(int i = 0; i < 3; i++)
		TCHECK(g.GetDoc().nodes[i].pos == states[i].layout_pos);

	// Dirty set must be non-empty (incremental invalidation, not full wipe)
	TCHECK(!g.GetDirtyEntities().IsEmpty());

	// Second apply with same states: no changes → serial unchanged
	uint64 serial_after = g.GetSerial();
	g.ApplyLayout(states);
	TCHECK(g.GetSerial() == serial_after);
}

CONSOLE_APP_MAIN
{
	TestTopSort();
	TestTopSortCycle();
	TestDijkstra();
	TestDijkstraUnreachable();
	TestSpringLayout();
	TestLayeredLayout();
	TestLayeredLayoutEmpty();
	TestOrderedTreeLayout();
	TestApplyLayout();

	if(failures)
		LOG("Node3: " + AsString(failures) + " FAILURE(S)");
	else
		LOG("Node3: all tests passed");

	SetExitCode(failures ? 1 : 0);
}
