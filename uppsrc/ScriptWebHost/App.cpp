#include "ScriptWebHost.h"

NAMESPACE_UPP

static const XmlNode* FindChildTag(const XmlNode& node, const char* tag)
{
	for(int i = 0; i < node.GetCount(); i++)
		if(node[i].IsTag(tag))
			return &node[i];
	return nullptr;
}

static String FindPropertyValue(const XmlNode& properties, const char* key)
{
	for(int i = 0; i < properties.GetCount(); i++) {
		const XmlNode& prop = properties[i];
		if(prop.IsTag("property") && prop.Attr("name") == key)
			return prop.Attr("value");
	}
	return String();
}

static void ResolveAnchorRect(Rect& r, String anchor, const Size& base_sz, String& h_mode, String& v_mode)
{
	int cx = r.Width();
	int cy = r.Height();
	int x = r.left;
	int y = r.top;

	anchor = ToUpper(anchor);
	if(anchor == "CENTER") {
		r.left = x - (base_sz.cx - cx) / 2;
		r.top = y - (base_sz.cy - cy) / 2;
		h_mode = "CENTER";
		v_mode = "CENTER";
	}
	else if(anchor == "BOTTOM_CENTER") {
		r.left = x - (base_sz.cx - cx) / 2;
		r.top = base_sz.cy - y - cy;
		h_mode = "CENTER";
		v_mode = "BOTTOM";
	}
	else if(anchor == "TOP_CENTER") {
		r.left = x - (base_sz.cx - cx) / 2;
		r.top = y;
		h_mode = "CENTER";
		v_mode = "TOP";
	}
	else if(anchor == "CENTER_LEFT") {
		r.left = x;
		r.top = y - (base_sz.cy - cy) / 2;
		h_mode = "LEFT";
		v_mode = "CENTER";
	}
	else if(anchor == "CENTER_RIGHT") {
		r.left = base_sz.cx - x - cx;
		r.top = y - (base_sz.cy - cy) / 2;
		h_mode = "RIGHT";
		v_mode = "CENTER";
	}
	else if(anchor == "BOTTOM_LEFT") {
		r.left = x;
		r.top = base_sz.cy - y - cy;
		h_mode = "LEFT";
		v_mode = "BOTTOM";
	}
	else if(anchor == "BOTTOM_RIGHT") {
		r.left = base_sz.cx - x - cx;
		r.top = base_sz.cy - y - cy;
		h_mode = "RIGHT";
		v_mode = "BOTTOM";
	}
	else if(anchor == "TOP_HSIZE") {
		r.left = x;
		r.top = y;
		h_mode = "HSIZE";
		v_mode = "TOP";
	}
	else if(anchor == "CENTER_HSIZE") {
		r.left = x;
		r.top = y - (base_sz.cy - cy) / 2;
		h_mode = "HSIZE";
		v_mode = "CENTER";
	}
	else if(anchor == "BOTTOM_HSIZE") {
		r.left = x;
		r.top = base_sz.cy - y - cy;
		h_mode = "HSIZE";
		v_mode = "BOTTOM";
	}
	else if(anchor == "LEFT_VSIZE") {
		r.left = x;
		r.top = y;
		h_mode = "LEFT";
		v_mode = "VSIZE";
	}
	else if(anchor == "CENTER_VSIZE") {
		r.left = x - (base_sz.cx - cx) / 2;
		r.top = y;
		h_mode = "CENTER";
		v_mode = "VSIZE";
	}
	else if(anchor == "RIGHT_VSIZE") {
		r.left = base_sz.cx - x - cx;
		r.top = y;
		h_mode = "RIGHT";
		v_mode = "VSIZE";
	}
	else if(anchor == "SIZE") {
		r.left = x;
		r.top = y;
		h_mode = "HSIZE";
		v_mode = "VSIZE";
	}
	else if(anchor == "TOP_RIGHT") {
		r.left = base_sz.cx - x - cx;
		r.top = y;
		h_mode = "RIGHT";
		v_mode = "TOP";
	}
	else {
		r.left = x;
		r.top = y;
		h_mode = "LEFT";
		v_mode = "TOP";
	}

	r.right = r.left + cx;
	r.bottom = r.top + cy;
}

