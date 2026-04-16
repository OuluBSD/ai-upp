#include <Node/Script/Script.h>
#include <Node/Core/Layout.h>

#ifdef flagGUI
#include <Node/Ctrl/Ctrl.h>
#endif

using namespace Upp;
using namespace Upp::Node;

// ---------------------------------------------------------------------------
// GUI path
// ---------------------------------------------------------------------------

#ifdef flagGUI

// ComfyUI-style group column layout.
// Groups are laid out horizontally; nodes stack vertically within each group.
static void ApplyGroupLayout(Graph& graph)
{
	const double COL_W    = 250.0;  // world units between group columns
	const double V_GAP    = 30.0;   // gap between nodes in a column
	const double EST_H    = 220.0;  // estimated node height (before scene build)
	const double START_X  = 20.0;

	// Column assignment per group
	struct ColGroup : Moveable<ColGroup> {
		String         label;
		Vector<String> node_ids;
	};

	Vector<ColGroup> columns;
	{ ColGroup g; g.label = "Loaders";
	  g.node_ids = { "n0","n1","n2","n3" }; columns.Add(pick(g)); }
	{ ColGroup g; g.label = "LoRA";
	  g.node_ids = { "n4","n5" }; columns.Add(pick(g)); }
	{ ColGroup g; g.label = "Prompts";
	  g.node_ids = { "n6","n7" }; columns.Add(pick(g)); }
	{ ColGroup g; g.label = "Latent";
	  g.node_ids = { "n8" }; columns.Add(pick(g)); }
	{ ColGroup g; g.label = "Samplers";
	  g.node_ids = { "n9","n10" }; columns.Add(pick(g)); }
	{ ColGroup g; g.label = "Output";
	  g.node_ids = { "n11","n12","n13" }; columns.Add(pick(g)); }

	// Compute the tallest column so we can center shorter ones
	double max_col_h = 0;
	for(const ColGroup& col : columns) {
		int cnt = col.node_ids.GetCount();
		double col_h = cnt * EST_H + max(0, cnt - 1) * V_GAP;
		max_col_h = max(max_col_h, col_h);
	}

	for(int ci = 0; ci < columns.GetCount(); ci++) {
		const ColGroup& col = columns[ci];
		int cnt = col.node_ids.GetCount();
		double col_h = cnt * EST_H + max(0, cnt - 1) * V_GAP;
		double cx = START_X + ci * COL_W;
		// Vertically center this column within the tallest column
		double cy = (max_col_h - col_h) / 2.0;
		for(const String& nid : col.node_ids) {
			NodeDoc* n = graph.FindNode(nid);
			if(!n) continue;
			n->pos = Pointf(cx, cy);
			cy += EST_H + V_GAP;
			graph.Invalidate(nid);
		}
	}
}

// Set readable labels and tint colors on nodes
static void PatchNodeMetadata(Graph& graph)
{
	struct { const char* id; const char* label; Color tint; } patches[] = {
		{ "n0",  "UnetLoader GGUF (MultiGPU)",       Null                     },
		{ "n1",  "UnetLoader GGUF (MultiGPU)",       Null                     },
		{ "n2",  "CLIPLoader GGUF (MultiGPU)",        Null                     },
		{ "n3",  "Seed Everywhere",                   Null                     },
		{ "n4",  "Power Lora Loader",                 Null                     },
		{ "n5",  "Power Lora Loader",                 Null                     },
		{ "n6",  "Positive Prompt",                   Color( 50, 120, 120)     }, // cyan tint
		{ "n7",  "ConditioningZeroOut",               Null                     },
		{ "n8",  "Empty Hunyuan Latent",              Null                     },
		{ "n9",  "KSampler Advanced",                 Color( 80,  40, 120)     }, // purple tint
		{ "n10", "KSampler Advanced",                 Color( 80,  40, 120)     }, // purple tint
		{ "n11", "VAE Decode",                        Null                     },
		{ "n12", "VAELoader (MultiGPU)",              Null                     },
		{ "n13", "Video Combine",                     Null                     },
	};
	for(auto& p : patches) {
		NodeDoc* n = graph.FindNode(p.id);
		if(!n) continue;
		n->label = p.label;
		n->tint_clr = p.tint;
	}
}

