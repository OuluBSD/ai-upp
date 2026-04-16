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

// ---------------------------------------------------------------------------
// Node type registry helpers
// ---------------------------------------------------------------------------

// Build a NodeDoc template from a type id and its pin/slot declarations.
// Mirrors the node types declared in in.eon so "Add Node" can create them.

static NodeDoc MakeUnetLoader()
{
	NodeDoc n; n.node_type_id = "comfyui.gguf.unet_loader"; n.label = "UnetLoader GGUF";
	n.category = "comfyui"; n.fill_clr = Color(40, 44, 52); n.line_clr = Color(80, 90, 110);
	{ auto& p = n.pins.Add(); p.id = "model"; p.kind = PinKind::Output; p.label = "MODEL"; p.color = Color(160, 80, 200); }
	auto AddSlot = [&](const char* id, const char* type, Value val) {
		auto& s = n.slots.Add(); s.id = id; s.type = type; s.properties.Add("value", val);
	};
	AddSlot("unet_file",               "DropList",      ""); AddSlot("device", "DropList", "cuda:0");
	AddSlot("virtual_vram_gb",         "EditDoubleSpin", 8.0); AddSlot("use_other_vram", "Option", false);
	AddSlot("expert_mode_allocations", "Null",           Value());
	return n;
}

static NodeDoc MakeClipLoader()
{
	NodeDoc n; n.node_type_id = "comfyui.gguf.clip_loader"; n.label = "CLIPLoader GGUF";
	n.category = "comfyui"; n.fill_clr = Color(40, 44, 52); n.line_clr = Color(80, 90, 110);
	{ auto& p = n.pins.Add(); p.id = "clip"; p.kind = PinKind::Output; p.label = "CLIP"; p.color = Color(200, 180, 60); }
	auto AddSlot = [&](const char* id, const char* type, Value val) {
		auto& s = n.slots.Add(); s.id = id; s.type = type; s.properties.Add("value", val);
	};
	AddSlot("clip_name", "DropList", ""); AddSlot("type", "DropList", "wan");
	AddSlot("device", "DropList", "cpu"); AddSlot("virtual_vram_gb", "EditDoubleSpin", 8.0);
	AddSlot("use_other_vram", "Option", false); AddSlot("expert_mode_allocations", "Null", Value());
	return n;
}

static NodeDoc MakeSeedEverywhere()
{
	NodeDoc n; n.node_type_id = "comfyui.seed_everywhere"; n.label = "Seed Everywhere";
	n.category = "comfyui"; n.fill_clr = Color(40, 44, 52); n.line_clr = Color(80, 90, 110);
	{ auto& p = n.pins.Add(); p.id = "seed"; p.kind = PinKind::Output; p.label = "INT"; p.color = Color(180, 180, 180); }
	auto AddSlot = [&](const char* id, const char* type, Value val) {
		auto& s = n.slots.Add(); s.id = id; s.type = type; s.properties.Add("value", val);
	};
	AddSlot("seed", "EditIntSpin", 0); AddSlot("control_after_generate", "DropList", "randomize");
	return n;
}

static NodeDoc MakePowerLoraLoader()
{
	NodeDoc n; n.node_type_id = "comfyui.power_lora_loader"; n.label = "Power Lora Loader";
	n.category = "comfyui"; n.fill_clr = Color(40, 44, 52); n.line_clr = Color(80, 90, 110);
	{ auto& p = n.pins.Add(); p.id = "model"; p.kind = PinKind::Input;  p.label = "MODEL"; p.color = Color(160, 80, 200); }
	{ auto& p = n.pins.Add(); p.id = "clip";  p.kind = PinKind::Input;  p.label = "CLIP";  p.color = Color(200, 180, 60); }
	{ auto& p = n.pins.Add(); p.id = "model"; p.kind = PinKind::Output; p.label = "MODEL"; p.color = Color(160, 80, 200); }
	{ auto& p = n.pins.Add(); p.id = "clip";  p.kind = PinKind::Output; p.label = "CLIP";  p.color = Color(200, 180, 60); }
	auto AddSlot = [&](const char* id, const char* type, Value val) {
		auto& s = n.slots.Add(); s.id = id; s.type = type; s.properties.Add("value", val);
	};
	AddSlot("toggle_all", "ToggleButton", true);
	AddSlot("lora_0", "ToggleEditDouble", 0.80); AddSlot("lora_1", "ToggleEditDouble", 0.60);
	AddSlot("lora_2", "ToggleEditDouble", 1.50); AddSlot("add_lora", "Button", Value());
	return n;
}

