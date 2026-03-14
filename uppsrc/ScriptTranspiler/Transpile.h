#ifndef _ScriptTranspiler_Transpile_h_
#define _ScriptTranspiler_Transpile_h_

struct PyToJsOptions {
	bool emit_prelude = true;
	bool browser_module = true;
};

struct PyToJsResult {
	bool ok = false;
	String javascript;
	Vector<String> warnings;
	Vector<String> errors;

	void Jsonize(JsonIO& json);
};

PyToJsResult TranspilePythonToJavascript(const String& source, const String& filename, const PyToJsOptions& options = PyToJsOptions());

#endif
