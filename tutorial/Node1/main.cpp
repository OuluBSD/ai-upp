#include <Node/Ctrl/Ctrl.h>
#include <Node/Core/Algo.h>

using namespace Upp;
using namespace Upp::Node;

// Node1: Tournament-tree hierarchical graph
// Demonstrates: command-based graph construction, tournament tree layout

struct App : public TopWindow {
	Graph            graph;
	EditorState      editor;
	HistoryStack     history;
	CommandDispatcher dispatcher;
	NodeViewportCtrl  viewport;

	App() {
		Title("Node Tutorial 1: Tournament Tree");
		Sizeable().Zoomable();

		dispatcher.RegisterStandardCommands();

		viewport.SetGraph(graph);
		viewport.SetEditor(editor);
		viewport.SetHistory(history);
		viewport.SetDispatcher(dispatcher);

		Add(viewport.SizePos());

		CommandContext ctx(graph, editor);

		// Build tournament tree via commands (not raw graph API)
		//          1
		//        /   \
		//       2     3
		//      / \   / \
		//     4   5 6   7
		//    /\  /\ /\  /\
		//   8  9 10 11 12 13 14 15

		history.Begin();
		for(int i = 1; i <= 15; i++) {
			EntityId id = "n" + AsString(i);
			ValueMap arg;
			arg.Add("id", id);
			arg.Add("x", 0.0);
			arg.Add("y", 0.0);
			history.Execute(ctx, dispatcher.Create("AddNode", arg));
		}
		history.Commit();

		// Apply labels and colors via direct doc access (layout styling, not structural)
		for(int i = 1; i <= 7; i++) {
			NodeDoc* nd = graph.FindNode("n" + AsString(i));
			if(nd) { nd->label = AsString(i); nd->fill_clr = Color(0xff, 0xAA, 0x88); nd->sz = Sizef(60, 40); }
		}
		for(int i = 8; i <= 15; i++) {
			NodeDoc* nd = graph.FindNode("n" + AsString(i));
			if(nd) { nd->label = AsString(i); nd->fill_clr = Color(0xAA, 0xff, 0x88); nd->sz = Sizef(60, 40); }
		}

		// Add edges via commands
		history.Begin();
		auto AddEdge = [&](const String& eid, const String& src, const String& tgt) {
			ValueMap arg;
			arg.Add("id", eid);
			arg.Add("source_node", src);
			arg.Add("source_pin", "");
			arg.Add("target_node", tgt);
			arg.Add("target_pin", "");
			history.Execute(ctx, dispatcher.Create("AddEdge", arg));
		};

		AddEdge("e_2_1",  "n2",  "n1");  AddEdge("e_3_1",  "n3",  "n1");
		AddEdge("e_4_2",  "n4",  "n2");  AddEdge("e_5_2",  "n5",  "n2");
		AddEdge("e_6_3",  "n6",  "n3");  AddEdge("e_7_3",  "n7",  "n3");
		AddEdge("e_8_4",  "n8",  "n4");  AddEdge("e_9_4",  "n9",  "n4");
		AddEdge("e_10_5", "n10", "n5");  AddEdge("e_11_5", "n11", "n5");
		AddEdge("e_12_6", "n12", "n6");  AddEdge("e_13_6", "n13", "n6");
		AddEdge("e_14_7", "n14", "n7");  AddEdge("e_15_7", "n15", "n7");
		history.Commit();

		// Execute tournament tree layout
		Vector<NodeState> states;
		TournamentTreeLayout(graph, states);
		graph.ApplyLayout(states);
	}
};

GUI_APP_MAIN
{
	App().Run();
}
