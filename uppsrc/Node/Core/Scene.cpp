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

static void AddNodeItems(Scene& scene, const NodeDoc& n, const Graph& graph, BezierRoutingPolicy& router)
{
	SceneItem& item = scene.Add();
	item.type = SceneItem::NODE;
	item.entity_id = n.id;
	item.rect = Rectf(n.pos, n.sz);
	item.fill_clr = n.fill_clr;
	item.line_clr = n.line_clr;
	item.line_width = n.line_width;
	item.shape = n.shape;

	if(!n.label.IsEmpty()) {
		SceneItem& lbl = scene.Add();
		lbl.type = SceneItem::LABEL;
		lbl.entity_id = n.id;
		lbl.text = n.label;
		lbl.rect = item.rect;
		lbl.font_height = 14;
		lbl.font_bold = true;
	}
	
	for(const auto& p : n.pins) {
		SceneItem& pi = scene.Add();
		pi.type = SceneItem::PIN;
		pi.entity_id = n.id + ":" + p.id;
		pi.rect = Rectf(n.pos + p.pos - p.sz/2.0, p.sz);
		pi.fill_clr = p.color;
	}
	
	for(const auto& s : n.slots) {
		SceneItem& si = scene.Add();
		si.type = SceneItem::WIDGET;
		si.entity_id = n.id + ":" + s.id;
		si.rect = Rectf(n.pos + s.rect.TopLeft(), s.rect.GetSize());
		si.text = s.type;
	}
}

static void AddEdgeItem(Scene& scene, const EdgeDoc& e, const Graph& graph, BezierRoutingPolicy& router)
{
	const NodeDoc* src_node = graph.FindNode(e.source_node);
	const NodeDoc* tgt_node = graph.FindNode(e.target_node);
	
	if(src_node && tgt_node) {
		Pointf p1 = src_node->pos + src_node->sz / 2.0;
		Pointf p2 = tgt_node->pos + tgt_node->sz / 2.0;
		
		for(const auto& p : src_node->pins)
			if(p.id == e.source_pin) { p1 = src_node->pos + p.pos; break; }
		for(const auto& p : tgt_node->pins)
			if(p.id == e.target_pin) { p2 = tgt_node->pos + p.pos; break; }
		
		RouteRequest req;
		req.source_pos = p1;
		req.target_pos = p2;
		RouteResponse resp = router.Route(req);
		
		SceneItem& item = scene.Add();
		item.type = SceneItem::EDGE;
		item.entity_id = e.id;
		item.path = pick(resp.path);
		item.line_clr = e.stroke_clr;
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

	if(dirty.IsEmpty()) {
		scene.Clear();
		for(const auto& g : doc.groups) {
			SceneItem& item = scene.Add();
			item.type = SceneItem::GROUP;
			item.entity_id = g.id;
			item.fill_clr = g.color;
		}
		for(const auto& e : doc.edges) AddEdgeItem(scene, e, graph, router);
		for(const auto& n : doc.nodes) AddNodeItems(scene, n, graph, router);
	}
	else {
		// Incremental: remove dirty entities
		for(const auto& id : dirty) {
			for(int i = scene.items.GetCount() - 1; i >= 0; i--) {
				if(scene.items[i].entity_id == id || scene.items[i].entity_id.StartsWith(id + ":"))
					scene.items.Remove(i);
			}
			// Also edges connected to dirty nodes
			for(const auto& e : doc.edges) {
				if(e.source_node == id || e.target_node == id) {
					for(int i = scene.items.GetCount() - 1; i >= 0; i--) {
						if(scene.items[i].entity_id == e.id)
							scene.items.Remove(i);
					}
				}
			}
		}
		
		// Re-add dirty entities; collect edges to re-add in a set to avoid duplicates
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
		for(const auto& eid : edges_to_add) {
			const EdgeDoc* e = graph.FindEdge(eid);
			if(e) AddEdgeItem(scene, *e, graph, router);
		}
	}
	
	scene.Reindex();
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
