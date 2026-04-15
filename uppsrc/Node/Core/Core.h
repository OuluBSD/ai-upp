#ifndef _Node_Core_Core_h_
#define _Node_Core_Core_h_

#include <Core/Core.h>

namespace Upp {

namespace Node {

// --- ID Model ---

using EntityId = String;

inline bool IsValidEntityId(const EntityId& id)
{
	if(id.IsEmpty())
		return false;
	for(int i = 0; i < id.GetCount(); i++) {
		int c = id[i];
		if(!IsAlNum(c) && c != '_' && c != '-')
			return false;
	}
	return true;
}

inline EntityId NormalizeEntityId(const EntityId& id)
{
	return id;
}

// --- Domain Model (Persistent) ---

enum class PinKind { Input, Output };

struct PinDoc : Moveable<PinDoc> {
	EntityId id;
	String   label;
	PinKind  kind = PinKind::Input;
	Pointf   pos; // Local position relative to node
	Color    color = Black();
	int      type = 0;
	Sizef    sz = Sizef(10, 10);
	
	void Jsonize(JsonIO& jio);
	void Xmlize(XmlIO& xio);
	void operator<<=(const PinDoc& src);
	PinDoc(const PinDoc& src) { *this <<= src; }
	PinDoc() {}
};

struct WidgetSlotDoc : Moveable<WidgetSlotDoc> {
	EntityId id;
	String   type;
	Rectf    rect; // Local rect
	ValueMap properties;
	
	void Jsonize(JsonIO& jio);
	void operator<<=(const WidgetSlotDoc& src);
	WidgetSlotDoc(const WidgetSlotDoc& src) { *this <<= src; }
	WidgetSlotDoc() {}
};

struct NodeDoc : Moveable<NodeDoc> {
	EntityId   id;
	String     label;
	Pointf     pos; // World position
	Sizef      sz = Sizef(100, 50);
	int        shape = 0; // 0: Rect, 1: Ellipse, 2: Diamond
	Color      fill_clr = White();
	Color      line_clr = Black();
	int        line_width = 1;
	Array<PinDoc> pins;
	Array<WidgetSlotDoc> slots;
	
	void Jsonize(JsonIO& jio);
	void Xmlize(XmlIO& xio);
	void operator<<=(const NodeDoc& src);
	NodeDoc(const NodeDoc& src) { *this <<= src; }
	NodeDoc() {}
};

struct EdgeDoc : Moveable<EdgeDoc> {
	EntityId id;
	EntityId source_node;
	EntityId source_pin;
	EntityId target_node;
	EntityId target_pin;
	String   label;
	double   weight = 1.0;
	int      line_width = 1;
	Color    stroke_clr = Black();
	bool     directed = false;
	double   attraction = 1.0;
	
	void Jsonize(JsonIO& jio);
	void Xmlize(XmlIO& xio);
	void operator<<=(const EdgeDoc& src);
	EdgeDoc(const EdgeDoc& src) { *this <<= src; }
	EdgeDoc() {}
};

struct GroupDoc : Moveable<GroupDoc> {
	EntityId      id;
	String        label;
	Vector<EntityId> nodes;
	Color         color = Gray();
	
	void Jsonize(JsonIO& jio);
	void Xmlize(XmlIO& xio);
	void operator<<=(const GroupDoc& src);
	GroupDoc(const GroupDoc& src) { *this <<= src; }
	GroupDoc() {}
};

struct GraphDoc : Moveable<GraphDoc> {
	int           version = 1;
	Array<NodeDoc>  nodes;
	Array<EdgeDoc>  edges;
	Array<GroupDoc> groups;
	
	void Jsonize(JsonIO& jio);
	void Xmlize(XmlIO& xio);
	void operator<<=(const GraphDoc& src);
	GraphDoc(const GraphDoc& src) { *this <<= src; }
	GraphDoc() {}
};

struct Style : Moveable<Style> {
	Color  fill_clr = White();
	Color  line_clr = Black();
	int    line_width = 1;
	String font_face;
	int    font_height = 12;
	bool   font_bold = false;
	
