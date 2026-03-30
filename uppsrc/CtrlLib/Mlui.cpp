#include "CtrlLib.h"

#include <cstdio>

NAMESPACE_UPP

static bool MluiGetBool(const ValueMap& map, const char *key, bool def)
{
	Value v = map[key];
	if(IsNull(v))
		return def;
	return (bool)v;
}

static int MluiGetInt(const ValueMap& map, const char *key, int def)
{
	Value v = map[key];
	if(IsNull(v))
		return def;
	return (int)v;
}

static String MluiGetString(const ValueMap& map, const char *key, const String& def = String())
{
	Value v = map[key];
	if(IsNull(v))
		return def;
	return (String)v;
}

static void MluiCollectElements(bool include_hidden, Array<AutomationElement>& out)
{
	GuiAutomationVisitor vis;
	vis.include_hidden = include_hidden;
	Vector<Ctrl *> top = Ctrl::GetTopCtrls();
	for(Ctrl *c : top) {
		if(!c)
			continue;
		if(c->IsVisible() || include_hidden) {
			vis.Read(*c);
			for(const AutomationElement& el : vis.elements)
				out.Add(el);
		}
	}
}

static ValueMap MluiElementToJson(const AutomationElement& el, const MluiSnapshotOptions& options)
{
	ValueMap item;
	item.Add("path", el.path);
	item.Add("type", el.semantic_type);
	item.Add("semantic_name", el.semantic_name);
	item.Add("semantic_path", el.semantic_path);
	item.Add("text", el.text);
	item.Add("value", el.value);
	item.Add("checked", el.checked);
	item.Add("enabled", el.enabled);
	item.Add("visible", el.visible);
	item.Add("is_menu", el.is_menu);

	if(options.include_layout) {
		ValueMap rect;
		if(!IsNull(el.rect)) {
			rect.Add("left", el.rect.left);
			rect.Add("top", el.rect.top);
			rect.Add("right", el.rect.right);
			rect.Add("bottom", el.rect.bottom);
			rect.Add("width", el.rect.GetWidth());
			rect.Add("height", el.rect.GetHeight());
		}
		item.Add("rect", rect);
	}
	if(options.include_visible_text)
		item.Add("visible_text", el.visible_text);
	if(options.include_visible_text_ratio)
		item.Add("visible_text_ratio", el.visible_text_ratio);

	return item;
}

Value BuildMluiSnapshot(const MluiSnapshotOptions& options)
{
	ValueArray arr;
	MluiGuiCall([&] {
		Array<AutomationElement> elements;
		MluiCollectElements(options.include_hidden, elements);
		for(const AutomationElement& el : elements)
			arr.Add(MluiElementToJson(el, options));
	});

	ValueMap out;
	out.Add("timestamp", Format("%", GetSysTime()));
	out.Add("count", arr.GetCount());
	out.Add("elements", arr);
	return out;
}

static bool MluiWriteByPath(const String& path, const Value& value, bool do_action, bool include_hidden)
{
	if(path.IsEmpty())
		return false;

	bool found = false;
	MluiGuiCall([&] {
		GuiAutomationVisitor vis;
		vis.include_hidden = include_hidden;
		Vector<Ctrl *> top = Ctrl::GetTopCtrls();
		for(Ctrl *c : top) {
			if(c && (c->IsVisible() || include_hidden)) {
				if(vis.Write(*c, path, value, do_action)) {
					found = true;
					break;
				}
			}
		}
	});
	return found;
}

static String MluiFindPathByPoint(Point p, bool include_hidden)
{
	Array<AutomationElement> elements;
	MluiGuiCall([&] {
		MluiCollectElements(include_hidden, elements);
	});

	int best = -1;
	int best_area = INT_MAX;
	for(int i = 0; i < elements.GetCount(); i++) {
		const AutomationElement& el = elements[i];
		if(el.path.IsEmpty() || IsNull(el.rect))
			continue;
		if(!el.rect.Contains(p))
			continue;
		int area = max(1, el.rect.GetWidth()) * max(1, el.rect.GetHeight());
		if(area <= best_area) {
			best_area = area;
			best = i;
		}
	}

	return best >= 0 ? elements[best].path : String();
}

static bool MluiSendKey(dword key, int count)
{
	bool ok = false;
	MluiGuiCall([&] {
		Ctrl *focus = Ctrl::GetFocusCtrl();
		if(focus) {
			ok = focus->Key(key, count);
			return;
		}

		Vector<Ctrl *> top = Ctrl::GetTopCtrls();
		for(Ctrl *c : top) {
			if(c && c->IsVisible()) {
				if(c->Key(key, count)) {
					ok = true;
					return;
				}
			}
		}
	});
	return ok;
}

