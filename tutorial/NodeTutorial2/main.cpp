#include <Node/Ctrl/Ctrl.h>
#include <Node/Core/Migration.h>
#include <Node/Core/Layout.h>

using namespace Upp;
using namespace Upp::Node;

struct App : public TopWindow {
	Graph            graph;
	EditorState      editor;
	HistoryStack     history;
	CommandDispatcher dispatcher;
	NodeViewportCtrl  viewport;
	
	App() {
		Title("Node Tutorial 2: Layout Algorithms");
		Sizeable().Zoomable();
		
		dispatcher.RegisterStandardCommands();
		
		viewport.SetGraph(graph);
		viewport.SetEditor(editor);
		viewport.SetHistory(history);
		viewport.SetDispatcher(dispatcher);
		
		Add(viewport.SizePos());
		
		LegacyFacade facade(graph);
		EntityId n1 = facade.AddNode("A", Pointf(0, 0));
		EntityId n2 = facade.AddNode("B", Pointf(0, 0));
		EntityId n3 = facade.AddNode("C", Pointf(0, 0));
		facade.AddEdge(n1, n2);
		facade.AddEdge(n2, n3);
		facade.AddEdge(n3, n1);
		
		// Apply Spring Layout
		Vector<NodeState> states;
		SpringLayout layout;
		layout.Iterations(1000).Run(graph, states);
		
		// Map states back to doc (in a real app, this would be a command)
		for(int i = 0; i < graph.GetDoc().nodes.GetCount(); i++)
			graph.GetDoc().nodes[i].pos = states[i].layout_pos;
		
		graph.Invalidate();
	}
};

GUI_APP_MAIN
{
	App().Run();
}
