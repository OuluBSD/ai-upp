#ifndef _NodeWorkbench_WorkbenchModel_h_
#define _NodeWorkbench_WorkbenchModel_h_

// WorkbenchModel.h — host-level solution/project/graph model
// No domain-specific logic. Supports .slnx/.grfproj/.grf (new) and
// .nnsln/.nnprj/.nngrf (legacy adapters).

#include <Core/Core.h>

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// Run mode — host-level execution context indicator
// ---------------------------------------------------------------------------
enum class WorkbenchRunMode {
	Testing,    // exploratory / unit-test mode
	Verifying,  // validation / CI mode
	Running,    // production / full-run mode
};

inline const char* RunModeLabel(WorkbenchRunMode m) {
	switch(m) {
	case WorkbenchRunMode::Testing:   return "Testing";
	case WorkbenchRunMode::Verifying: return "Verifying";
	case WorkbenchRunMode::Running:   return "Running";
	}
	return "Testing";
}

// ---------------------------------------------------------------------------
// Diagnostic item — produced by domain validators, shown in the host pane
// ---------------------------------------------------------------------------
enum class DiagSeverity { Info, Warning, Error };

struct WorkbenchDiagnostic : public Moveable<WorkbenchDiagnostic> {
	DiagSeverity severity = DiagSeverity::Info;
	String       message;
	String       entity_id;   // node/edge id, or empty
	String       source;      // e.g. "validator", "compiler"
};

// ---------------------------------------------------------------------------
// WorkbenchGraph — lightweight metadata wrapper (full content lives in
//                  Node::Graph owned by the window; this is the project-model
//                  record that survives without the UI open)
// ---------------------------------------------------------------------------
struct WorkbenchGraph : public Moveable<WorkbenchGraph> {
	int    version      = 1;
	String name;
	String description;
	ValueMap metadata;

	void Jsonize(JsonIO& jio);
	bool Load(const String& path);
	bool Save(const String& path) const;
};

// ---------------------------------------------------------------------------
// WorkbenchProject — analogous to .grfproj / .nnprj
// ---------------------------------------------------------------------------
struct WorkbenchProject : public Moveable<WorkbenchProject> {
	int    version       = 1;
	String name          = "new_project";
	Vector<String> graphs;        // relative paths to .grf files
	String startup_graph;
	ValueMap metadata;

	void Jsonize(JsonIO& jio);
	bool Load(const String& path);
	bool Save(const String& path) const;

	// Legacy adapter: load .nnprj (same JSON schema, different extension)
	bool LoadLegacy(const String& path) { return Load(path); }
};

// ---------------------------------------------------------------------------
// WorkbenchSolution — analogous to .slnx / .nnsln
// ---------------------------------------------------------------------------
struct WorkbenchSolution : public Moveable<WorkbenchSolution> {
	int    version        = 1;
	String name           = "new_solution";
	Vector<String> projects;      // relative paths to .grfproj files
	String active_project;
	ValueMap metadata;

	void Jsonize(JsonIO& jio);
	bool Load(const String& path);
	bool Save(const String& path) const;

	// Legacy adapter: load .nnsln
	bool LoadLegacy(const String& path) { return Load(path); }
};

// ---------------------------------------------------------------------------
// IScriptRuntime — backend-agnostic scripting interface (Task 3.01)
//
// Domains instantiate a concrete backend (e.g. ByteVMScriptRuntime) and
// register it with NodeWorkbenchWindow::SetScriptRuntime().  The host
// passes it back through INodeWorkbenchDomain::OnDomainInit so the domain
// can configure it.  NodeWorkbench itself never includes ByteVM headers.
// ---------------------------------------------------------------------------
struct ScriptRunResult : public Moveable<ScriptRunResult> {
	bool   ok      = false;
	String output;           // stdout / print lines
	String error;            // compile or runtime error message
	String result;           // last expression result as string, may be empty
};

class IScriptRuntime {
public:
	virtual ~IScriptRuntime() {}

	// Identity
	virtual String GetBackendId()   const = 0;  // e.g. "bytevm", "cpython"
	virtual String GetBackendDesc() const = 0;

	// Lifecycle
	virtual bool   Init(String& error_out) = 0;
	virtual void   Reset() = 0;         // clear interpreter state

	// Execution
	// Run source code string; optional filename for error messages.
	virtual ScriptRunResult RunSource(const String& src,
	                                  const String& filename = "inline") = 0;
	// Call a named function (must already be defined by a previous RunSource).
	// args is a JSON-serialisable Value (array or map).
	virtual ScriptRunResult CallFunction(const String& fn_name,
	                                     const Value& args) = 0;

	// Streaming output — set before calling Run/Call.
	// Each print() line fires this event.
	Event<const String&> WhenOutput;
};

// ---------------------------------------------------------------------------
// Extension registry — maps file extensions to kind strings
// ---------------------------------------------------------------------------
struct WorkbenchExtensions {
	static String KindFromPath(const String& path);
	// true if extension is any known solution/project/graph type
	static bool   IsKnownKind(const String& ext);
};

END_UPP_NAMESPACE

#endif
