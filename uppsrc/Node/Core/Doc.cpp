#include "Core.h"

namespace Upp {

namespace Node {

void PinDoc::Jsonize(JsonIO& jio)
{
	jio
		("id", id)
		("label", label)
		("kind", (int&)kind)
		("pos", pos)
		("color", color)
		("type_name", type_name)
		("type", type)
		("sz", sz)
	;
}

void PinDoc::Xmlize(XmlIO& xio)
{
	XmlizeByJsonize(xio, *this);
}

void WidgetSlotDoc::Jsonize(JsonIO& jio)
{
	jio("id", id)("type", type)("rect", rect)("properties", properties);
}

void NodeDoc::Jsonize(JsonIO& jio)
{
	jio
		("id", id)
		("label", label)
		("node_type_id", node_type_id)
		("category", category)
		("tint_clr", tint_clr)
		("time_str", time_str)
		("pos", pos)
		("sz", sz)
		("shape", shape)
		("fill_clr", fill_clr)
		("line_clr", line_clr)
		("line_width", line_width)
		("pins", pins)
		("slots", slots)
	;
}

void NodeDoc::Xmlize(XmlIO& xio)
{
	XmlizeByJsonize(xio, *this);
}

void EdgeDoc::Jsonize(JsonIO& jio)
{
	jio
		("id", id)
		("source_node", source_node)
		("source_pin", source_pin)
		("target_node", target_node)
		("target_pin", target_pin)
		("label", label)
		("weight", weight)
		("line_width", line_width)
		("stroke_clr", stroke_clr)
		("directed", directed)
		("attraction", attraction)
	;
}

void EdgeDoc::Xmlize(XmlIO& xio)
{
	XmlizeByJsonize(xio, *this);
}

void GroupDoc::Jsonize(JsonIO& jio)
{
	jio
		("id", id)
		("label", label)
		("vfs_path", vfs_path)
		("nodes", nodes)
		("color", color)
		("style", style)
	;
}

void GroupDoc::Xmlize(XmlIO& xio)
{
	XmlizeByJsonize(xio, *this);
}

void GraphDoc::Jsonize(JsonIO& jio)
{
	jio
		("version", version)
		("nodes", nodes)
		("edges", edges)
		("groups", groups)
	;
}

void GraphDoc::Xmlize(XmlIO& xio)
{
	XmlizeByJsonize(xio, *this);
}

void PinDoc::operator<<=(const PinDoc& src)
{
	id = src.id;
	label = src.label;
	kind = src.kind;
	pos = src.pos;
	color = src.color;
	type_name = src.type_name;
	type = src.type;
	sz = src.sz;
}

void WidgetSlotDoc::operator<<=(const WidgetSlotDoc& src)
{
	id = src.id;
	type = src.type;
	rect = src.rect;
	properties = src.properties;
}

void NodeDoc::operator<<=(const NodeDoc& src)
{
	id = src.id;
	label = src.label;
	node_type_id = src.node_type_id;
	category = src.category;
	tint_clr = src.tint_clr;
	time_str = src.time_str;
	pos = src.pos;
	sz = src.sz;
	shape = src.shape;
	fill_clr = src.fill_clr;
	line_clr = src.line_clr;
	line_width = src.line_width;
	pins <<= src.pins;
	slots <<= src.slots;
}

void EdgeDoc::operator<<=(const EdgeDoc& src)
{
	id = src.id;
	source_node = src.source_node;
	source_pin = src.source_pin;
	target_node = src.target_node;
	target_pin = src.target_pin;
	label = src.label;
	weight = src.weight;
	line_width = src.line_width;
	stroke_clr = src.stroke_clr;
	directed = src.directed;
	attraction = src.attraction;
}

void GroupDoc::operator<<=(const GroupDoc& src)
{
	id = src.id;
	label = src.label;
	vfs_path = src.vfs_path;
	nodes <<= src.nodes;
	color = src.color;
	style = src.style;
}

void GraphDoc::operator<<=(const GraphDoc& src)
{
	version = src.version;
	nodes <<= src.nodes;
	edges <<= src.edges;
	groups <<= src.groups;
}

} // namespace Node

} // namespace Upp
