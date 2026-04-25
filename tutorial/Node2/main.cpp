#include <Node/Ctrl/Ctrl.h>
#include <Node/Core/Algo.h>

using namespace Upp;
using namespace Upp::Node;

// Node2: Ordered dependency graph (CS prerequisites)
// Demonstrates: command-based construction, ordered tree layout, selection commands

struct App : public TopWindow {
	Graph            graph;
	EditorState      editor;
	HistoryStack     history;
	CommandDispatcher dispatcher;
	NodeViewportCtrl  viewport;

	App() {
		Title("Node Tutorial 2: CS Prerequisites");
		Sizeable().Zoomable();

		dispatcher.RegisterStandardCommands();

		viewport.SetGraph(graph);
		viewport.SetEditor(editor);
		viewport.SetHistory(history);
		viewport.SetDispatcher(dispatcher);

		Add(viewport.SizePos());

		CommandContext ctx(graph, editor);

		// CS class IDs must be valid entity IDs (no spaces). Labels carry the display text.
		struct Course { String id; String label; };
		Course courses[] = {
			{ "CS_31",   "CS 31"   },
			{ "CS_32",   "CS 32"   },
			{ "CS_33",   "CS 33"   },
			{ "CS_35L",  "CS 35L"  },
			{ "CS_M51A", "CS M51A" },
			{ "CS_111",  "CS 111"  },
			{ "CS_118",  "CS 118"  },
			{ "CS_131",  "CS 131"  },
		};

		Color default_fill = Color(0xff, 0xAA, 0x88);

		// Add all nodes via commands (transactional)
		history.Begin();
		for(const auto& c : courses) {
			ValueMap arg;
			arg.Add("id", c.id);
			arg.Add("x", 0.0);
			arg.Add("y", 0.0);
			history.Execute(ctx, dispatcher.Create("AddNode", arg));
		}
		history.Commit();

		// Set labels and styling via doc (non-structural, no command needed)
		for(const auto& c : courses) {
			NodeDoc* nd = graph.FindNode(c.id);
			if(nd) { nd->label = c.label; nd->fill_clr = default_fill; nd->sz = Sizef(80, 40); }
		}

		// Add dependency edges via commands (prerequisite → dependent)
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

		AddEdge("e1",  "CS_31",  "CS_32");
		AddEdge("e2",  "CS_32",  "CS_33");
		AddEdge("e3",  "CS_33",  "CS_35L");
		// CS_M51A is standalone — no edges
		AddEdge("e4",  "CS_32",  "CS_111");
		AddEdge("e5",  "CS_33",  "CS_111");
		AddEdge("e6",  "CS_35L", "CS_111");
		AddEdge("e7",  "CS_32",  "CS_118");
		AddEdge("e8",  "CS_33",  "CS_118");
		AddEdge("e9",  "CS_35L", "CS_118");
		AddEdge("e10", "CS_111", "CS_118");
		AddEdge("e11", "CS_32",  "CS_131");
		AddEdge("e12", "CS_33",  "CS_131");
		AddEdge("e13", "CS_35L", "CS_131");
		history.Commit();

		// Demonstrate selection: pre-select the root course
		{
			ValueMap arg;
			arg.Add("id", "CS_31");
			arg.Add("exclusive", true);
			history.Execute(ctx, dispatcher.Create("Select", arg));
		}

		// Apply ordered tree layout (prerequisite flow)
		Vector<NodeState> states;
		OrderedTreeLayout(graph, states);
		graph.ApplyLayout(states);
	}
};

GUI_APP_MAIN
{
	App().Run();
}
