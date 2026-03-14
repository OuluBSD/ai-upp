#ifndef _ScriptWebHost_App_h_
#define _ScriptWebHost_App_h_

class ScriptWebHostApp : public SkylarkApp {
	String session_id;
	String gamestate_path;
	String browser_root;
	bool   verbose_logging = false;

public:
	typedef ScriptWebHostApp CLASSNAME;

	ScriptWebHostApp();

	bool   ApplyCommandLine(const Vector<String>& args, String& error, bool& show_help);
	String GetSessionId() const         { return session_id; }
	String GetGamestatePath() const     { return gamestate_path; }
	String GetBrowserRoot() const       { return browser_root; }
	bool   IsVerboseLogging() const     { return verbose_logging; }
	String GetStatusJson() const;
	String GetBootstrapJson() const;
};

int ScriptWebHostMain(const Vector<String>& args);

#endif
