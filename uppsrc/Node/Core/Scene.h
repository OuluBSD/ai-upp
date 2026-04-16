#ifndef _Node_Core_Scene_h_
#define _Node_Core_Scene_h_

#include "Core.h"
#include "Routing.h"

namespace Upp {

namespace Node {

struct SceneItem : Moveable<SceneItem> {
	enum Type { NODE, PIN, EDGE, GROUP, LABEL, WIDGET };
	Type     type;
	EntityId entity_id;
	
	// Geometry
	Rectf    rect;
	Vector<Pointf> path;
	
	// Styling
	Color    fill_clr = Null;
	Color    line_clr = Black();
	int      line_width = 1;
	String   text;
	String   font_face;
	int      font_height = 12;
	bool     font_bold = false;
	bool     font_italic = false;
	int      shape = 0;     // NODE: 0=Rect, 1=Ellipse, 2=Diamond, 3=RoundRect
	bool     directed = false; // EDGE: draw arrowhead at target end
	String   image_path;    // WIDGET image items
	bool     badge = false;   // overlay label — skip hit testing
	bool     overlay = false; // paint above all node bodies (z-layer 3)
	Color    text_clr = Null; // explicit text color override
	// PCB routing metadata (non-empty only for PCB style edges)
	Vector<int> seg_layer;   // layer per segment (0=front, 1=back)
	Vector<int> via_indices; // path[] indices that are via/bend points

	void Jsonize(JsonIO& jio) {
		jio
			("type", (int&)type)
			("entity_id", entity_id)
			("rect", rect)
			("path", path)
			("fill_clr", fill_clr)
			("line_clr", line_clr)
			("line_width", line_width)
			("text", text)
			("font_face", font_face)
			("font_height", font_height)
			("font_bold", font_bold)
			("font_italic", font_italic)
			("shape", shape)
			("directed", directed)
		;
	}
};

struct SpatialIndex : Moveable<SpatialIndex> {
	Rectf bounds;
	Size  grid_sz;
	Vector<Vector<int>> cells;
	
	void Clear() { cells.Clear(); }
	void Build(const Vector<SceneItem>& items);
	Vector<int> Query(Pointf p) const;
	Vector<int> Query(Rectf r) const;
};

struct Scene : Moveable<Scene> {
	Vector<SceneItem> items;
	SpatialIndex      index;
	bool              dirty = true;
	
	void Clear() { items.Clear(); index.Clear(); dirty = true; }
	SceneItem& Add() { dirty = true; return items.Add(); }
	void Reindex() { index.Build(items); }
	
	void Jsonize(JsonIO& jio) {
		jio("items", items);
	}
	
	// Hit Testing
	struct HitResult {
		EntityId entity_id;
		SceneItem::Type type;
		double distance = 1e300;
		
		operator bool() const { return !entity_id.IsEmpty(); }
	};
	
	HitResult HitTest(Pointf p, double tolerance = 2.0) const;
	Vector<EntityId> MarqueeSelect(Rectf r) const;
};

class SceneBuilder {
public:
	virtual ~SceneBuilder() {}
	virtual bool IsDirty(const Graph& graph) const = 0;
	virtual void Build(Scene& scene, const Graph& graph) = 0;
};

class BaselineSceneBuilder : public SceneBuilder {
	mutable uint64 last_graph_serial = 0;
public:
	EdgeStyle edge_style = EdgeStyle::Curved;

	virtual bool IsDirty(const Graph& graph) const override;
	virtual void Build(Scene& scene, const Graph& graph) override;
};

} // namespace Node

} // namespace Upp

#endif