	void Jsonize(JsonIO& jio) {
		jio
			("fill_clr", fill_clr)
			("line_clr", line_clr)
			("line_width", line_width)
			("font_face", font_face)
			("font_height", font_height)
			("font_bold", font_bold)
		;
	}
};

// --- Model Services ---

struct ValidationMessage : Moveable<ValidationMessage> {
	enum Severity { INFO, WARNING, ERROR };
	Severity severity;
	String   message;
	EntityId entity_id;
	
	ValidationMessage(Severity s, const String& m, const EntityId& id = String())
		: severity(s), message(m), entity_id(id) {}
	ValidationMessage() : severity(INFO) {}
};

struct PerfMetrics : public Moveable<PerfMetrics> {
	double scene_build_ms = 0;
	double paint_bridge_ms = 0;
	double hit_query_ms = 0;
	double command_latency_ms = 0;
	
	void Jsonize(JsonIO& jio) {
		jio
			("scene_build_ms", scene_build_ms)
			("paint_bridge_ms", paint_bridge_ms)
			("hit_query_ms", hit_query_ms)
			("command_latency_ms", command_latency_ms)
		;
	}
};

class Graph {
	GraphDoc    doc;
	PerfMetrics metrics;
	uint64      serial = 1;
	Index<EntityId> dirty_entities;

	// O(1) lookup indices — kept in sync with doc arrays
	VectorMap<EntityId, int> node_idx;
	VectorMap<EntityId, int> edge_idx;
	VectorMap<EntityId, int> group_idx;

	void RebuildIndex();

public:
	// Rebuild O(1) lookup indices from current doc state (needed after direct doc manipulation)
	void RebuildIndexPublic() { RebuildIndex(); }
	void      Invalidate() { serial++; dirty_entities.Clear(); }
	void      Invalidate(const EntityId& id) { serial++; dirty_entities.FindAdd(id); }
	uint64    GetSerial() const { return serial; }
	const Index<EntityId>& GetDirtyEntities() const { return dirty_entities; }

	// CRUD
	NodeDoc&  AddNode(const EntityId& id);
	void      RemoveNode(const EntityId& id);
	NodeDoc*  FindNode(const EntityId& id);
	const NodeDoc* FindNode(const EntityId& id) const;

	EdgeDoc&  AddEdge(const EntityId& id, const EntityId& source_node, const EntityId& source_pin,
	                  const EntityId& target_node, const EntityId& target_pin);
	void      RemoveEdge(const EntityId& id);
	EdgeDoc*  FindEdge(const EntityId& id);
	const EdgeDoc* FindEdge(const EntityId& id) const;

	GroupDoc& AddGroup(const EntityId& id);
	void      RemoveGroup(const EntityId& id);
	GroupDoc* FindGroup(const EntityId& id);
	const GroupDoc* FindGroup(const EntityId& id) const;

	// Validation
	Vector<ValidationMessage> Validate() const;

	// IO
	bool      LoadJson(const String& json, Vector<ValidationMessage>& errors);
	String    SaveJson() const;

	bool      LoadXml(const String& xml, Vector<ValidationMessage>& errors);
	String    SaveXml() const;

	// Access
	const GraphDoc& GetDoc() const { return doc; }
	GraphDoc&       GetDoc()       { return doc; }

	PerfMetrics&       GetMetrics()       { return metrics; }
	const PerfMetrics& GetMetrics() const { return metrics; }

	void Clear() { doc.nodes.Clear(); doc.edges.Clear(); doc.groups.Clear();
	               node_idx.Clear(); edge_idx.Clear(); group_idx.Clear(); Invalidate(); }
};

// --- Runtime State (Transient) ---

struct NodeState : Moveable<NodeState> {
	Pointf layout_pos;
	Pointf layout_force;
	bool is_selected = false;
	bool is_hovered = false;
	Pointf target_pos;
	bool   is_animating = false;
	double distance = 1e300;
	bool   optimized = false;
	int    sort_importance = 0;
	EntityId predecessor;
};

struct EdgeState : Moveable<EdgeState> {
	bool   is_selected = false;
	bool   is_hovered = false;
	double flow_offset = 0;
	bool   is_flow_animating = false;
	double flow_speed = 0;
};

struct GroupState : Moveable<GroupState> {
	bool is_selected = false;
	bool is_collapsed = false;
};

} // namespace Node

} // namespace Upp

#endif