static NodeDoc MakePositivePrompt()
{
	NodeDoc n; n.node_type_id = "comfyui.positive_prompt"; n.label = "Positive Prompt";
	n.category = "comfyui"; n.fill_clr = Color(40, 44, 52); n.tint_clr = Color(50, 120, 120); n.line_clr = Color(80, 90, 110);
	{ auto& p = n.pins.Add(); p.id = "clip";         p.kind = PinKind::Input;  p.label = "CLIP";         p.color = Color(200, 180, 60); }
	{ auto& p = n.pins.Add(); p.id = "conditioning"; p.kind = PinKind::Output; p.label = "CONDITIONING"; p.color = Color(220, 130, 60); }
	{ auto& s = n.slots.Add(); s.id = "text"; s.type = "DocEdit"; s.properties.Add("value", ""); }
	return n;
}

static NodeDoc MakeConditioningZeroOut()
{
	NodeDoc n; n.node_type_id = "comfyui.conditioning_zero_out"; n.label = "ConditioningZeroOut";
	n.category = "comfyui"; n.fill_clr = Color(40, 44, 52); n.line_clr = Color(80, 90, 110);
	{ auto& p = n.pins.Add(); p.id = "conditioning"; p.kind = PinKind::Input;  p.label = "CONDITIONING"; p.color = Color(220, 130, 60); }
	{ auto& p = n.pins.Add(); p.id = "conditioning"; p.kind = PinKind::Output; p.label = "CONDITIONING"; p.color = Color(220, 130, 60); }
	return n;
}

static NodeDoc MakeEmptyHunyuanLatent()
{
	NodeDoc n; n.node_type_id = "comfyui.empty_hunyuan_latent"; n.label = "Empty Hunyuan Latent";
	n.category = "comfyui"; n.fill_clr = Color(40, 44, 52); n.line_clr = Color(80, 90, 110);
	{ auto& p = n.pins.Add(); p.id = "latent"; p.kind = PinKind::Output; p.label = "LATENT"; p.color = Color(220, 100, 100); }
	auto AddSlot = [&](const char* id, Value val) {
		auto& s = n.slots.Add(); s.id = id; s.type = "EditIntSpin"; s.properties.Add("value", val);
	};
	AddSlot("width", 480); AddSlot("height", 832); AddSlot("length", 81); AddSlot("batch_size", 1);
	return n;
}

static NodeDoc MakeKSamplerAdvanced()
{
	NodeDoc n; n.node_type_id = "comfyui.ksampler_advanced"; n.label = "KSampler Advanced";
	n.category = "comfyui"; n.fill_clr = Color(40, 44, 52); n.tint_clr = Color(80, 40, 120); n.line_clr = Color(80, 90, 110);
	for(const char* id : (const char*[]){"model", "positive", "negative", "latent_image"}) {
		auto& p = n.pins.Add(); p.id = id; p.kind = PinKind::Input;  p.label = ToUpper(String(id)); p.color = Color(180, 180, 180);
	}
	{ auto& p = n.pins.Add(); p.id = "latent"; p.kind = PinKind::Output; p.label = "LATENT"; p.color = Color(220, 100, 100); }
	auto AddSlot = [&](const char* id, const char* type, Value val) {
		auto& s = n.slots.Add(); s.id = id; s.type = type; s.properties.Add("value", val);
	};
	AddSlot("add_noise",              "Option",       true);
	AddSlot("noise_seed",             "EditIntSpin",  1234567890);
	AddSlot("control_after_generate", "DropList",     "fixed");
	AddSlot("steps",                  "EditIntSpin",  16);
	AddSlot("cfg",                    "EditDoubleSpin", 1.0);
	AddSlot("sampler_name",           "DropList",     "res_2s");
	AddSlot("scheduler",              "DropList",     "bong_tangent");
	AddSlot("start_at_step",          "EditIntSpin",  0);
	AddSlot("end_at_step",            "EditIntSpin",  16);
	AddSlot("return_with_leftover_noise", "Option",   false);
	return n;
}

