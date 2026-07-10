#include "VisualStateModel/VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// Reads every <property name="..." value="..."/> under `item`'s <properties>
// child (if any) into `el.properties`, in document order, and also mirrors
// the three well-known ones (Type/Variable/Parent) into their own fields for
// convenience. Any property this task doesn't have a named field for (Label,
// Frame, Font.Height, Text.Align, Anchor, ...) is still preserved in
// `properties` — nothing is silently dropped, and an unrecognized property
// name/value never causes a failure or warning (per task 0112's guardrail
// that the parser must be agnostic to whatever widget vocabulary a given
// `.form` file actually uses).
static void VsmReadItemProperties(const XmlNode& item, VsmFormElement& el)
{
	int pi = item.FindTag("properties");
	if(pi < 0)
		return;
	const XmlNode& props = item.Node(pi);
	for(int i = 0; i < props.GetCount(); i++) {
		const XmlNode& p = props.Node(i);
		if(!p.IsTag("property"))
			continue;
		String key = p.Attr("name");
		String val = p.Attr("value");
		if(key.IsEmpty())
			continue;
		el.properties.Add(key, val);
		if(key == "Type")
			el.type = val;
		else if(key == "Variable")
			el.variable = val;
		else if(key == "Parent")
			el.parent = val;
	}
}

// Parses one <content><item ...>...</item> element into a VsmFormElement.
static VsmFormElement VsmReadFormElement(const XmlNode& item)
{
	VsmFormElement el;
	el.x  = item.AttrInt("x", 0);
	el.y  = item.AttrInt("y", 0);
	el.cx = item.AttrInt("cx", 0);
	el.cy = item.AttrInt("cy", 0);

	int ni = item.FindTag("name");
	if(ni >= 0)
		el.name = item.Node(ni).GatherText();

	VsmReadItemProperties(item, el);
	return el;
}

// Parses one <layouts><item> (one designed component) into a VsmFormLayout:
// its <content> child holds the flat element list, its trailing
// <properties> block holds the Form.Width/Form.Height/Form.ScaleMode/
// Form.Name properties (a design-space size, NOT necessarily 1024x648 or any
// other fixed constant — read from the file, never assumed).
static VsmFormLayout VsmReadFormLayout(const XmlNode& layout_item)
{
	VsmFormLayout layout;

	int ci = layout_item.FindTag("content");
	if(ci >= 0) {
		const XmlNode& content = layout_item.Node(ci);
		for(int i = 0; i < content.GetCount(); i++) {
			const XmlNode& item = content.Node(i);
			if(!item.IsTag("item"))
				continue;
			layout.elements.Add(VsmReadFormElement(item));
		}
	}

	int pi = layout_item.FindTag("properties");
	if(pi >= 0) {
		const XmlNode& props = layout_item.Node(pi);
		for(int i = 0; i < props.GetCount(); i++) {
			const XmlNode& p = props.Node(i);
			if(!p.IsTag("property"))
				continue;
			String key = p.Attr("name");
			String val = p.Attr("value");
			if(key == "Form.Width")
				layout.width = StrInt(val);
			else if(key == "Form.Height")
				layout.height = StrInt(val);
			else if(key == "Form.ScaleMode")
				layout.scale_mode = val;
			else if(key == "Form.Name")
				layout.name = val;
		}
	}
	// Fall back to the layout item's own <name> tag if Form.Name wasn't set
	// (not observed in either existing form, but cheap to handle).
	if(layout.name.IsEmpty()) {
		int ni = layout_item.FindTag("name");
		if(ni >= 0)
			layout.name = layout_item.Node(ni).GatherText();
	}

	return layout;
}

Vector<VsmFormLayout> VsmParseFormLayouts(const XmlNode& form_root)
{
	Vector<VsmFormLayout> out;

	const XmlNode& form = form_root.IsTag("form") ? form_root : form_root["form"];
	if(form.IsVoid())
		return out;

	const XmlNode& layouts = form["layouts"];
	if(layouts.IsVoid())
		return out;

	for(int i = 0; i < layouts.GetCount(); i++) {
		const XmlNode& item = layouts.Node(i);
		if(!item.IsTag("item"))
			continue;
		out.Add(VsmReadFormLayout(item));
	}

	return out;
}

Vector<VsmFormLayout> VsmParseFormFile(const String& path)
{
	Vector<VsmFormLayout> out;
	if(!FileExists(path))
		return out;
	XmlNode root = ParseXMLFile(path);
	if(root.IsVoid())
		return out;
	return VsmParseFormLayouts(root);
}

} // namespace Upp
