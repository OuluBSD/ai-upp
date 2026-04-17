#include "Scene.h"
#include "Routing.h"

namespace Upp {

namespace Node {

void SpatialIndex::Build(const Vector<SceneItem>& items)
{
	Clear();
	if(items.IsEmpty()) return;
	
	bounds = Rectf(items[0].rect);
	for(const auto& item : items) {
		if(item.type == SceneItem::EDGE) {
			for(const auto& pt : item.path) bounds.Union(pt);
		}
		else {
			bounds.Union(item.rect);
		}
	}
	
	// Adaptive grid: ~4 items per cell on average, clamped to [4,64] per axis
	int n = items.GetCount();
	int g = max(4, min(64, (int)sqrt((double)n / 4.0)));
	double aspect = bounds.Height() > 1e-6 ? bounds.Width() / bounds.Height() : 1.0;
	int gx = max(1, (int)(g * sqrt(aspect)));
	int gy = max(1, (int)(g / sqrt(aspect)));
	grid_sz = Size(min(64, gx), min(64, gy));
	cells.SetCount(grid_sz.cx * grid_sz.cy);
	
	for(int i = 0; i < items.GetCount(); i++) {
		const auto& item = items[i];
		Rectf r = item.rect;
		if(item.type == SceneItem::EDGE) {
			if(item.path.GetCount()) {
				r = Rectf(item.path[0], Sizef(0, 0));
				for(const auto& pt : item.path) r.Union(pt);
			}
		}
		
		int x1 = max(0, min(grid_sz.cx - 1, (int)((r.left - bounds.left) / max(1.0, bounds.Width()) * grid_sz.cx)));
		int y1 = max(0, min(grid_sz.cy - 1, (int)((r.top - bounds.top) / max(1.0, bounds.Height()) * grid_sz.cy)));
		int x2 = max(0, min(grid_sz.cx - 1, (int)((r.right - bounds.left) / max(1.0, bounds.Width()) * grid_sz.cx)));
		int y2 = max(0, min(grid_sz.cy - 1, (int)((r.bottom - bounds.top) / max(1.0, bounds.Height()) * grid_sz.cy)));
		
		for(int y = y1; y <= y2; y++)
			for(int x = x1; x <= x2; x++)
				cells[y * grid_sz.cx + x].Add(i);
	}
}

Vector<int> SpatialIndex::Query(Pointf p) const
{
	if(cells.IsEmpty() || !bounds.Contains(p)) return Vector<int>();
	int x = max(0, min(grid_sz.cx - 1, (int)((p.x - bounds.left) / max(1.0, bounds.Width()) * grid_sz.cx)));
	int y = max(0, min(grid_sz.cy - 1, (int)((p.y - bounds.top) / max(1.0, bounds.Height()) * grid_sz.cy)));
	return clone(cells[y * grid_sz.cx + x]);
}

Vector<int> SpatialIndex::Query(Rectf r) const
{
	Vector<int> res;
	if(cells.IsEmpty()) return res;
	
	int x1 = max(0, min(grid_sz.cx - 1, (int)((r.left - bounds.left) / max(1.0, bounds.Width()) * grid_sz.cx)));
	int y1 = max(0, min(grid_sz.cy - 1, (int)((r.top - bounds.top) / max(1.0, bounds.Height()) * grid_sz.cy)));
	int x2 = max(0, min(grid_sz.cx - 1, (int)((r.right - bounds.left) / max(1.0, bounds.Width()) * grid_sz.cx)));
	int y2 = max(0, min(grid_sz.cy - 1, (int)((r.bottom - bounds.top) / max(1.0, bounds.Height()) * grid_sz.cy)));
	
	Index<int> idx;
	for(int y = y1; y <= y2; y++)
		for(int x = x1; x <= x2; x++) {
			for(int i : cells[y * grid_sz.cx + x])
				idx.FindAdd(i);
		}
	return idx.PickKeys();
}

bool BaselineSceneBuilder::IsDirty(const Graph& graph) const
{
	return graph.GetSerial() != last_graph_serial;
}

// ComfyUI-style layout constants (world units)
static const double NODE_W         = 200.0;
static const double TITLE_H        = 26.0;
static const double PIN_ROW_H      = 22.0;
static const double SLOT_ROW_H     = 24.0;
static const double DOCEDIT_ROW_H  = 6 * SLOT_ROW_H; // 6 rows tall by default
static const double DOCEDIT_MIN_W  = 160.0;           // min node width when DocEdit present
static const double IMAGE_H        = 140.0;
static const double PIN_R          = 5.0;   // pin circle radius
static const double H_PAD          = 10.0;

static double SlotHeight(const WidgetSlotDoc& s)
{
	if(s.type == "Image" || s.type == "IMAGE") return IMAGE_H;
	if(s.type == "DocEdit")                    return DOCEDIT_ROW_H;
	return SLOT_ROW_H;
}

static bool IsFullWidthSlot(const WidgetSlotDoc& s)
{
	// Slots that span full width (no separate left-side label)
	return s.type == "IMAGE" || s.type == "Image" ||
	       s.type == "DocEdit" || s.type == "Button" ||
	       s.type == "Null";
}

static bool HasToggle(const WidgetSlotDoc& s)
{
	return s.type == "ToggleButton" || s.type == "ToggleEditDouble";
}

static void AddNodeItems(Scene& scene, const NodeDoc& n, const Graph& graph, BezierRoutingPolicy& router)
{
	// Count inputs/outputs to size the pin area
	int in_count = 0, out_count = 0;
	for (const auto& p : n.pins)
		(p.kind == PinKind::Output ? out_count : in_count)++;
	int pin_rows = max(in_count, out_count);

	// Compute slot area height and required minimum width
	double slot_area_h = 0;
	double min_w = n.sz.cx;
	for (const auto& s : n.slots) {
		slot_area_h += SlotHeight(s);
		if(s.type == "DocEdit") min_w = max(min_w, DOCEDIT_MIN_W + H_PAD * 2);
	}

	double node_h = TITLE_H + pin_rows * PIN_ROW_H + slot_area_h + 8.0;
	double node_w = max(n.sz.cx, min_w); // widen if needed for DocEdit

	// Write computed height back so bounding boxes and hit-tests are accurate
	const_cast<NodeDoc&>(n).sz.cy = node_h;

	Rectf node_rect(n.pos.x, n.pos.y, n.pos.x + node_w, n.pos.y + node_h);

	// Body color — blend tint if set
	Color body_clr = n.fill_clr;
	if (!IsNull(n.tint_clr)) {
		// Simple tint blend: 60% body, 40% tint
		body_clr = Color(
			(body_clr.GetR() * 3 + n.tint_clr.GetR()) / 4,
			(body_clr.GetG() * 3 + n.tint_clr.GetG()) / 4,
			(body_clr.GetB() * 3 + n.tint_clr.GetB()) / 4
		);
	}
	Color title_clr = Color(
		max(0, body_clr.GetR() - 20),
		max(0, body_clr.GetG() - 20),
		max(0, body_clr.GetB() - 20)
	);

	// NODE body
	{
		SceneItem& item = scene.Add();
		item.type = SceneItem::NODE;
		item.entity_id = n.id;
		item.rect = node_rect;
		item.fill_clr = body_clr;
		item.line_clr = n.line_clr;
		item.line_width = n.line_width;
		item.shape = 3; // rounded rect
	}

	// Title bar (dark overlay at top)
	{
		Rectf title_rect(node_rect.left, node_rect.top, node_rect.right, node_rect.top + TITLE_H);
		SceneItem& lbl = scene.Add();
		lbl.type = SceneItem::LABEL;
		lbl.entity_id = n.id;
		lbl.text = n.label.IsEmpty() ? n.id : n.label;
		lbl.rect = title_rect;
		lbl.font_height = 12;
		lbl.font_bold = true;
		lbl.fill_clr = title_clr;
		lbl.line_clr = n.line_clr;
		lbl.text_clr = Color(230, 230, 230);
		lbl.shape = 3; // rounded top corners
		lbl.badge = false;
	}

	// Category text — right-aligned in title bar, smaller font, truncated
	if(!n.category.IsEmpty()) {
		const double MAX_CAT_W = node_w * 0.50; // at most half the title width
		String cat = n.category;
		// Hard-limit: truncate at 12 chars
		if(cat.GetCount() > 12) cat = cat.Left(11) + "…";
		Rectf title_rect(node_rect.left, node_rect.top, node_rect.right, node_rect.top + TITLE_H);
		SceneItem& b = scene.Add();
		b.type = SceneItem::LABEL; b.entity_id = n.id;
		b.text = cat;
		b.rect = Rectf(node_rect.right - MAX_CAT_W, title_rect.top,
		               node_rect.right - 4,          title_rect.bottom);
		b.font_height = 9; b.fill_clr = Null;
		b.line_clr = Null;
		b.text_clr = Color(130, 180, 130); // muted green tint, readable on dark title
		b.badge = false; b.font_italic = true; // right-aligned via font_italic flag
		b.overlay = false;
	}
	// Time badge — top-left, same style as category but default color
	if(!n.time_str.IsEmpty()) {
		const double BH = 14.0;
		double bw = min(70.0, node_w * 0.4);
		Rectf r(node_rect.left + 2, node_rect.top - BH - 2,
		        node_rect.left + bw + 2, node_rect.top - 2);
		SceneItem& b = scene.Add();
		b.type = SceneItem::LABEL; b.entity_id = n.id;
		b.text = n.time_str; b.rect = r;
		b.font_height = 9; b.fill_clr = Color(45, 45, 55);
		b.line_clr = Color(70, 70, 80); b.text_clr = Color(200, 200, 200);
		b.badge = true; b.overlay = true; b.shape = 3;
	}

	// Pin rows — pair inputs on left, outputs on right
	// Gather inputs and outputs in order
	Vector<const PinDoc*> inputs, outputs;
	for (const auto& p : n.pins)
		(p.kind == PinKind::Output ? outputs : inputs).Add(&p);

	double pin_area_top = node_rect.top + TITLE_H + 4.0;
	int max_rows = max(inputs.GetCount(), outputs.GetCount());

	for (int row = 0; row < max_rows; row++) {
		double row_cy = pin_area_top + row * PIN_ROW_H + PIN_ROW_H / 2.0;

		// Input pin (left edge)
		// Entity ID format: "nodeId:in:pinId" — the "in:"/"out:" infix makes
		// same-named input and output pins unique in the scene (e.g. ConditioningZeroOut)
		if (row < inputs.GetCount()) {
			const PinDoc& p = *inputs[row];
			Pointf pin_center(node_rect.left, row_cy);

			// Write pin world position back so edges can use it
			const_cast<PinDoc&>(p).pos = Pointf(0, row_cy - n.pos.y);

			Rectf pr(pin_center.x - PIN_R, pin_center.y - PIN_R,
			         pin_center.x + PIN_R, pin_center.y + PIN_R);
			SceneItem& pi = scene.Add();
			pi.type = SceneItem::PIN;
			pi.entity_id = n.id + ":in:" + p.id;
			pi.rect = pr;
			pi.fill_clr = p.color;
			pi.line_clr = Color(40, 40, 40);
			pi.shape = 1; // circle

			// Input label (right of pin, left-aligned)
			SceneItem& pl = scene.Add();
			pl.type = SceneItem::LABEL;
			pl.entity_id = n.id + ":in:" + p.id;
			pl.badge = false;
			pl.font_italic = false;
			pl.text = p.label;
			pl.font_height = 11;
			pl.rect = Rectf(node_rect.left + H_PAD + PIN_R, row_cy - PIN_ROW_H/2,
			                node_rect.left + node_w/2, row_cy + PIN_ROW_H/2);
			pl.text_clr = Color(200, 200, 200);
			pl.fill_clr = Null;
			pl.line_clr = Null;
		}

		// Output pin (right edge)
		if (row < outputs.GetCount()) {
			const PinDoc& p = *outputs[row];
			Pointf pin_center(node_rect.right, row_cy);

			// Write pin world position back
			const_cast<PinDoc&>(p).pos = Pointf(node_w, row_cy - n.pos.y);

			Rectf pr(pin_center.x - PIN_R, pin_center.y - PIN_R,
			         pin_center.x + PIN_R, pin_center.y + PIN_R);
			SceneItem& pi = scene.Add();
			pi.type = SceneItem::PIN;
			pi.entity_id = n.id + ":out:" + p.id;
			pi.rect = pr;
			pi.fill_clr = p.color;
			pi.line_clr = Color(40, 40, 40);
			pi.shape = 1; // circle

			// Output label (left of pin, right-aligned, uppercase)
			SceneItem& pl = scene.Add();
			pl.type = SceneItem::LABEL;
			pl.entity_id = n.id + ":out:" + p.id;
			pl.badge = true;
			pl.text = ToUpper(p.label.IsEmpty() ? p.id : p.label);
			pl.font_height = 11;
			pl.rect = Rectf(node_rect.left + node_w/2, row_cy - PIN_ROW_H/2,
			                node_rect.right - H_PAD - PIN_R, row_cy + PIN_ROW_H/2);
			pl.text_clr = Color(200, 200, 200);
			pl.fill_clr = Null;
			pl.line_clr = Null;
			pl.font_italic = true; // right-align hint (drawn differently)
		}
	}

	// Widget slots
	double sy = pin_area_top + max_rows * PIN_ROW_H + 4.0;
	const double TOGGLE_W  = 18.0; // width of the toggle button square
	const double SLOT_PAD  = 2.0;

	for (const auto& s : n.slots) {
		double sh = SlotHeight(s);
		EntityId slot_eid = n.id + ":" + s.id;

		if (s.type == "Image" || s.type == "IMAGE") {
			// Full-width image thumbnail
			SceneItem& si = scene.Add();
			si.type = SceneItem::WIDGET;
			si.entity_id = slot_eid;
			si.rect = Rectf(node_rect.left + 1, sy, node_rect.right - 1, sy + sh);
			si.text = s.type;
			Value v = s.properties["value"];
			if(!v.IsVoid() && !v.IsNull())
				si.image_path = v.ToString();

		} else if (s.type == "DocEdit") {
			// Full-width multiline edit
			SceneItem& si = scene.Add();
			si.type = SceneItem::WIDGET;
			si.entity_id = slot_eid;
			si.rect = Rectf(node_rect.left + SLOT_PAD, sy + SLOT_PAD,
			                node_rect.right - SLOT_PAD, sy + sh - SLOT_PAD);
			si.text = s.type;

		} else if (s.type == "Button") {
			// Full-width button
			SceneItem& si = scene.Add();
			si.type = SceneItem::WIDGET;
			si.entity_id = slot_eid;
			si.rect = Rectf(node_rect.left + SLOT_PAD, sy + SLOT_PAD,
			                node_rect.right - SLOT_PAD, sy + sh - SLOT_PAD);
			si.text = s.type;

		} else if (s.type == "Null") {
			// Null slot: just a plain label (no editable ctrl)
			SceneItem& lbl = scene.Add();
			lbl.type = SceneItem::LABEL;
			lbl.entity_id = slot_eid;
			lbl.badge = false;
			lbl.text = s.id;
			lbl.font_height = 10;
			lbl.rect = Rectf(node_rect.left + H_PAD, sy,
			                 node_rect.right - H_PAD, sy + sh);
			lbl.text_clr = Color(120, 120, 130);
			lbl.fill_clr = Color(28, 30, 36);
			lbl.line_clr = Null;

		} else if (s.type == "ToggleButton") {
			// ToggleButton row: toggle square on left, label "Toggle All" centered, value label on right
			SceneItem& toggle = scene.Add();
			toggle.type = SceneItem::WIDGET;
			toggle.entity_id = slot_eid;
			toggle.rect = Rectf(node_rect.left + H_PAD, sy + SLOT_PAD,
			                    node_rect.left + H_PAD + TOGGLE_W, sy + sh - SLOT_PAD);
			toggle.text = s.type;

			SceneItem& lbl = scene.Add();
			lbl.type = SceneItem::LABEL;
			lbl.entity_id = slot_eid;
			lbl.badge = true; // center aligned
			lbl.text = s.id;
			lbl.font_height = 10;
			lbl.rect = Rectf(node_rect.left + H_PAD + TOGGLE_W + 4, sy,
			                 node_rect.right - H_PAD, sy + sh);
			lbl.text_clr = Color(180, 180, 180);
			lbl.fill_clr = Null;
			lbl.line_clr = Null;

		} else if (s.type == "ToggleEditDouble") {
			// ToggleEditDouble: toggle square on left, slot name in middle, EditDouble on right
			SceneItem& toggle = scene.Add();
			toggle.type = SceneItem::WIDGET;
			toggle.entity_id = slot_eid + ":toggle";
			toggle.rect = Rectf(node_rect.left + H_PAD, sy + SLOT_PAD,
			                    node_rect.left + H_PAD + TOGGLE_W, sy + sh - SLOT_PAD);
			toggle.text = "ToggleButton";

			// Label — truncated slot id as the lora name
			SceneItem& lbl = scene.Add();
			lbl.type = SceneItem::LABEL;
			lbl.entity_id = slot_eid;
			lbl.badge = false;
			lbl.text = s.id;
			lbl.font_height = 9;
			double label_right = node_rect.right - 40.0;
			lbl.rect = Rectf(node_rect.left + H_PAD + TOGGLE_W + 4, sy,
			                 label_right, sy + sh);
			lbl.text_clr = Color(180, 180, 180);
			lbl.fill_clr = Null;
			lbl.line_clr = Null;

			// EditDouble on the right
			SceneItem& si = scene.Add();
			si.type = SceneItem::WIDGET;
			si.entity_id = slot_eid;
			si.rect = Rectf(label_right + SLOT_PAD, sy + SLOT_PAD,
			                node_rect.right - SLOT_PAD, sy + sh - SLOT_PAD);
			si.text = "EditDoubleSpin";

		} else {
			// Standard slot row: label on left half, Ctrl on right half
			SceneItem& lbl = scene.Add();
			lbl.type = SceneItem::LABEL;
			lbl.entity_id = slot_eid;
			lbl.badge = false;
			lbl.text = s.id;
			lbl.font_height = 10;
			lbl.rect = Rectf(node_rect.left + H_PAD, sy,
			                 node_rect.left + node_w * 0.5, sy + sh);
			lbl.text_clr = Color(180, 180, 180);
			lbl.fill_clr = Color(28, 30, 36);
			lbl.line_clr = Null;

			SceneItem& si = scene.Add();
			si.type = SceneItem::WIDGET;
			si.entity_id = slot_eid;
			si.rect = Rectf(node_rect.left + node_w * 0.5, sy + SLOT_PAD,
			                node_rect.right - SLOT_PAD, sy + sh - SLOT_PAD);
			si.text = s.type;
		}

		sy += sh;
	}
}

static void AddEdgeItem(Scene& scene, const EdgeDoc& e, const Graph& graph,
                        BezierRoutingPolicy& router, EdgeStyle style, const Vector<Rectf>& obstacles)
{
	const NodeDoc* src_node = graph.FindNode(e.source_node);
	const NodeDoc* tgt_node = graph.FindNode(e.target_node);

	if(src_node && tgt_node) {
		Pointf p1 = src_node->pos + src_node->sz / 2.0;
		Pointf p2 = tgt_node->pos + tgt_node->sz / 2.0;
		Color  pin_clr = Null; // use source pin color for edge

		// Match by name AND kind: source must be Output, target must be Input.
		// Nodes like ConditioningZeroOut have same name for both in and out.
		for(const auto& p : src_node->pins)
			if(p.id == e.source_pin && p.kind == PinKind::Output) {
				p1 = src_node->pos + p.pos;
				pin_clr = p.color;
				break;
			}
		for(const auto& p : tgt_node->pins)
			if(p.id == e.target_pin && p.kind == PinKind::Input)
				{ p2 = tgt_node->pos + p.pos; break; }

		RouteRequest req;
		req.source_pos = p1;
		req.target_pos = p2;
		req.style = style;
		// Net id: source_pin \x01 target_pin — shared port = same net, no overlap penalty
		req.net_id = e.source_pin + "\x01" + e.target_pin;
		// Pass all node boxes except source and target as obstacles
		for(const Rectf& r : obstacles)
			req.obstacles.Add(r);
		RouteResponse resp = router.Route(req);

		SceneItem& item = scene.Add();
		item.type = SceneItem::EDGE;
		item.entity_id = e.id;
		item.path       = pick(resp.path);
		item.seg_layer  = pick(resp.seg_layer);
		item.via_indices = pick(resp.via_indices);
		// Use source pin color if edge has no explicit stroke, otherwise edge doc color
		item.line_clr = !IsNull(pin_clr) ? pin_clr : e.stroke_clr;
		item.line_width = e.line_width;
		item.directed = e.directed;
		item.text = e.label;
	}
}

void BaselineSceneBuilder::Build(Scene& scene, const Graph& graph)
{
	const Index<EntityId>& dirty = graph.GetDirtyEntities();
	const GraphDoc& doc = graph.GetDoc();
	BezierRoutingPolicy router;
	
	LOG("BaselineSceneBuilder::Build " << doc.nodes.GetCount() << " nodes, " << doc.edges.GetCount() << " edges, " << dirty.GetCount() << " dirty");

	// Collect all node bounding boxes.
	// For PCB routing these are BOTH obstacles (passed per-edge to avoid routing through
	// src/tgt nodes) AND the source of truth for the shared PCB grid.
	// Note: computed sz.cy is only accurate AFTER AddNodeItems() has run — so we build
	// nodes first, then collect boxes, then call BeginBatch, then route edges.

	auto CollectAllBoxes = [&]() {
		Vector<Rectf> all;
		for(const auto& n : doc.nodes)
			all.Add(Rectf(n.pos.x, n.pos.y, n.pos.x + n.sz.cx, n.pos.y + n.sz.cy));
		return all;
	};

	auto CollectObstacles = [&](const String& skip_src, const String& skip_tgt) {
		Vector<Rectf> obs;
		for(const auto& n : doc.nodes) {
			if(n.id == skip_src || n.id == skip_tgt) continue;
			obs.Add(Rectf(n.pos.x, n.pos.y, n.pos.x + n.sz.cx, n.pos.y + n.sz.cy));
		}
		return obs;
	};

	// Compute bounding box for a group based on its member nodes
	auto ComputeGroupBounds = [&](const GroupDoc& g) -> Rectf {
		Rectf bounds(1e300, 1e300, -1e300, -1e300);
		bool first = true;
		
		for(const auto& nid : g.nodes) {
			const NodeDoc* node = graph.FindNode(nid);
			if(!node) continue;
			
			Rectf node_rect(node->pos.x, node->pos.y, 
			                node->pos.x + node->sz.cx, node->pos.y + node->sz.cy);
			
			if(first) {
				bounds = node_rect;
				first = false;
			} else {
				bounds.left   = min(bounds.left,   node_rect.left);
				bounds.top    = min(bounds.top,    node_rect.top);
				bounds.right  = max(bounds.right,  node_rect.right);
				bounds.bottom = max(bounds.bottom, node_rect.bottom);
			}
		}
		
		if(first) {
			// Empty group or no valid nodes - use default
			return Rectf(0, 0, 200, 200);
		}
		
		// Add padding around the group (40px on all sides)
		const double GROUP_PAD = 40.0;
		const double TITLE_H   = 30.0; // Space for group title bar
		bounds.left   -= GROUP_PAD;
		bounds.top    -= TITLE_H + GROUP_PAD;
		bounds.right  += GROUP_PAD;
		bounds.bottom += GROUP_PAD;
		
		return bounds;
	};
	
	auto SceneBounds = [&](const Vector<Rectf>& boxes) -> Rectf {
		if(boxes.IsEmpty()) return Rectf(0, 0, 200, 200);
		Rectf b = boxes[0];
		for(const auto& r : boxes) {
			b.left   = min(b.left,   r.left);
			b.top    = min(b.top,    r.top);
			b.right  = max(b.right,  r.right);
			b.bottom = max(b.bottom, r.bottom);
		}
		return b;
	};

	if(dirty.IsEmpty() || scene.items.IsEmpty()) {
		scene.Clear();
		
		// Nodes first: AddNodeItems writes pin positions + sz.cy back into NodeDoc.
		for(const auto& n : doc.nodes) AddNodeItems(scene, n, graph, router);
		
		// Now build groups AFTER nodes have their positions
		for(const auto& g : doc.groups) {
			SceneItem& item = scene.Add();
			item.type = SceneItem::GROUP;
			item.entity_id = g.id;
			item.text = g.label;  // Pass label to scene for rendering
			// Use style-based color or fallback to legacy color
			item.fill_clr = g.style.saturation > 0 ? g.style.ComputeFillColor() : g.color;
			item.line_clr = Color(min(100, (int)item.fill_clr.GetR() + 30),
			                      min(100, (int)item.fill_clr.GetG() + 30),
			                      min(100, (int)item.fill_clr.GetB() + 30));
			item.rect = ComputeGroupBounds(g);
		}

		// Now that node heights are known, initialise the PCB shared grid.
		{
			Vector<Rectf> all_boxes = CollectAllBoxes();
			router.BeginBatch(SceneBounds(all_boxes), all_boxes, edge_style);
		}

		for(const auto& e : doc.edges) {
			Vector<Rectf> obs = CollectObstacles(e.source_node, e.target_node);
			AddEdgeItem(scene, e, graph, router, edge_style, obs);
		}
		LOG("Full build: " << scene.items.GetCount() << " items");
	}
	else {
		// Incremental: for PCB styles do a full rebuild (grid state would be inconsistent
		// if only some edges are re-routed into a partial grid).
		bool is_pcb = (edge_style == EdgeStyle::PCBHVFast ||
		               edge_style == EdgeStyle::PCBHVLee  ||
		               edge_style == EdgeStyle::PCB45);
		if(is_pcb) {
			// Re-run full build: clear and redo everything
			scene.Clear();
			for(const auto& n : doc.nodes) AddNodeItems(scene, n, graph, router);
			
			// Build groups after nodes
			for(const auto& g : doc.groups) {
				SceneItem& item = scene.Add();
				item.type = SceneItem::GROUP;
				item.entity_id = g.id;
				item.text = g.label;  // Pass label to scene for rendering
				item.fill_clr = g.style.saturation > 0 ? g.style.ComputeFillColor() : g.color;
				item.line_clr = Color(min(100, (int)item.fill_clr.GetR() + 30),
				                      min(100, (int)item.fill_clr.GetG() + 30),
				                      min(100, (int)item.fill_clr.GetB() + 30));
				item.rect = ComputeGroupBounds(g);
			}
			{
				Vector<Rectf> all_boxes = CollectAllBoxes();
				router.BeginBatch(SceneBounds(all_boxes), all_boxes, edge_style);
			}
			for(const auto& e : doc.edges) {
				Vector<Rectf> obs = CollectObstacles(e.source_node, e.target_node);
				AddEdgeItem(scene, e, graph, router, edge_style, obs);
			}
			LOG("Full build (PCB incremental promoted): " << scene.items.GetCount() << " items");
		}
		else {
			router.BeginBatch(Rectf(0,0,0,0), Vector<Rectf>(), edge_style);

			// Standard incremental: remove dirty entities
			for(const auto& id : dirty) {
				for(int i = scene.items.GetCount() - 1; i >= 0; i--) {
					if(scene.items[i].entity_id == id || scene.items[i].entity_id.StartsWith(id + ":"))
						scene.items.Remove(i);
				}
				for(const auto& e : doc.edges) {
					if(e.source_node == id || e.target_node == id) {
						for(int i = scene.items.GetCount() - 1; i >= 0; i--) {
							if(scene.items[i].entity_id == e.id)
								scene.items.Remove(i);
						}
					}
				}
				// Also remove affected group items
				for(const auto& g : doc.groups) {
					for(const auto& nid : g.nodes) {
						if(nid == id) {
							// Rebuild this group's rect
							for(int i = scene.items.GetCount() - 1; i >= 0; i--) {
								if(scene.items[i].entity_id == g.id && scene.items[i].type == SceneItem::GROUP)
									scene.items.Remove(i);
							}
							break;
						}
					}
				}
			}

			Index<EntityId> edges_to_add;
			for(const auto& id : dirty) {
				const NodeDoc* n = graph.FindNode(id);
				if(n) {
					AddNodeItems(scene, *n, graph, router);
					for(const auto& e : doc.edges)
						if(e.source_node == id || e.target_node == id)
							edges_to_add.FindAdd(e.id);
				}
				else {
					const EdgeDoc* e = graph.FindEdge(id);
					if(e) edges_to_add.FindAdd(e->id);
				}
			}
			
			// Rebuild affected groups after node updates
			for(const auto& g : doc.groups) {
				bool rebuild_needed = false;
				for(const auto& nid : g.nodes) {
					if(dirty.Find(nid) >= 0) {
						rebuild_needed = true;
						break;
					}
				}
				if(rebuild_needed) {
					SceneItem& item = scene.Add();
					item.type = SceneItem::GROUP;
					item.entity_id = g.id;
					item.text = g.label;  // Pass label to scene for rendering
					item.fill_clr = g.style.saturation > 0 ? g.style.ComputeFillColor() : g.color;
					item.line_clr = Color(min(100, (int)item.fill_clr.GetR() + 30),
					                      min(100, (int)item.fill_clr.GetG() + 30),
					                      min(100, (int)item.fill_clr.GetB() + 30));
					item.rect = ComputeGroupBounds(g);
				}
			}
			
			for(const auto& eid : edges_to_add) {
				const EdgeDoc* e = graph.FindEdge(eid);
				if(e) {
					Vector<Rectf> obs = CollectObstacles(e->source_node, e->target_node);
					AddEdgeItem(scene, *e, graph, router, edge_style, obs);
				}
			}
			LOG("Incremental build: " << scene.items.GetCount() << " items");
		}
	}
	
	scene.Reindex();
	LOG("Spatial index bounds: " << scene.index.bounds);
	last_graph_serial = graph.GetSerial();
	scene.dirty = false;
}

static double DistPointSeg(Pointf p, Pointf a, Pointf b)
{
	double dx = b.x - a.x;
	double dy = b.y - a.y;
	double l2 = dx * dx + dy * dy;
	if(l2 == 0.0) return Distance(p, a);
	double t = max(0.0, min(1.0, ((p.x - a.x) * dx + (p.y - a.y) * dy) / l2));
	Pointf projection = a + t * (b - a);
	return Distance(p, projection);
}

Scene::HitResult Scene::HitTest(Pointf p, double tolerance) const
{
	HitResult res;
	Vector<int> candidates = index.Query(p);
	if(candidates.IsEmpty()) return res;
	
	const SceneItem::Type order[] = { SceneItem::PIN, SceneItem::LABEL, SceneItem::NODE, SceneItem::EDGE, SceneItem::GROUP };
	
	for(auto type : order) {
		for(int i : candidates) {
			const auto& item = items[i];
			if(item.type != type) continue;
			
			if(type == SceneItem::EDGE) {
				for(int j = 0; j < item.path.GetCount() - 1; j++) {
					double d = DistPointSeg(p, item.path[j], item.path[j+1]);
					if(d <= tolerance && d < res.distance) {
						res.entity_id = item.entity_id;
						res.type = item.type;
						res.distance = d;
					}
				}
			}
			else if(item.rect.Contains(p)) {
				res.entity_id = item.entity_id;
				res.type = item.type;
				res.distance = 0;
				return res;
			}
		}
		if(res) return res;
	}
	
	return res;
}

Vector<EntityId> Scene::MarqueeSelect(Rectf r) const
{
	Vector<EntityId> res;
	Vector<int> candidates = index.Query(r);
	Index<EntityId> ids;
	
	for(int i : candidates) {
		const auto& item = items[i];
		if(item.type == SceneItem::NODE || item.type == SceneItem::EDGE || item.type == SceneItem::GROUP) {
			bool inside = false;
			if(item.type == SceneItem::EDGE) {
				inside = true;
				for(const auto& pt : item.path)
					if(!r.Contains(pt)) { inside = false; break; }
			}
			else {
				inside = r.Contains(item.rect);
			}
			
			if(inside && ids.Find(item.entity_id) < 0) {
				res.Add(item.entity_id);
				ids.Add(item.entity_id);
			}
		}
	}
	return res;
}

} // namespace Node

} // namespace Upp