static Value MakeRectJson(const Rect& r)
{
	ValueMap m;
	m.Add("left", r.left);
	m.Add("top", r.top);
	m.Add("right", r.right);
	m.Add("bottom", r.bottom);
	m.Add("width", r.Width());
	m.Add("height", r.Height());
	return m;
}

static Value MakeNormalizedRectJson(const Rect& r, const Size& base_sz)
{
	ValueMap m;
	double bw = base_sz.cx > 0 ? (double)base_sz.cx : 1.0;
	double bh = base_sz.cy > 0 ? (double)base_sz.cy : 1.0;
	m.Add("left", r.left / bw);
	m.Add("top", r.top / bh);
	m.Add("right", r.right / bw);
	m.Add("bottom", r.bottom / bh);
	m.Add("width", r.Width() / bw);
	m.Add("height", r.Height() / bh);
	return m;
}

static Value MakeParentLocalLayoutJson(const Rect& child_rect, const Rect& parent_rect)
{
	ValueMap m;
	int pw = max(1, parent_rect.Width());
	int ph = max(1, parent_rect.Height());
	Rect local = child_rect;
	local.Offset(-parent_rect.left, -parent_rect.top);
	m.Add("rect", MakeRectJson(local));
	m.Add("normalized_rect", MakeNormalizedRectJson(local, Size(pw, ph)));

	ValueMap browser;
	browser.Add("position", "absolute");
	browser.Add("left", (double)local.left / pw);
	browser.Add("top", (double)local.top / ph);
	browser.Add("width", (double)local.Width() / pw);
	browser.Add("height", (double)local.Height() / ph);
	browser.Add("transform_x", 0.0);
	browser.Add("transform_y", 0.0);
	browser.Add("stretch_x", false);
	browser.Add("stretch_y", false);
	m.Add("browser", browser);
	return m;
}

static Value MakeBrowserLayoutJson(int x, int y, int cx, int cy, String anchor, const Size& base_sz)
{
	ValueMap m;
	double bw = base_sz.cx > 0 ? (double)base_sz.cx : 1.0;
	double bh = base_sz.cy > 0 ? (double)base_sz.cy : 1.0;
	double left = x / bw;
	double top = y / bh;
	double right = (base_sz.cx - x - cx) / bw;
	double bottom = (base_sz.cy - y - cy) / bh;
	double width = cx / bw;
	double height = cy / bh;

	anchor = ToUpper(anchor);
	m.Add("position", "absolute");
	m.Add("anchor", anchor);
	m.Add("width", width);
	m.Add("height", height);
	m.Add("stretch_x", false);
	m.Add("stretch_y", false);
	m.Add("transform_x", 0.0);
	m.Add("transform_y", 0.0);

	if(anchor == "CENTER") {
		m.Add("left", left);
		m.Add("top", top);
		m.Set("transform_x", -0.5);
		m.Set("transform_y", -0.5);
	}
	else if(anchor == "BOTTOM_CENTER") {
		m.Add("left", left);
		m.Add("bottom", y / bh);
		m.Set("transform_x", -0.5);
	}
	else if(anchor == "TOP_CENTER") {
		m.Add("left", left);
		m.Add("top", top);
		m.Set("transform_x", -0.5);
	}
	else if(anchor == "CENTER_LEFT") {
		m.Add("left", left);
		m.Add("top", top);
		m.Set("transform_y", -0.5);
	}
	else if(anchor == "CENTER_RIGHT") {
		m.Add("right", x / bw);
		m.Add("top", top);
		m.Set("transform_y", -0.5);
	}
	else if(anchor == "BOTTOM_LEFT") {
		m.Add("left", left);
		m.Add("bottom", y / bh);
	}
	else if(anchor == "BOTTOM_RIGHT") {
		m.Add("right", x / bw);
		m.Add("bottom", y / bh);
	}
	else if(anchor == "TOP_HSIZE") {
		m.Add("left", left);
		m.Add("right", right);
		m.Add("top", top);
		m.Set("stretch_x", true);
	}
	else if(anchor == "CENTER_HSIZE") {
		m.Add("left", left);
		m.Add("right", right);
		m.Add("top", top);
		m.Set("stretch_x", true);
		m.Set("transform_y", -0.5);
	}
	else if(anchor == "BOTTOM_HSIZE") {
		m.Add("left", left);
		m.Add("right", right);
		m.Add("bottom", y / bh);
		m.Set("stretch_x", true);
	}
	else if(anchor == "LEFT_VSIZE") {
		m.Add("left", left);
		m.Add("top", top);
		m.Add("bottom", bottom);
		m.Set("stretch_y", true);
	}
	else if(anchor == "CENTER_VSIZE") {
		m.Add("left", left);
		m.Add("top", top);
		m.Add("bottom", bottom);
		m.Set("stretch_y", true);
		m.Set("transform_x", -0.5);
	}
	else if(anchor == "RIGHT_VSIZE") {
		m.Add("right", x / bw);
		m.Add("top", top);
		m.Add("bottom", bottom);
		m.Set("stretch_y", true);
	}
	else if(anchor == "SIZE") {
		m.Add("left", left);
		m.Add("right", right);
		m.Add("top", top);
		m.Add("bottom", bottom);
		m.Set("stretch_x", true);
		m.Set("stretch_y", true);
	}
	else if(anchor == "TOP_RIGHT") {
		m.Add("right", x / bw);
		m.Add("top", top);
	}
	else {
		m.Add("left", left);
		m.Add("top", top);
	}
	return m;
}

