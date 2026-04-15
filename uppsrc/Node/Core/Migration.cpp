#include "Migration.h"

namespace Upp {

namespace Node {

// Helper: read a double from Value, return default if missing/null
static double ValDouble(const Value& v, const char* key, double def = 0.0)
{
	Value f = v[key];
	return f.IsNull() || f.IsError() ? def : (double)f;
}

static String ValStr(const Value& v, const char* key, const char* def = "")
{
	Value f = v[key];
	return f.IsNull() || f.IsError() ? String(def) : f.ToString();
}

static Color ValColor(const Value& v, const char* key, Color def = Black())
{
	Value c = v[key];
	if(c.IsNull() || c.IsError()) return def;
	return Color((int)ValDouble(c, "red", 0), (int)ValDouble(c, "green", 0), (int)ValDouble(c, "blue", 0));
}

ImportResult ImportGraphLib(Graph& target, const String& legacy_json)
{
	ImportResult res;
	Value jv = ParseJSON(legacy_json);
	if(jv.IsError()) {
		res.warnings.Add(ValidationMessage(ValidationMessage::ERROR, "Legacy JSON parse error"));
		return res;
	}

	target.Clear();

	// Map nodes
	Value nodes = jv["nodes"];
	for(int i = 0; i < nodes.GetCount(); i++) {
		Value n = nodes[i];
		EntityId id = ValStr(n, "id");
		if(id.IsEmpty()) {
			id = "n_import_" + AsString(i);
			res.warnings.Add(ValidationMessage(ValidationMessage::WARNING, "Node at index " + AsString(i) + " has no id, assigned: " + id));
		}
		if(target.FindNode(id)) {
			res.warnings.Add(ValidationMessage(ValidationMessage::WARNING, "Duplicate node id skipped: " + id, id));
			continue;
		}
		NodeDoc& nd = target.AddNode(id);
		nd.label = ValStr(n, "label");
		nd.pos.x  = ValDouble(n["pos"], "x");
		nd.pos.y  = ValDouble(n["pos"], "y");
		nd.sz.cx  = ValDouble(n["sz"], "cx", 100);
		nd.sz.cy  = ValDouble(n["sz"], "cy", 50);
		nd.fill_clr = ValColor(n, "fill_clr", White());

		Value pins = n["pins"];
		for(int j = 0; j < pins.GetCount(); j++) {
			Value p = pins[j];
			EntityId pid = ValStr(p, "id");
			if(pid.IsEmpty()) pid = "p" + AsString(j);
			PinDoc& pd = nd.pins.Add();
			pd.id    = pid;
			pd.label = ValStr(p, "label");
			pd.kind  = (int)ValDouble(p, "kind", 0) == 0 ? PinKind::Input : PinKind::Output;
			pd.pos.x = ValDouble(p["pos"], "x");
			pd.pos.y = ValDouble(p["pos"], "y");
		}
	}

	// Map edges
	Value edges = jv["edges"];
	for(int i = 0; i < edges.GetCount(); i++) {
		Value e = edges[i];
		EntityId id  = ValStr(e, "id");
		if(id.IsEmpty()) id = "e_import_" + AsString(i);
		EntityId src = ValStr(e, "source_node");
		EntityId tgt = ValStr(e, "target_node");
		EntityId sp  = ValStr(e, "source_pin");
		EntityId tp  = ValStr(e, "target_pin");

		if(!target.FindNode(src)) {
			res.warnings.Add(ValidationMessage(ValidationMessage::WARNING, "Edge " + id + " skipped: missing source node " + src, id));
			continue;
		}
		if(!target.FindNode(tgt)) {
			res.warnings.Add(ValidationMessage(ValidationMessage::WARNING, "Edge " + id + " skipped: missing target node " + tgt, id));
			continue;
		}
		if(target.FindEdge(id)) {
			res.warnings.Add(ValidationMessage(ValidationMessage::WARNING, "Duplicate edge id skipped: " + id, id));
			continue;
		}
		EdgeDoc& ed = target.AddEdge(id, src, sp, tgt, tp);
		ed.label      = ValStr(e, "label");
		ed.weight     = ValDouble(e, "weight", 1.0);
		ed.stroke_clr = ValColor(e, "stroke_clr", Black());
	}

	res.success = true;
	return res;
}

EntityId LegacyFacade::AddNode(const String& label, Pointf pos)
{
	EntityId id = "n_" + Uuid::Create().ToString();
	NodeDoc& n = target.AddNode(id);
	n.label = label;
	n.pos = pos;
	return id;
}

EntityId LegacyFacade::AddEdge(const EntityId& src, const EntityId& tgt,
                                const EntityId& src_pin, const EntityId& tgt_pin)
{
	EntityId id = "e_" + Uuid::Create().ToString();

	// Resolve pin IDs: use provided, or fall back to first pin on the node, or ""
	auto first_pin = [&](const EntityId& node_id) -> EntityId {
		const NodeDoc* n = target.FindNode(node_id);
		if(n && n->pins.GetCount()) return n->pins[0].id;
		return String();
	};

	EntityId sp = src_pin.IsEmpty() ? first_pin(src) : src_pin;
	EntityId tp = tgt_pin.IsEmpty() ? first_pin(tgt) : tgt_pin;
	target.AddEdge(id, src, sp, tgt, tp);
	return id;
}

} // namespace Node

} // namespace Upp