String HandleMluiJsonRequest(const String& request_json)
{
	MluiRequest req;
	String parse_error;
	if(!MluiParseJsonRequest(request_json, req, parse_error))
		return MluiMakeJsonResponse(false, Value(), Value(), parse_error);

	Value id = req.id;
	String method = req.method;
	ValueMap p = req.params;

	if(method == "ping") {
		ValueMap m;
		m.Add("text", "pong");
		return MluiMakeJsonResponse(true, id, m);
	}

	if(method == "snapshot") {
		MluiSnapshotOptions o;
		o.include_hidden = MluiGetBool(p, "include_hidden", false);
		o.include_layout = MluiGetBool(p, "include_layout", true);
		o.include_visible_text = MluiGetBool(p, "include_visible_text", true);
		o.include_visible_text_ratio = MluiGetBool(p, "include_visible_text_ratio", true);

		String mode = ToLower(MluiGetString(p, "visibility_mode", "both"));
		if(mode == "none") {
			o.include_visible_text = false;
			o.include_visible_text_ratio = false;
		}
		else if(mode == "absolute" || mode == "text") {
			o.include_visible_text = true;
			o.include_visible_text_ratio = false;
		}
		else if(mode == "ratio" || mode == "percent") {
			o.include_visible_text = false;
			o.include_visible_text_ratio = true;
		}

		return MluiMakeJsonResponse(true, id, BuildMluiSnapshot(o));
	}

	if(method == "click") {
		bool include_hidden = MluiGetBool(p, "include_hidden", false);
		String path = MluiGetString(p, "path");
		if(path.IsEmpty() && !IsNull(p.Get("x", Value())) && !IsNull(p.Get("y", Value()))) {
			Point pt(MluiGetInt(p, "x", 0), MluiGetInt(p, "y", 0));
			path = MluiFindPathByPoint(pt, include_hidden);
		}
		if(path.IsEmpty())
			return MluiMakeJsonResponse(false, id, Value(), "Missing target path/point");
		if(!MluiWriteByPath(path, Value(), true, include_hidden))
			return MluiMakeJsonResponse(false, id, Value(), "Click target not found: " + path);
		ValueMap m;
		m.Add("path", path);
		m.Add("clicked", true);
		return MluiMakeJsonResponse(true, id, m);
	}

	if(method == "set") {
		bool include_hidden = MluiGetBool(p, "include_hidden", false);
		String path = MluiGetString(p, "path");
		if(path.IsEmpty())
			return MluiMakeJsonResponse(false, id, Value(), "Missing path");
		Value value = p.Get("value", Value());
		if(!MluiWriteByPath(path, value, false, include_hidden))
			return MluiMakeJsonResponse(false, id, Value(), "Set target not found: " + path);
		ValueMap m;
		m.Add("path", path);
		m.Add("set", true);
		return MluiMakeJsonResponse(true, id, m);
	}

	if(method == "key") {
		String text = MluiGetString(p, "text");
		if(!text.IsEmpty()) {
			bool ok = true;
			WString w = text.ToWString();
			for(int i = 0; i < w.GetCount(); i++)
				ok = MluiSendKey((dword)w[i], 1) && ok;
			if(!ok)
				return MluiMakeJsonResponse(false, id, Value(), "Key dispatch failed");
			return MluiMakeJsonResponse(true, id, text);
		}

		dword key = (dword)MluiGetInt(p, "key", 0);
		int count = max(1, MluiGetInt(p, "count", 1));
		if(!key)
			return MluiMakeJsonResponse(false, id, Value(), "Missing key/text");
		if(!MluiSendKey(key, count))
			return MluiMakeJsonResponse(false, id, Value(), "Key dispatch failed");
		ValueMap m;
		m.Add("key", (int)key);
		m.Add("count", count);
		return MluiMakeJsonResponse(true, id, m);
	}

	if(method == "mouse") {
		String event = ToLower(MluiGetString(p, "event", "click"));
		if(event == "click" || event == "leftdown" || event == "leftup" || event == "leftdouble") {
			ValueMap p2 = p;
			p2.Add("include_hidden", MluiGetBool(p, "include_hidden", false));
			ValueMap req2;
			req2.Add("method", "click");
			req2.Add("params", p2);
			return HandleMluiJsonRequest(AsJSON(req2));
		}
		return MluiMakeJsonResponse(false, id, Value(), "Unsupported mouse event: " + event);
	}

	return MluiMakeJsonResponse(false, id, Value(), "Unknown method: " + method);
}

void StartMluiRuntime()
{
	static ::Upp::MluiRuntime s_runtime;
	if(s_runtime.IsStarted())
		return;

	bool use_stdio = false;
	bool use_tcp = false;
	String bind_host;
	int bind_port = 0;

#ifdef flagMLUI
	use_stdio = true;
#endif

	String bind_spec = GetMluiServerAddress();
	if(!bind_spec.IsEmpty()) {
		if(MluiParseBindAddress(bind_spec, bind_host, bind_port))
			use_tcp = true;
		else
			std::fprintf(stderr, "MLUI: invalid --mlui-server__ value: %s\n", bind_spec.ToStd().c_str());
	}

	if(!use_stdio && !use_tcp)
		return;

	s_runtime.Configure(use_stdio, use_tcp, bind_host, bind_port);
	s_runtime.SetJsonHandler([](const String& request_json) {
		return HandleMluiJsonRequest(request_json);
	});
	s_runtime.Start();
}

void RegisterMluiRuntimeStarter()
{
	SetMluiRuntimeStarter(StartMluiRuntime);
}

END_UPP_NAMESPACE