static Value ParseFormSummary(const String& form_path)
{
	ValueMap out;
	out.Add("path", form_path);
	if(form_path.IsEmpty() || !FileExists(form_path)) {
		out.Add("error", "form file not found");
		return out;
	}

	XmlNode xml;
	try {
		xml = ParseXML(LoadFile(form_path));
	}
	catch(Exc& e) {
		out.Add("error", "xml parse failed: " + String(e));
		return out;
	}

	const XmlNode* form = nullptr;
	if(xml.IsTag("form"))
		form = &xml;
	else
		form = FindChildTag(xml, "form");
	if(!form) {
		out.Add("error", "missing <form> root");
		return out;
	}

	const XmlNode* layouts = FindChildTag(*form, "layouts");
	const XmlNode* item = layouts ? FindChildTag(*layouts, "item") : nullptr;
	const XmlNode* content = item ? FindChildTag(*item, "content") : nullptr;
	const XmlNode* properties = item ? FindChildTag(*item, "properties") : nullptr;

	ValueMap meta;
	Size form_size(400, 300);
	if(properties) {
		meta.Add("width", FindPropertyValue(*properties, "Form.Width"));
		meta.Add("height", FindPropertyValue(*properties, "Form.Height"));
		meta.Add("name", FindPropertyValue(*properties, "Form.Name"));
		meta.Add("background", FindPropertyValue(*properties, "CardGame.Background"));
		form_size.cx = max(1, ScanInt(FindPropertyValue(*properties, "Form.Width")));
		form_size.cy = max(1, ScanInt(FindPropertyValue(*properties, "Form.Height")));
	}
	meta.Add("width_int", form_size.cx);
	meta.Add("height_int", form_size.cy);
	out.Add("meta", meta);

	ValueArray objects;
	VectorMap<String, int> object_index;
	Vector<Rect> resolved_rects;
	if(content) {
		for(int i = 0; i < content->GetCount(); i++) {
			const XmlNode& obj = (*content)[i];
			if(!obj.IsTag("item"))
				continue;

			ValueMap one;
			int x = obj.AttrInt("x");
			int y = obj.AttrInt("y");
			int cx = obj.AttrInt("cx");
			int cy = obj.AttrInt("cy");
			one.Add("x", x);
			one.Add("y", y);
			one.Add("cx", cx);
			one.Add("cy", cy);
			one.Add("align", obj.AttrInt("align"));
			one.Add("valign", obj.AttrInt("valign"));

			const XmlNode* name = FindChildTag(obj, "name");
			const XmlNode* obj_props = FindChildTag(obj, "properties");
			if(name)
				one.Add("name", ~(*name));
			if(obj_props) {
				one.Add("id", FindPropertyValue(*obj_props, "Variable"));
				one.Add("type", FindPropertyValue(*obj_props, "Type"));
				one.Add("user_class", FindPropertyValue(*obj_props, "UserClass"));
				one.Add("anchor", FindPropertyValue(*obj_props, "Anchor"));
				one.Add("parent", FindPropertyValue(*obj_props, "Parent"));
				one.Add("label", FindPropertyValue(*obj_props, "Label"));
				one.Add("action", FindPropertyValue(*obj_props, "Action"));
				one.Add("tip", FindPropertyValue(*obj_props, "Tip"));
			}

			Rect raw_rect(x, y, x + cx, y + cy);
			String h_mode, v_mode;
			ResolveAnchorRect(raw_rect, AsString(one["anchor"]), form_size, h_mode, v_mode);
			ValueMap layout;
			layout.Add("anchor", one["anchor"]);
			layout.Add("h_mode", h_mode);
			layout.Add("v_mode", v_mode);
			layout.Add("base_rect", MakeRectJson(raw_rect));
			layout.Add("normalized_rect", MakeNormalizedRectJson(raw_rect, form_size));
			layout.Add("edge_offsets", ValueMap());
			ValueMap edges;
			edges.Add("left", raw_rect.left);
			edges.Add("top", raw_rect.top);
			edges.Add("right", form_size.cx - raw_rect.right);
			edges.Add("bottom", form_size.cy - raw_rect.bottom);
			layout.Set("edge_offsets", edges);
			ValueMap edge_ratios;
			edge_ratios.Add("left", form_size.cx ? (double)raw_rect.left / form_size.cx : 0.0);
			edge_ratios.Add("top", form_size.cy ? (double)raw_rect.top / form_size.cy : 0.0);
			edge_ratios.Add("right", form_size.cx ? (double)(form_size.cx - raw_rect.right) / form_size.cx : 0.0);
			edge_ratios.Add("bottom", form_size.cy ? (double)(form_size.cy - raw_rect.bottom) / form_size.cy : 0.0);
			layout.Add("edge_ratios", edge_ratios);
			layout.Add("browser", MakeBrowserLayoutJson(x, y, cx, cy, AsString(one["anchor"]), form_size));
			one.Add("layout", layout);
			String id = AsString(one["id"]);
			if(!id.IsEmpty())
				object_index.Add(id, objects.GetCount());
			resolved_rects.Add(raw_rect);
			objects.Add(one);
		}
	}

	for(int i = 0; i < objects.GetCount(); i++) {
		ValueMap one = objects[i];
		String parent = AsString(one["parent"]);
		if(!parent.IsEmpty()) {
			int q = object_index.Find(parent);
			if(q >= 0 && q < resolved_rects.GetCount()) {
				ValueMap layout = one["layout"];
				layout.Add("parent_id", parent);
				layout.Add("local_to_parent", MakeParentLocalLayoutJson(resolved_rects[i], resolved_rects[q]));
				one.Set("layout", layout);
				objects.Set(i, one);
			}
		}
	}
	out.Add("objects", objects);
	return out;
}

