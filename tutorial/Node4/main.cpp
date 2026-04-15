#include <Node/Ctrl/Ctrl.h>
#include <Node/Core/Algo.h>
#include <Node/Core/Layout.h>

using namespace Upp;
using namespace Upp::Node;

// Node4: Weighted graph + shortest path (Dijkstra)
// Demonstrates: weighted edges, Dijkstra pathfinding, visual route highlighting

struct App : public TopWindow {
	Graph            graph;
	EditorState      editor;
	HistoryStack     history;
	CommandDispatcher dispatcher;
	NodeViewportCtrl  viewport;

	EntityId start_node = "Berlin";

	void RunDijkstra() {
		// Run Dijkstra from start node
		Vector<PathNode> result = Dijkstra(graph, start_node);

		// Reset all edge highlights
		for(auto& e : graph.GetDoc().edges) {
			e.line_width = 1;
			e.stroke_clr = Black();
		}

		// Update node labels with distances and highlight shortest-path edges
		Index<EntityId> ids;
		for(const auto& n : graph.GetDoc().nodes) ids.Add(n.id);

		for(const auto& pn : result) {
			NodeDoc* nd = graph.FindNode(pn.entity_id);
			if(nd) {
				// Keep original city name, append distance
				String dist = (pn.distance >= 1e200) ? "∞" : AsString((int)pn.distance);
				nd->label = nd->id + "\n(" + dist + ")";
			}

			// Highlight the edge from predecessor
			if(!pn.predecessor.IsEmpty()) {
				for(auto& e : graph.GetDoc().edges) {
					if((e.source_node == pn.predecessor && e.target_node == pn.entity_id) ||
					   (e.source_node == pn.entity_id && e.target_node == pn.predecessor)) {
						e.line_width = 3;
						e.stroke_clr = Color(0x00, 0xAA, 0xFF); // Blue highlight
					}
				}
			}
		}

		graph.Invalidate();
	}

	App() {
		Title("Node Tutorial 4: Shortest Path");
		Sizeable().Zoomable();

		dispatcher.RegisterStandardCommands();

		viewport.SetGraph(graph);
		viewport.SetEditor(editor);
		viewport.SetHistory(history);
		viewport.SetDispatcher(dispatcher);
		viewport.WhenNodeClick = [this](const EntityId& id) {
			start_node = id;
			RunDijkstra();
			viewport.Refresh();
		};

		Add(viewport.SizePos());

		CommandContext ctx(graph, editor);

		// Add city nodes via commands
		history.Begin();
		struct City { String id; };
		City cities[] = {
			{ "New_York" }, { "Berlin" }, { "Tel_Aviv" },
			{ "Tokyo" }, { "Roma" }, { "Madrid" }
		};

		for(const auto& c : cities) {
			ValueMap arg;
			arg.Add("id", c.id);
			arg.Add("x", 0.0);
			arg.Add("y", 0.0);
			history.Execute(ctx, dispatcher.Create("AddNode", arg));
		}
		history.Commit();

		// Style nodes
		for(const auto& c : cities) {
			NodeDoc* nd = graph.FindNode(c.id);
			if(nd) {
				// Replace underscores with spaces for display
				nd->label = c.id;
				nd->label.Replace("_", " ");
				nd->fill_clr = Color(0xff, 0xAA, 0x88);
				nd->sz = Sizef(90, 50);
			}
		}

		// Add weighted edges via commands
		history.Begin();
		auto AddWeightedEdge = [&](const String& eid, const String& from, const String& to, double weight) {
			ValueMap arg;
			arg.Add("id", eid);
			arg.Add("source_node", from);
			arg.Add("source_pin", "");
			arg.Add("target_node", to);
			arg.Add("target_pin", "");
			history.Execute(ctx, dispatcher.Create("AddEdge", arg));
			EdgeDoc* ed = graph.FindEdge(eid);
			if(ed) { ed->weight = weight; ed->directed = false; }
		};

		AddWeightedEdge("e1",  "Tokyo",    "Tel_Aviv", 5);
		AddWeightedEdge("e2",  "Tokyo",    "New_York", 8);
		AddWeightedEdge("e3",  "Tokyo",    "Berlin",   7);
		AddWeightedEdge("e4",  "Tel_Aviv", "Berlin",   3);
		AddWeightedEdge("e5",  "Tel_Aviv", "New_York", 6);
		AddWeightedEdge("e6",  "Tel_Aviv", "Roma",     2);
		AddWeightedEdge("e7",  "Roma",     "New_York", 5);
		AddWeightedEdge("e8",  "Berlin",   "New_York", 4);
		AddWeightedEdge("e9",  "Madrid",   "New_York", 7);
		AddWeightedEdge("e10", "Madrid",   "Roma",     3);
		AddWeightedEdge("e11", "Madrid",   "Tokyo",    10);
		history.Commit();

		// Apply spring layout for initial positioning
		Vector<NodeState> states;
		SpringLayout layout;
		layout.Iterations(500).Seed(42);
		layout.Run(graph, states);
		graph.ApplyLayout(states);

		// Run initial Dijkstra from Berlin
		RunDijkstra();
	}
};

GUI_APP_MAIN
{
	App().Run();
}
