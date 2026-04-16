#include <Node/Script/Script.h>
#include <Node/Core/Layout.h>

#ifdef flagGUI
#include <Node/Ctrl/Ctrl.h>
#endif

using namespace Upp;
using namespace Upp::Node;

// ---------------------------------------------------------------------------
// GUI path - Group layout and metadata patching
// ---------------------------------------------------------------------------

#ifdef flagGUI

// Autoencoder workflow: groups are horizontally arranged
static void ApplyGroupLayout(Graph& graph)
{
	const double COL_W   = 340.0;  // Width between group columns (includes padding)
	const double V_GAP   = 40.0;   // Gap between nodes in a column
	const double START_X = 30.0;   // Starting X position
	const double START_Y = 50.0;   // Starting Y position (for title bar space)

	// Read group definitions from the graph and arrange them horizontally
	// Order matches the workflow: encoder, decoder, compile, data, train, test, inference, prep
	Vector<String> group_order = {"enc", "dec", "comp", "data", "train", "test", "inf", "prep"};

	// Position nodes in their respective group columns
	for(int ci = 0; ci < group_order.GetCount(); ci++) {
		String grp_id = group_order[ci];
		double cx = START_X + ci * COL_W;
		double cy = START_Y;
		
		// Try to get nodes from group definition first
		const GroupDoc* grp = graph.FindGroup(grp_id);
		int count = 0;
		if(grp) {
			LOG("Positioning group " << grp_id << ": ");
			for(const String& nid : grp->nodes) {
				NodeDoc* n = graph.FindNode(nid);
				if(!n) continue;
				n->pos = Pointf(cx, cy);
				cy += 75.0 + V_GAP;  // Node height (~75) + gap
				graph.Invalidate(nid);
				LOG(nid << "(" << n->pos << ") ");
				count++;
			}
		} else {
			// Fallback: find nodes by prefix matching
			String prefix = grp_id + "_";
			for(NodeDoc& n : graph.GetDoc().nodes) {
				if(n.id.StartsWith(prefix)) {
					n.pos = Pointf(cx, cy);
					cy += 75.0 + V_GAP;
					graph.Invalidate(n.id);
					count++;
				}
			}
		}
		LOG("(total: " << count << ")\n");
	}
}

// Patch labels and tints for demo visual appeal
static void PatchNodeMetadata(Graph& graph)
{
	// Auto-generate labels based on node IDs if they don't have custom ones
	for(NodeDoc& n : graph.GetDoc().nodes) {
		if(n.label.IsEmpty() || n.label == n.id) {
			// Generate a readable label from the ID
			String lbl = n.id;
			lbl.Replace("_", " ");
			n.label = lbl;
		}

		// Set tint colors based on group membership
		const String id = n.id;
		if(id.StartsWith("enc_"))
			n.tint_clr = Color(50, 180, 100);    // Greenish (encoder)
		else if(id.StartsWith("dec_"))
			n.tint_clr = Color(50, 180, 100);    // Greenish (decoder)
		else if(id.StartsWith("train_"))
			n.tint_clr = Color(200, 50, 80);     // Reddish (training)
		else if(id.StartsWith("prep_"))
			n.tint_clr = Color(80, 120, 200);    // Blueish (preparation)
	}
}

#else

// Console version stubs
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
		
		Vector<ValidationMessage> errors;
		if(LoadEonFile(graph, AppendFileName(data_dir, "in.eon"), errors)) {
			PatchNodeMetadata(graph);
			ApplyGroupLayout(graph);
		} else {
			for(const auto& e : errors)
				LOG("ERROR: " << e.message);
		}
		viewport.Refresh();
	}
};

GUI_APP_MAIN { App().Run(); }

#endif  // flagGUI

// ---------------------------------------------------------------------------
// Headless / CLI path
// ---------------------------------------------------------------------------

#ifndef flagGUI
CONSOLE_APP_MAIN {
	const Vector<String>& args = CommandLine();
	String path = GetDataFile("in.eon");
	if(args.GetCount() >= 1) path = args[0];

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

	// Also run layout for console to verify group assignments
	ApplyGroupLayout(graph);
}
#endif  // !flagGUI