static NodeDoc MakeVaeDecode()
{
	NodeDoc n; n.node_type_id = "comfyui.vae_decode"; n.label = "VAE Decode";
	n.category = "comfyui"; n.fill_clr = Color(40, 44, 52); n.line_clr = Color(80, 90, 110);
	{ auto& p = n.pins.Add(); p.id = "samples"; p.kind = PinKind::Input;  p.label = "LATENT"; p.color = Color(220, 100, 100); }
	{ auto& p = n.pins.Add(); p.id = "vae";     p.kind = PinKind::Input;  p.label = "VAE";    p.color = Color(100, 200, 120); }
	{ auto& p = n.pins.Add(); p.id = "image";   p.kind = PinKind::Output; p.label = "IMAGE";  p.color = Color(100, 160, 220); }
	return n;
}

static NodeDoc MakeVaeLoader()
{
	NodeDoc n; n.node_type_id = "comfyui.vae_loader"; n.label = "VAELoader";
	n.category = "comfyui"; n.fill_clr = Color(40, 44, 52); n.line_clr = Color(80, 90, 110);
	{ auto& p = n.pins.Add(); p.id = "vae"; p.kind = PinKind::Output; p.label = "VAE"; p.color = Color(100, 200, 120); }
	auto AddSlot = [&](const char* id, const char* type, Value val) {
		auto& s = n.slots.Add(); s.id = id; s.type = type; s.properties.Add("value", val);
	};
	AddSlot("vae_name", "EditField", ""); AddSlot("device", "DropList", "cpu");
	return n;
}

static NodeDoc MakeVideoCombine()
{
	NodeDoc n; n.node_type_id = "comfyui.video_combine"; n.label = "Video Combine";
	n.category = "comfyui"; n.fill_clr = Color(40, 44, 52); n.line_clr = Color(80, 90, 110);
	{ auto& p = n.pins.Add(); p.id = "image_batch"; p.kind = PinKind::Input;  p.label = "IMAGE"; p.color = Color(100, 160, 220); }
	{ auto& p = n.pins.Add(); p.id = "video";       p.kind = PinKind::Output; p.label = "VIDEO"; p.color = Color(60, 180, 180); }
	auto AddSlot = [&](const char* id, const char* type, Value val) {
		auto& s = n.slots.Add(); s.id = id; s.type = type; s.properties.Add("value", val);
	};
	AddSlot("frame_rate",    "EditIntSpin",  24);
	AddSlot("loop_count",    "EditIntSpin",  0);
	AddSlot("filename_path", "EditField",    "");
	AddSlot("format",        "EditField",    "video/webm");
	AddSlot("pingpong",      "Option",       false);
	AddSlot("save_image",    "Option",       true);
	return n;
}

static void RegisterAllNodeTypes(NodeViewportCtrl& viewport)
{
	viewport.RegisterNodeType("comfyui.gguf.unet_loader",     "UnetLoader GGUF",       MakeUnetLoader);
	viewport.RegisterNodeType("comfyui.gguf.clip_loader",     "CLIPLoader GGUF",       MakeClipLoader);
	viewport.RegisterNodeType("comfyui.seed_everywhere",      "Seed Everywhere",       MakeSeedEverywhere);
	viewport.RegisterNodeType("comfyui.power_lora_loader",    "Power Lora Loader",     MakePowerLoraLoader);
	viewport.RegisterNodeType("comfyui.positive_prompt",      "Positive Prompt",       MakePositivePrompt);
	viewport.RegisterNodeType("comfyui.conditioning_zero_out","ConditioningZeroOut",   MakeConditioningZeroOut);
	viewport.RegisterNodeType("comfyui.empty_hunyuan_latent", "Empty Hunyuan Latent",  MakeEmptyHunyuanLatent);
	viewport.RegisterNodeType("comfyui.ksampler_advanced",    "KSampler Advanced",     MakeKSamplerAdvanced);
	viewport.RegisterNodeType("comfyui.vae_decode",           "VAE Decode",            MakeVaeDecode);
	viewport.RegisterNodeType("comfyui.vae_loader",           "VAELoader",             MakeVaeLoader);
	viewport.RegisterNodeType("comfyui.video_combine",        "Video Combine",         MakeVideoCombine);
}

// ---------------------------------------------------------------------------

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
		RegisterAllNodeTypes(viewport);
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
