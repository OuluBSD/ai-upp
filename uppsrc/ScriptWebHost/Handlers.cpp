#include "ScriptWebHost.h"

NAMESPACE_UPP

static const ScriptWebHostApp& GetScriptWebHostApp(const Http& http)
{
	return static_cast<const ScriptWebHostApp&>(http.App());
}

static String JoinPathArgs(const Http& http)
{
	String path;
	for(int i = 0; i < http.GetParamCount(); i++) {
		if(i)
			path << '/';
		path << http[i];
	}
	return path;
}

static String GuessContentType(const String& file)
{
	String ext = ToLower(GetFileExt(file));
	if(ext == ".css")
		return "text/css; charset=utf-8";
	if(ext == ".js" || ext == ".mjs")
		return "text/javascript; charset=utf-8";
	if(ext == ".json")
		return "application/json; charset=utf-8";
	if(ext == ".svg")
		return "image/svg+xml";
	if(ext == ".png")
		return "image/png";
	if(ext == ".jpg" || ext == ".jpeg")
		return "image/jpeg";
	if(ext == ".gif")
		return "image/gif";
	if(ext == ".webp")
		return "image/webp";
	if(ext == ".form" || ext == ".txt" || ext == ".md" || ext == ".py")
		return "text/plain; charset=utf-8";
	return "application/octet-stream";
}

SKYLARK(HomePage, "")
{
	const ScriptWebHostApp& app = GetScriptWebHostApp(http);
	http.ContentType("text/html; charset=utf-8")
	    << "<!DOCTYPE html><html><head><title>ScriptWebHost</title>"
	    << "<link rel='stylesheet' href='/fs/uppsrc/ScriptWebHost/static/style.css'>"
	    << "</head><body>"
	    << "<div class='shell'>"
	    << "<section class='stage-wrap'>"
	    << "<div class='stage-head'>"
	    << "<div><div class='title'>ScriptWebHost</div><div class='meta'>Session <code>" << app.GetSessionId() << "</code></div></div>"
	    << "<div class='meta' id='stage-title'>" << app.GetGamestatePath() << "</div>"
	    << "</div>"
	    << "<div class='table-frame'><div id='table' class='table'><div id='sprite-layer'></div></div></div>"
	    << "</section>"
	    << "<aside class='side'>"
	    << "<section class='panel'><h2>Status</h2><div id='status' class='status'>Loading browser runtime...</div></section>"
	    << "<section class='panel'><h2>Log</h2><div class='log' id='log'></div></section>"
	    << "<section class='panel'><h2>Bootstrap</h2><div class='legend'>"
	    << "<span class='chip'><a href='/api/status'>/api/status</a></span>"
	    << "<span class='chip'><a href='/api/bootstrap'>/api/bootstrap</a></span>"
	    << "<span class='chip'><a href='/api/transpile-entry.js'>/api/transpile-entry.js</a></span>"
	    << "<span class='chip'>DOM renderer</span><span class='chip'>Python host shim</span></div>"
	    << "<pre id='bootstrap'>loading...</pre></section>"
	    << "</aside></div>"
	    << "<script src='/fs/uppsrc/ScriptWebHost/static/runtime.js'></script></body></html>";
}

SKYLARK(Status, "api/status")
{
	const ScriptWebHostApp& app = GetScriptWebHostApp(http);
	http.ContentType("application/json; charset=utf-8") << app.GetStatusJson();
}

SKYLARK(Bootstrap, "api/bootstrap")
{
	const ScriptWebHostApp& app = GetScriptWebHostApp(http);
	http.ContentType("application/json; charset=utf-8") << app.GetBootstrapJson();
}

SKYLARK(TranspileEntryJs, "api/transpile-entry.js")
{
	const ScriptWebHostApp& app = GetScriptWebHostApp(http);
	PyToJsResult tr = app.GetEntryTranspile();
	http.ContentType("text/javascript; charset=utf-8");
	if(tr.ok)
		http << tr.javascript;
	else {
		http << "// transpilation failed\n";
		for(int i = 0; i < tr.errors.GetCount(); i++)
			http << "// error: " << tr.errors[i] << "\n";
	}
}

SKYLARK(TranspileModuleJs, "api/transpile-module")
{
	const ScriptWebHostApp& app = GetScriptWebHostApp(http);
	String path = http["path"];
	PyToJsResult tr = app.GetModuleTranspile(path);
	http.ContentType("text/javascript; charset=utf-8");
	if(tr.ok)
		http << tr.javascript;
	else {
		http << "// transpilation failed\n";
		for(int i = 0; i < tr.errors.GetCount(); i++)
			http << "// error: " << tr.errors[i] << "\n";
	}
}

SKYLARK(RepoFile, "fs/**")
{
	const ScriptWebHostApp& app = GetScriptWebHostApp(http);
	String rel = JoinPathArgs(http);
	String path = NormalizePath(AppendFileName(app.GetServerRootPath(), rel));
	if(!FileExists(path)) {
		http.Response(404, "Not found");
		return;
	}
	http.ContentType(GuessContentType(path));
	http.SendFile(path, 1 << 16);
}

SKYLARK(CatchAll, "**")
{
	http.Redirect(HomePage);
}

END_UPP_NAMESPACE
