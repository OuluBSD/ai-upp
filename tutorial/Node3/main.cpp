#include <Node/Ctrl/Ctrl.h>
#include <Node/Core/Layout.h>

using namespace Upp;
using namespace Upp::Node;

// Node3: Spring layout + styling + directed edges
// Demonstrates: node shapes/colors, directed/labeled edges, spring layout, command-based construction

struct App : public TopWindow {
	Graph            graph;
	EditorState      editor;
	HistoryStack     history;
	CommandDispatcher dispatcher;
	NodeViewportCtrl  viewport;

	App() {
		Title("Node Tutorial 3: Spring Layout & Styling");
		Sizeable().Zoomable();

		dispatcher.RegisterStandardCommands();

		viewport.SetGraph(graph);
		viewport.SetEditor(editor);
		viewport.SetHistory(history);
		viewport.SetDispatcher(dispatcher);

		Add(viewport.SizePos());

		CommandContext ctx(graph, editor);

		Color default_fill = Color(0xff, 0xAA, 0x88);

		// Add nodes via commands
		history.Begin();
		auto AddNode = [&](const String& id, const String& label, int shape, Color fill, Sizef sz) {
			ValueMap arg;
			arg.Add("id", id);
			arg.Add("x", 0.0);
			arg.Add("y", 0.0);
			history.Execute(ctx, dispatcher.Create("AddNode", arg));
			NodeDoc* nd = graph.FindNode(id);
			if(nd) { nd->label = label; nd->shape = shape; nd->fill_clr = fill; nd->sz = sz; }
		};

		AddNode("strawberry", "strawberry", 0, default_fill, Sizef(80, 40));
		AddNode("cherry",     "cherry",     0, default_fill, Sizef(80, 40));
		AddNode("tomato",     "Tomato",     2, default_fill, Sizef(100, 50)); // Diamond shape
		AddNode("meat",       "meat and greed", 0, default_fill, Sizef(120, 50)); // Rectangle
		AddNode("kiwi",       "kiwi",       0, default_fill, Sizef(80, 40));
		AddNode("penguin",    "penguin",    0, default_fill, Sizef(80, 40));
		AddNode("apple",      "apple",      0, Color(0x55, 0x66, 0xff), Sizef(80, 40)); // Blue
		history.Commit();

		// Add edges via commands with styling
		history.Begin();
		auto AddEdge = [&](const String& eid, const String& src, const String& tgt,
		                   bool directed = false, const String& label = "",
		                   int lw = 1, Color stroke = Black()) {
			ValueMap arg;
			arg.Add("id", eid);
			arg.Add("source_node", src);
			arg.Add("source_pin", "");
			arg.Add("target_node", tgt);
			arg.Add("target_pin", "");
			history.Execute(ctx, dispatcher.Create("AddEdge", arg));
			EdgeDoc* ed = graph.FindEdge(eid);
			if(ed) { ed->directed = directed; ed->label = label; ed->line_width = lw; ed->stroke_clr = stroke; }
		};

		AddEdge("e_kiwi_penguin", "kiwi",    "penguin",  true, "Label");
		AddEdge("e_str_cherry",   "strawberry", "cherry", true);
		AddEdge("e_cherry_apple", "cherry",  "apple");
		AddEdge("e_1_id35",       "tomato",  "meat");
		AddEdge("e_penguin_id35", "penguin", "meat");
		AddEdge("e_penguin_apple","penguin", "apple");
		AddEdge("e_kiwi_id35",    "kiwi",    "meat");
		AddEdge("e_1_cherry",     "tomato",  "cherry",  true);
		AddEdge("e_id35_apple",   "meat",    "apple",   false, "Meat-to-apple", 2, Color(0xbb, 0xff, 0xaa));
		AddEdge("e_str_apple",    "strawberry", "apple");
		history.Commit();

		// Apply spring layout
		Vector<NodeState> states;
		SpringLayout layout;
		layout.Iterations(500).Seed(42);
		layout.Run(graph, states);
		graph.ApplyLayout(states);
	}
};

GUI_APP_MAIN
{
	App().Run();
}
