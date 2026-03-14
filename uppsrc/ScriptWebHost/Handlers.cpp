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
	    << "<!DOCTYPE html><html><head><title>ScriptWebHost</title></head><body>"
	    << "<h1>ScriptWebHost</h1>"
	    << "<p>Session: " << app.GetSessionId() << "</p>"
	    << "<p>GameState: " << app.GetGamestatePath() << "</p>"
	    << "<p>Status endpoint: <a href=\"/api/status\">/api/status</a></p>"
	    << "</body></html>";
}

SKYLARK(Status, "api/status")
{
	const ScriptWebHostApp& app = GetScriptWebHostApp(http);
	http.ContentType("application/json; charset=utf-8") << app.GetStatusJson();
}

SKYLARK(CatchAll, "**")
{
	http.Redirect(HomePage);
}

END_UPP_NAMESPACE
