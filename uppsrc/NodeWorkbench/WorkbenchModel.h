#ifndef _NodeWorkbench_WorkbenchModel_h_
#define _NodeWorkbench_WorkbenchModel_h_

// WorkbenchModel.h — host-level solution/project/graph model
// No domain-specific logic. Supports .slnx/.grfproj/.grf (new) and
// .nnsln/.nnprj/.nngrf (legacy adapters).

#include <Core/Core.h>

NAMESPACE_UPP

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
// Extension registry — maps file extensions to kind strings
// ---------------------------------------------------------------------------
struct WorkbenchExtensions {
	static String KindFromPath(const String& path);
	// true if extension is any known solution/project/graph type
	static bool   IsKnownKind(const String& ext);
};

END_UPP_NAMESPACE

#endif
