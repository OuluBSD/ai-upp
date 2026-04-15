#include <Node/Ctrl/Ctrl.h>
#include <Node/Core/Migration.h>

using namespace Upp;
using namespace Upp::Node;

struct App : public TopWindow {
	Graph            graph;
	EditorState      editor;
	HistoryStack     history;
	CommandDispatcher dispatcher;
	NodeViewportCtrl  viewport;
	
	App() {
		Title("Node Tutorial 1: Basic Graph");
		Sizeable().Zoomable();
		
		dispatcher.RegisterStandardCommands();
		
		viewport.SetGraph(graph);
		viewport.SetEditor(editor);
		viewport.SetHistory(history);
		viewport.SetDispatcher(dispatcher);
		
		Add(viewport.SizePos());
		
		LegacyFacade facade(graph);
		EntityId n1 = facade.AddNode("Hello", Pointf(50, 50));
		EntityId n2 = facade.AddNode("World", Pointf(200, 150));
		facade.AddEdge(n1, n2);
	}
};

GUI_APP_MAIN
{
	App().Run();
}
