#include <Node/Script/Script.h>
#include <Node/Core/Layout.h>
#include <Node/Core/ForceLayout.h>

static bool HasVerboseFlag()
{
	for(const Upp::String& a : Upp::CommandLine())
		if(a == "-v" || a == "--verbose") return true;
	return false;
}

#ifdef flagGUI
#include <Node/Ctrl/Ctrl.h>
#endif

using namespace Upp;
using namespace Upp::Node;

// ---------------------------------------------------------------------------
// GUI path - Group layout and metadata patching
// ---------------------------------------------------------------------------

#ifdef flagGUI

// Autoencoder workflow: scripted layout from the neural-node-graph-reveng2.txt
// ## Layout coordinates section, with auto-packed nodes inside each group.
static void ApplyGroupLayout(Graph& graph)
{
	// Prescribed coordinates from the workflow description.
	// Format: [x, y, width, height] → converted to Rectf(x, y, x+w, y+h).
	ScriptedLayout sl;
	sl.NodePadding(20.0)
	  .GroupInnerPadding(25.0)
	  // Top-level nodes
	  .SetGroupRect("node104",  Rectf(200,  140, 200+1420, 140+2350))
	  // Groups
	  .SetGroupRect("/enc",     Rectf(1700, 100, 1700+4300, 100+1600))
	  .SetGroupRect("/dec",     Rectf(6100, 100, 6100+4600, 100+1600))
	  .SetGroupRect("/comp",    Rectf(2100, 1800, 2100+3000, 1800+1200))
	  .SetGroupRect("/data",    Rectf(1500, 3100, 1500+1600, 3100+1400))
	  .SetGroupRect("/train",   Rectf(3200, 3100, 3200+1700, 3100+1700))
	  .SetGroupRect("/test",    Rectf(4900, 3100, 4900+1600, 3100+1600))
	  .SetGroupRect("/testdata",Rectf(6700, 3100, 6700+1600, 3100+1600))
	  .SetGroupRect("/inf",     Rectf(8400, 1800, 8400+2400, 1800+2800))
	  .SetGroupRect("/prep",    Rectf(11000,1900, 11000+1700,1900+1600));

	sl.Run(graph);
}

// Apply tint colors based on group membership
static void PatchNodeMetadata(Graph& graph)
{
	for(NodeDoc& n : graph.GetDoc().nodes) {
		const String& id = n.id;
		if(id.StartsWith("enc_") || id.StartsWith("dec_"))
			n.tint_clr = Color(50, 180, 100);      // Green — encoder/decoder
		else if(id.StartsWith("train_"))
			n.tint_clr = Color(200, 50, 80);       // Red — training
		else if(id.StartsWith("test_"))
			n.tint_clr = Color(200, 80, 50);       // Orange-red — testing
		else if(id.StartsWith("testdata_"))
			n.tint_clr = Color(80, 130, 200);      // Blue — test data loading
		else if(id.StartsWith("data_"))
			n.tint_clr = Color(60, 110, 200);      // Blue — training data
		else if(id.StartsWith("comp_"))
			n.tint_clr = Color(180, 160, 50);      // Yellow — compile
		else if(id.StartsWith("inf_"))
			n.tint_clr = Color(140, 60, 200);      // Purple — inference
		else if(id.StartsWith("prep_"))
			n.tint_clr = Color(60, 180, 180);      // Teal — prep
	}
}

#else

// Console version: just dump group info (no GUI layout needed)
static void ApplyGroupLayout(Graph& graph) {
	for(const auto& g : graph.GetDoc().groups) {
		Cout() << "Group " << g.id << " has " << g.nodes.GetCount() << " nodes\n";
	}
}

static void PatchNodeMetadata(Graph& graph) {}

#endif

// ---------------------------------------------------------------------------
// Node type registry
// ---------------------------------------------------------------------------

static NodeDoc MakeNNLayer(String type_id, const char* label) {
	NodeDoc n; n.node_type_id = type_id; n.label = label;
	n.category = "NeuralNetworkToolkit"; n.fill_clr = Color(45, 50, 60); n.line_clr = Color(70, 80, 95);
	return n;
}

