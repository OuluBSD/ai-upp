#include "ScriptWebHost.h"

NAMESPACE_UPP

static const ScriptWebHostApp& GetScriptWebHostApp(const Http& http)
{
	return static_cast<const ScriptWebHostApp&>(http.App());
}

SKYLARK(HomePage, "")
{
	const ScriptWebHostApp& app = GetScriptWebHostApp(http);
	http.ContentType("text/html; charset=utf-8")
	    << "<!DOCTYPE html><html><head><title>ScriptWebHost</title>"
	    << "<style>body{font-family:sans-serif;max-width:900px;margin:40px auto;padding:0 20px}"
	    << "pre{background:#f5f5f5;padding:16px;border-radius:8px;overflow:auto}"
	    << "code{font-family:monospace}</style></head><body>"
	    << "<h1>ScriptWebHost</h1>"
	    << "<p>Session: <code>" << app.GetSessionId() << "</code></p>"
	    << "<p>GameState: <code>" << app.GetGamestatePath() << "</code></p>"
	    << "<p>Endpoints: <a href=\"/api/status\">/api/status</a> | "
	    << "<a href=\"/api/bootstrap\">/api/bootstrap</a></p>"
	    << "<h2>Bootstrap</h2><pre id=\"bootstrap\">loading...</pre>"
	    << "<script>"
	       "fetch('/api/bootstrap').then(r=>r.text()).then(t=>document.getElementById('bootstrap').textContent=t)"
	       ".catch(e=>document.getElementById('bootstrap').textContent=String(e));"
	    << "</script></body></html>";
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

SKYLARK(CatchAll, "**")
{
	http.Redirect(HomePage);
}

END_UPP_NAMESPACE