// Add image slots to nodes that display output images
static void AddImageSlots(Graph& graph, const String& data_dir)
{
	struct { const char* node_id; const char* img_file; } images[] = {
		{ "n9",  "a.jpg" },
		{ "n10", "b.jpg" },
		{ "n13", "c.jpg" },
	};
	for(auto& im : images) {
		NodeDoc* n = graph.FindNode(im.node_id);
		if(!n) continue;
		WidgetSlotDoc& slot = n->slots.Add();
		slot.id   = "image_preview";
		slot.type = "IMAGE";
		slot.properties.Add("value", AppendFileName(data_dir, im.img_file));
		graph.Invalidate(im.node_id);
	}
}

struct App : TopWindow {
	Graph             graph;
	EditorState       editor;
	HistoryStack      history;
	CommandDispatcher dispatcher;
	NodeViewportCtrl  viewport;

	App() {
		Title("Node Tutorial 5: WAN 2.2 T2V Pipeline");
		Sizeable().Zoomable();

		dispatcher.RegisterStandardCommands();
		viewport.SetGraph(graph);
		viewport.SetEditor(editor);
		viewport.SetHistory(history);
		viewport.SetDispatcher(dispatcher);
		Add(viewport.SizePos());

		// GetDataFile uses UPP_MAIN__ (set by IDE) or falls back to exe dir.
		// When running the binary directly, fall back to the source directory.
		String data_dir = GetDataFile("");
		if(!FileExists(AppendFileName(data_dir, "in.eon")))
			data_dir = GetFileDirectory(__FILE__); // source-relative fallback
		Vector<ValidationMessage> errors;
		if (!LoadEonFile(graph, AppendFileName(data_dir, "in.eon"), errors)) {
			for (const auto& e : errors)
				LOG("ERROR: " << e.message);
		} else {
			PatchNodeMetadata(graph);
			AddImageSlots(graph, data_dir);
			ApplyGroupLayout(graph);
		}
		viewport.Refresh();
	}
};

GUI_APP_MAIN { App().Run(); }

// ---------------------------------------------------------------------------
// Headless / CLI path
// ---------------------------------------------------------------------------

#else

CONSOLE_APP_MAIN {
	const Vector<String>& args = CommandLine();

	String path = GetDataFile("in.eon");
	if (args.GetCount() >= 1)
		path = args[0];

	Graph graph;
	Vector<ValidationMessage> errors;
	if (!LoadEonFile(graph, path, errors)) {
		for (const auto& e : errors)
			Cout() << "ERROR: " << e.message << "\n";
		SetExitCode(1);
		return;
	}

	const GraphDoc& doc = graph.GetDoc();
	Cout() << "Loaded: " << path << "\n";
	Cout() << "Nodes:  " << doc.nodes.GetCount() << "\n";
	Cout() << "Edges:  " << doc.edges.GetCount() << "\n\n";

	for (const NodeDoc& n : doc.nodes) {
		Cout() << "  node " << n.id;
		if (!n.label.IsEmpty() && n.label != n.id)
			Cout() << " (" << n.label << ")";
		Cout() << "  pins=" << n.pins.GetCount()
		       << "  slots=" << n.slots.GetCount() << "\n";
	}
	Cout() << "\n";

	for (const EdgeDoc& e : doc.edges) {
		Cout() << "  " << e.source_node;
		if (!e.source_pin.IsEmpty()) Cout() << "." << e.source_pin;
		Cout() << " -> " << e.target_node;
		if (!e.target_pin.IsEmpty()) Cout() << "." << e.target_pin;
		Cout() << "\n";
	}

	if (args.GetCount() >= 2 && args[1] == "--save") {
		String out = SaveEon(graph);
		Cout() << "\n--- SaveEon output ---\n" << out;
	}
}

#endif