ScriptWebHostApp::ScriptWebHostApp()
{
	root = "";
	path = GetCurrentDirectory();
	static_dir = "static";
	browser_root = "/";
	session_id = "default";
	port = 9234;
	ip = "127.0.0.1";
	threads = 1;
#ifdef _DEBUG
	prefork = 0;
	use_caching = false;
#endif
}

bool ScriptWebHostApp::ApplyCommandLine(const Vector<String>& args, String& error, bool& show_help)
{
	show_help = false;
	for(const String& arg : args) {
		if(arg == "--help" || arg == "-h") {
			show_help = true;
			return true;
		}
		if(arg.StartsWith("--port=")) {
			int p = ScanInt(arg.Mid(7));
			if(p <= 0 || p > 65535) {
				error = "invalid port: " + arg.Mid(7);
				return false;
			}
			port = p;
		}
		else if(arg.StartsWith("--ip="))
			ip = arg.Mid(5);
		else if(arg.StartsWith("--session="))
			session_id = arg.Mid(10);
		else if(arg.StartsWith("--gamestate="))
			gamestate_path = NormalizePath(arg.Mid(12));
		else if(arg.StartsWith("--root="))
			path = NormalizePath(arg.Mid(7));
		else if(arg.StartsWith("--browser-root="))
			browser_root = arg.Mid(15);
		else if(arg == "--verbose")
			verbose_logging = true;
		else {
			error = "unknown argument: " + arg;
			return false;
		}
	}
	if(path.IsEmpty())
		path = GetCurrentDirectory();
	if(browser_root.IsEmpty())
		browser_root = "/";
	return true;
}

String ScriptWebHostApp::GetStatusJson() const
{
	ValueMap m;
	m.Add("status", "ok");
	m.Add("session_id", session_id);
	m.Add("gamestate_path", gamestate_path);
	m.Add("browser_root", browser_root);
	m.Add("ip", ip);
	m.Add("port", port);
	return AsJSON(m, true);
}

