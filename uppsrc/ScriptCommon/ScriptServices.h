#ifndef _ScriptCommon_ScriptServices_h_
#define _ScriptCommon_ScriptServices_h_

struct ScriptRunRequest {
	String path;
	Event<const String&> WhenStdout;
	Event<const String&> WhenStderr;
};

struct ScriptRunResult : Moveable<ScriptRunResult> {
	bool   ok = false;
	bool   runtime_error = false;
	String path;
	String kind;
	String error;
};

struct ScriptLintIssue : Moveable<ScriptLintIssue> {
	int    line = 0;
	int    column = 0;
	String severity;
	String text;
};

struct ScriptLintResult : Moveable<ScriptLintResult> {
	bool                    ok = false;
	String                  path;
	Vector<ScriptLintIssue> issues;
};

struct ScriptPluginInfo : Moveable<ScriptPluginInfo> {
	String id;
	String name;
	String description;
};

struct ScriptPluginTestCaseResult : Moveable<ScriptPluginTestCaseResult> {
	String name;
	bool   ok = false;
	String message;
};

struct ScriptPluginTestResult : Moveable<ScriptPluginTestResult> {
	String                              plugin_id;
	bool                                ok = false;
	int                                 passed = 0;
	int                                 failed = 0;
	String                              error;
	Vector<ScriptPluginTestCaseResult>  results;
};

class ScriptServices {
	String workspace;

public:
	ScriptServices(const String& workspace = String());

	String GetWorkspace() const;
	String ResolvePath(const String& path) const;

	ScriptRunResult        RunFile(const ScriptRunRequest& req) const;
	ScriptLintResult       LintFile(const String& path) const;
	Vector<ScriptPluginInfo> ListPlugins() const;
	ScriptPluginTestResult TestPlugin(const String& plugin_id, const String& case_name = String()) const;
};

#endif
