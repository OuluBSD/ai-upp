#include "ScriptWebHost.h"

NAMESPACE_UPP

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

static void PrintUsage()
{
	Cout() <<
		"ScriptWebHost\n"
		"  --port=<n>           Listen port (default 9234)\n"
		"  --ip=<addr>          Listen address (default 127.0.0.1)\n"
		"  --session=<id>       Session identifier\n"
		"  --gamestate=<path>   Target .gamestate file\n"
		"  --root=<path>        Asset/template root\n"
		"  --browser-root=<p>   Browser entry route (default /)\n";
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

#ifdef _DEBUG
	StdLogSetup(LOG_FILE|LOG_COUT);
	Ini::skylark_log = true;
#endif

	app.Run();
	return 0;
}

END_UPP_NAMESPACE