String ScriptWebHostApp::GetBootstrapJson() const
{
	ValueMap m;
	m.Add("session_id", session_id);
	m.Add("browser_root", browser_root);
	m.Add("gamestate_path", gamestate_path);

	Value gamestate = Null;
	String gamestate_dir;
	if(!gamestate_path.IsEmpty() && FileExists(gamestate_path)) {
		gamestate_dir = GetFileDirectory(gamestate_path);
		String json = LoadFile(gamestate_path);
		Value parsed = ParseJSON(json);
		if(!parsed.IsVoid())
			gamestate = parsed;
	}

	m.Add("gamestate_dir", gamestate_dir);
	m.Add("gamestate", gamestate);

	if(gamestate.Is<ValueMap>()) {
		ValueMap resolved;
		String entry_script = AsString(gamestate["entry_script"]);
		String entry_language = AsString(gamestate["entry_language"]);
		String layout = AsString(gamestate["layout"]);
		String host = AsString(gamestate["host"]);
		String runtime_host = AsString(gamestate["runtime_host"]);
		if(entry_language.IsEmpty())
			entry_language = "python";
		resolved.Add("entry_language", entry_language);
		if(!entry_script.IsEmpty())
			resolved.Add("entry_script_path", AppendFileName(gamestate_dir, entry_script));
		if(!layout.IsEmpty())
			resolved.Add("layout_path", AppendFileName(gamestate_dir, layout));
		if(!host.IsEmpty())
			resolved.Add("host", host);
		if(!runtime_host.IsEmpty())
			resolved.Add("runtime_host", runtime_host);
		m.Add("resolved", resolved);
		if(!layout.IsEmpty())
			m.Add("form", ParseFormSummary(AppendFileName(gamestate_dir, layout)));
		if(ToLower(entry_language) == "python")
			m.Add("transpile", StoreAsJsonValue(GetEntryTranspile()));
	}

	return AsJSON(m, true);
}

PyToJsResult ScriptWebHostApp::GetEntryTranspile() const
{
	PyToJsResult out;
	if(gamestate_path.IsEmpty() || !FileExists(gamestate_path)) {
		out.errors.Add("missing gamestate file");
		return out;
	}
	Value gamestate = ParseJSON(LoadFile(gamestate_path));
	if(gamestate.IsVoid()) {
		out.errors.Add("invalid gamestate json");
		return out;
	}
	String entry_script = AsString(gamestate["entry_script"]);
	String entry_language = AsString(gamestate["entry_language"]);
	if(entry_language.IsEmpty())
		entry_language = "python";
	if(ToLower(entry_language) != "python") {
		out.errors.Add("transpiler currently expects python entry source");
		return out;
	}
	if(entry_script.IsEmpty()) {
		out.errors.Add("missing entry_script");
		return out;
	}
	String entry_path = AppendFileName(GetFileDirectory(gamestate_path), entry_script);
	if(!FileExists(entry_path)) {
		out.errors.Add("entry script not found: " + entry_path);
		return out;
	}
	return TranspilePythonToJavascript(LoadFile(entry_path), entry_path);
}

static void PrintUsage()
{
	Cout() <<
		"ScriptWebHost\n"
		"  --port=<n>           Listen port (default 9234)\n"
		"  --ip=<addr>          Listen address (default 127.0.0.1)\n"
		"  --session=<id>       Session identifier\n"
		"  --gamestate=<path>   Target .gamestate file\n"
		"  --root=<path>        Asset/template root\n"
		"  --browser-root=<p>   Browser entry route (default /)\n"
		"  --verbose            Enable Skylark logging\n";
}

int ScriptWebHostMain(const Vector<String>& args)
{
	String error;
	bool show_help = false;
	ScriptWebHostApp app;
	if(!app.ApplyCommandLine(args, error, show_help)) {
		Cerr() << "ScriptWebHost: " << error << "\n";
		PrintUsage();
		return 1;
	}
	if(show_help) {
		PrintUsage();
		return 0;
	}

	if(app.IsVerboseLogging()) {
		StdLogSetup(LOG_FILE|LOG_COUT);
		Ini::skylark_log = true;
	}

	app.Run();
	return 0;
}

END_UPP_NAMESPACE
