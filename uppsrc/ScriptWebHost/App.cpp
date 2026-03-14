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
	if(properties) {
		meta.Add("width", FindPropertyValue(*properties, "Form.Width"));
		meta.Add("height", FindPropertyValue(*properties, "Form.Height"));
		meta.Add("name", FindPropertyValue(*properties, "Form.Name"));
		meta.Add("background", FindPropertyValue(*properties, "CardGame.Background"));
	}
	out.Add("meta", meta);

	ValueArray objects;
	if(content) {
		for(int i = 0; i < content->GetCount(); i++) {
			const XmlNode& obj = (*content)[i];
			if(!obj.IsTag("item"))
				continue;

			ValueMap one;
			one.Add("x", obj.AttrInt("x"));
			one.Add("y", obj.AttrInt("y"));
			one.Add("cx", obj.AttrInt("cx"));
			one.Add("cy", obj.AttrInt("cy"));
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
			objects.Add(one);
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
		String layout = AsString(gamestate["layout"]);
		String host = AsString(gamestate["host"]);
		String runtime_host = AsString(gamestate["runtime_host"]);
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
	}

	return AsJSON(m, true);
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
