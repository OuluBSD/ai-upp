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
// EST_H must be generous enough to cover the tallest nodes (KSampler with image).
static void ApplyGroupLayout(Graph& graph)
{
	const double COL_W   = 260.0;  // world units between group columns
	const double V_GAP   = 40.0;   // gap between nodes in a column
	const double EST_H   = 320.0;  // generous estimate; KSamplers are ~600 world units
	const double START_X = 20.0;

	struct ColGroup : Moveable<ColGroup> {
		String         label;
		Vector<String> node_ids;
		double         est_h; // per-node height estimate for this group
	};

	Vector<ColGroup> columns;
	{ ColGroup g; g.label="Loaders";      g.est_h=200; g.node_ids={"n0","n1","n2","n3"}; columns.Add(pick(g)); }
	{ ColGroup g; g.label="LoRA";         g.est_h=220; g.node_ids={"n4","n5"};           columns.Add(pick(g)); }
	{ ColGroup g; g.label="Conditioning"; g.est_h=200; g.node_ids={"n6","n7","n8"};      columns.Add(pick(g)); }
	{ ColGroup g; g.label="Samplers";     g.est_h=600; g.node_ids={"n9","n10"};          columns.Add(pick(g)); }
	{ ColGroup g; g.label="Output";       g.est_h=200; g.node_ids={"n11","n12","n13"};   columns.Add(pick(g)); }

	// Tallest column height determines the centering baseline
	double max_col_h = 0;
	for(const ColGroup& col : columns) {
		int cnt = col.node_ids.GetCount();
		double col_h = cnt * col.est_h + max(0, cnt - 1) * V_GAP;
		max_col_h = max(max_col_h, col_h);
	}

	for(int ci = 0; ci < columns.GetCount(); ci++) {
		const ColGroup& col = columns[ci];
		int cnt = col.node_ids.GetCount();
		double col_h = cnt * col.est_h + max(0, cnt - 1) * V_GAP;
		double cx = START_X + ci * COL_W;
		double cy = (max_col_h - col_h) / 2.0;
		for(const String& nid : col.node_ids) {
			NodeDoc* n = graph.FindNode(nid);
			if(!n) continue;
			n->pos = Pointf(cx, cy);
			cy += col.est_h + V_GAP;
			graph.Invalidate(nid);
		}
	}
}

// Patch labels, tints, time strings and image paths that are not in the EON file.
static void PatchNodeMetadata(Graph& graph, const String& data_dir)
{
	struct Meta {
		const char* id;
		const char* label;
		Color       tint;
		const char* time;     // e.g. "519.4s" — shown as floating badge above node
		const char* img_file; // if non-null, add IMAGE slot with this file
	};
	static const Meta patches[] = {
		{ "n0",  "UnetLoader GGUF (MultiGPU)",  Null,                    nullptr,       nullptr  },
		{ "n1",  "UnetLoader GGUF (MultiGPU)",  Null,                    nullptr,       nullptr  },
		{ "n2",  "CLIPLoader GGUF (MultiGPU)",  Null,                    nullptr,       nullptr  },
		{ "n3",  "Seed Everywhere",             Null,                    "0.002s",      nullptr  },
		{ "n4",  "Power Lora Loader",           Null,                    nullptr,       nullptr  },
		{ "n5",  "Power Lora Loader",           Null,                    nullptr,       nullptr  },
		{ "n6",  "Positive Prompt",             Color( 50, 120, 120),    nullptr,       nullptr  },
		{ "n7",  "ConditioningZeroOut",         Null,                    nullptr,       nullptr  },
		{ "n8",  "Empty Hunyuan Latent",        Null,                    "0.004s",      nullptr  },
		{ "n9",  "KSampler Advanced",           Color( 80,  40, 120),    "519.4s",      "a.jpg"  },
		{ "n10", "KSampler Advanced",           Color( 80,  40, 120),    "534.9s",      "b.jpg"  },
		{ "n11", "VAE Decode",                  Null,                    "10.4s",       nullptr  },
		{ "n12", "VAELoader (MultiGPU)",        Null,                    nullptr,       nullptr  },
		{ "n13", "Video Combine",               Null,                    "16.8s",       "c.jpg"  },
	};

	for(const auto& p : patches) {
		NodeDoc* n = graph.FindNode(p.id);
		if(!n) continue;
		n->label    = p.label;
		n->tint_clr = p.tint;
		if(p.time)
			n->time_str = p.time;
		if(p.img_file) {
			WidgetSlotDoc& slot = n->slots.Add();
			slot.id   = "image_preview";
			slot.type = "IMAGE";
			slot.properties.Add("value", AppendFileName(data_dir, p.img_file));
			graph.Invalidate(p.id);
		}
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
			data_dir = GetFileDirectory(__FILE__);
		Vector<ValidationMessage> errors;
		if(!LoadEonFile(graph, AppendFileName(data_dir, "in.eon"), errors)) {
			for(const auto& e : errors)
				LOG("ERROR: " << e.message);
		} else {
			PatchNodeMetadata(graph, data_dir);
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
	if(args.GetCount() >= 1)
		path = args[0];

	Graph graph;
	Vector<ValidationMessage> errors;
	if(!LoadEonFile(graph, path, errors)) {
		for(const auto& e : errors)
			Cout() << "ERROR: " << e.message << "\n";
		SetExitCode(1);
		return;
	}

	const GraphDoc& doc = graph.GetDoc();
	Cout() << "Loaded: " << path << "\n";
	Cout() << "Nodes:  " << doc.nodes.GetCount() << "\n";
	Cout() << "Edges:  " << doc.edges.GetCount() << "\n\n";

	for(const NodeDoc& n : doc.nodes) {
		Cout() << "  node " << n.id;
		if(!n.label.IsEmpty() && n.label != n.id)
			Cout() << " (" << n.label << ")";
		Cout() << "  pins=" << n.pins.GetCount()
		       << "  slots=" << n.slots.GetCount() << "\n";
	}
	Cout() << "\n";

	for(const EdgeDoc& e : doc.edges) {
		Cout() << "  " << e.source_node;
		if(!e.source_pin.IsEmpty()) Cout() << "." << e.source_pin;
		Cout() << " -> " << e.target_node;
		if(!e.target_pin.IsEmpty()) Cout() << "." << e.target_pin;
		Cout() << "\n";
	}

	if(args.GetCount() >= 2 && args[1] == "--save") {
		String out = SaveEon(graph);
		Cout() << "\n--- SaveEon output ---\n" << out;
	}
}

#endif