static NodeDoc MakeJjkNode(String type_id, const char* label) {
	NodeDoc n; n.node_type_id = type_id; n.label = label;
	n.category = "Jjk-Nodes"; n.fill_clr = Color(50, 45, 55); n.line_clr = Color(75, 65, 80);
	return n;
}

static NodeDoc MakeGenericNode(const char* label) {
	NodeDoc n; n.node_type_id = "generic"; n.label = label;
	n.fill_clr = Color(40, 44, 52); n.line_clr = Color(80, 90, 110);
	return n;
}

#ifdef flagGUI
static void RegisterAllNodeTypes(NodeViewportCtrl& viewport) {
	// NeuralNetworkToolkit nodes
	viewport.RegisterNodeType("generic_node", "Generic Node",
		[]{return MakeNNLayer("generic_node", "Generic Node");});
}

// ---------------------------------------------------------------------------

struct App : TopWindow {
	Graph             graph;
	EditorState       editor;
	HistoryStack      history;
	CommandDispatcher dispatcher;
	NodeViewportCtrl  viewport;

	App() {
		Title("Node Tutorial 6: Neural Network Autoencoder with Groups");
		Sizeable().Zoomable();

		dispatcher.RegisterStandardCommands();
		RegisterAllNodeTypes(viewport);
		viewport.SetGraph(graph);
		viewport.SetEditor(editor);
		viewport.SetHistory(history);
		viewport.SetDispatcher(dispatcher);
		Add(viewport.SizePos());

		String data_dir = GetDataFile("");
		if(!FileExists(AppendFileName(data_dir, "in.eon")))
			data_dir = GetFileDirectory(__FILE__);

		// Register the prescribed-coordinate autoencoder layout as a named entry.
		// It appears in the Layout Orientation menu; ApplyLayout() calls it.
		viewport.RegisterLayout("Autoencoder (Prescribed)", [this](Graph& g) {
			PatchNodeMetadata(g);
			ApplyGroupLayout(g);
		});

		Vector<ValidationMessage> errors;
		bool loaded = LoadEonFile(graph, AppendFileName(data_dir, "in.eon"), errors);
		if(loaded || graph.GetDoc().nodes.GetCount() > 0) {
			// Activate the custom layout so it's pre-selected in the menu
			viewport.SetActiveLayout("Autoencoder (Prescribed)");
			viewport.ApplyLayout();
		} else {
			for(const auto& e : errors)
				LOG("ERROR: " << e.message);
		}
	}
};

GUI_APP_MAIN {
	SetForceVerbose(HasVerboseFlag());
	App().Run();
}

#endif  // flagGUI

// ---------------------------------------------------------------------------
// Headless / CLI path
// ---------------------------------------------------------------------------

#ifndef flagGUI
CONSOLE_APP_MAIN {
	const Vector<String>& args = CommandLine();
	String path = GetDataFile("in.eon");
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] != "-v" && args[i] != "--verbose")
			path = args[i];
	}
	SetForceVerbose(HasVerboseFlag());

	Graph graph;
	Vector<ValidationMessage> errors;
	if(!LoadEonFile(graph, path, errors)) {
		for(const auto& e : errors) Cout() << "ERROR: " << e.message << "\n";
		SetExitCode(1); return;
	}

	const GraphDoc& doc = graph.GetDoc();
	Cout() << "Loaded: " << path << "\n";
	Cout() << "Nodes:  " << doc.nodes.GetCount() << "\n";
	Cout() << "Edges:  " << doc.edges.GetCount() << "\n";
	Cout() << "Groups: " << doc.groups.GetCount() << "\n\n";

	for(const auto& g : doc.groups) {
		Cout() << "Group: " << g.id << " (" << g.label << ") [path:" << g.vfs_path << "] nodes="
		       << g.nodes.GetCount() << " style(sat=" << g.style.saturation
		       << ",hue=" << g.style.hue << ",con=" << g.style.contrast << ")\n";
	}
	Cout() << "\n";

	ForceRefineGraph(graph, 600);

}
#endif  // !flagGUI
