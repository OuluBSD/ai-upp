#ifndef _NodeWorkbench_NodeWorkbench_h_
#define _NodeWorkbench_NodeWorkbench_h_

#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>
#include <Node/Ctrl/Ctrl.h>

#include "WorkbenchModel.h"

NAMESPACE_UPP

// forward
class NodeWorkbenchWindow;

// ---------------------------------------------------------------------------
// INodeWorkbenchDomain  (Task 02)
//
// Implement this interface and pass to NodeWorkbenchWindow::RegisterDomain()
// to plug in domain-specific behaviour.  All methods have default no-op
// implementations so a minimal domain compiles with no overrides.
// ---------------------------------------------------------------------------
class INodeWorkbenchDomain {
public:
	virtual ~INodeWorkbenchDomain() {}

	// --- Identity ---
	// Stable machine-readable identifier, e.g. "neural", "electric", "generic".
	virtual String GetDomainId()   const { return "generic"; }
	// Human-readable display name (used in title bar, menus).
	virtual String GetDomainName() const { return "Generic"; }
	// One-line description shown in future domain-picker UI.
	virtual String GetDomainDesc() const { return ""; }

	// --- Lifecycle ---
	// Called once after DockInit() completes.  Register node types on the
	// viewport here; read domain-specific config; prime the palette.
	virtual void OnDomainInit(NodeWorkbenchWindow& /*host*/) {}
	// Called before the window closes.
	virtual void OnDomainClose(NodeWorkbenchWindow& /*host*/) {}

	// --- File events ---
	// Called after the host has loaded / is about to save a graph file.
	virtual void OnGraphLoaded(NodeWorkbenchWindow& /*host*/,
	                           const String& /*path*/) {}
	virtual void OnGraphSaving(NodeWorkbenchWindow& /*host*/,
	                           const String& /*path*/) {}
	// Called after a project/solution is opened so the domain can
	// e.g. prime a compilation cache.
	virtual void OnProjectOpened(NodeWorkbenchWindow& /*host*/,
	                             const WorkbenchProject& /*prj*/,
	                             const String& /*path*/) {}
	virtual void OnSolutionOpened(NodeWorkbenchWindow& /*host*/,
	                              const WorkbenchSolution& /*sln*/,
	                              const String& /*path*/) {}

	// --- Palette seeding ---
	// Called by host during DockInit (after OnDomainInit).  Fill palette_out
	// with category → node-label pairs; the host populates the palette lists.
	struct PaletteItem : public Moveable<PaletteItem> {
		String category;
		String label;
		String type_id;    // passed back to viewport.RegisterNodeType
	};
	virtual void BuildPalette(Vector<PaletteItem>& /*palette_out*/) {}

	// --- Validation / diagnostics ---
	// Called when the user triggers "Verify" or the host auto-validates.
	// Append diagnostic items to diag_out.
	virtual void ValidateGraph(NodeWorkbenchWindow& /*host*/,
	                           Vector<WorkbenchDiagnostic>& /*diag_out*/) {}

	// --- Compile / run ---
	// Called by the host "Run" / "Compile" menu items.  Return false on error;
	// write human-readable output to log_out.
	virtual bool CompileGraph(NodeWorkbenchWindow& /*host*/,
	                          String& /*log_out*/) { return true; }
	virtual bool RunGraph(NodeWorkbenchWindow& /*host*/,
	                      String& /*log_out*/) { return true; }

	// --- Quick-fix provider ---
	// Return a list of short fix labels applicable to the selected diagnostic.
	// The host calls ApplyQuickFix(index) when the user clicks one.
	virtual Vector<String> GetQuickFixes(const WorkbenchDiagnostic& /*diag*/) {
		return {};
	}
	virtual void ApplyQuickFix(NodeWorkbenchWindow& /*host*/,
	                           const WorkbenchDiagnostic& /*diag*/,
	                           int /*fix_index*/) {}

	// --- Template generation ---
	// Called by host "New from Template..." dialog.  Fill out with template
	// descriptors.  GenerateTemplate() is then called with the chosen index
	// and a user-chosen destination directory; it should create graph/project/
	// solution files there and return the primary solution path (or graph path
	// if no solution is created).  Return empty string on failure.
	struct TemplateDesc : public Moveable<TemplateDesc> {
		String name;         // display name, e.g. "Classify 2D"
		String category;     // e.g. "neural.classification"
		String description;  // one-line description shown in picker
	};
	virtual void GetTemplates(Vector<TemplateDesc>& /*out*/) {}
	virtual String GenerateTemplate(int /*index*/, const String& /*dest_dir*/,
	                                String& /*error_out*/) { return String(); }

	// --- Menus ---
	// Extra entries appended to the domain sub-menu in the menu bar.
	virtual void BuildDomainMenu(Bar& /*bar*/) {}

	// --- File-type metadata ---
	// File-open/save filter strings (Tab-separated label\textension).
	virtual String GetGraphFileFilter()    const { return "Graph files (*.grf)\t*.grf"; }
	virtual String GetProjectFileFilter()  const { return "Project files (*.grfproj)\t*.grfproj"; }
	virtual String GetSolutionFileFilter() const { return "Solution files (*.slnx *.sln)\t*.slnx *.sln"; }

	// Extra extensions this domain owns (used by OpenPath auto-dispatch).
	// Return pipe-separated list, e.g. ".myext|.myext2".
	virtual String GetExtraExtensions() const { return ""; }
};

// ---------------------------------------------------------------------------
// DomainRegistry  (compile-time plugin registration)
//
// Domains self-register via a static DomainRegistry::Entry at file scope.
// The host can enumerate registered domains to build pickers or auto-dispatch.
//
// Usage:
//   static DomainRegistry::Entry s_reg([] { return new MyDomain; });
// ---------------------------------------------------------------------------
class DomainRegistry {
public:
	typedef Function<INodeWorkbenchDomain*()> Factory;

	struct Entry {
		Entry(Factory f);
	};

	static int                    GetCount();
	static INodeWorkbenchDomain*  Create(int i);   // caller owns the pointer
	static INodeWorkbenchDomain*  CreateById(const String& id); // nullptr if not found
};

// ---------------------------------------------------------------------------
// NodeWorkbenchWindow  (Tasks 02 + 03)
//
// Generic DockWindow host for graph-based domains.
// ---------------------------------------------------------------------------
class NodeWorkbenchWindow : public DockWindow {
public:
	typedef NodeWorkbenchWindow CLASSNAME;

	NodeWorkbenchWindow();
	virtual void DockInit() override;
	virtual void Close() override;

	// Register a domain provider.  Must be called before the window is opened.
	// Ownership is NOT transferred; caller must keep the object alive.
	void RegisterDomain(INodeWorkbenchDomain& domain);

	// Open any known file by path — auto-dispatches by extension.
	// Returns false if the extension is not recognised or load fails.
	bool OpenPath(const String& path);

	// Fine-grained file ops (used by menus and OpenPath).
	bool OpenGraphFile(const String& path);
	bool SaveGraphFile(const String& path);
	bool OpenProjectFile(const String& path);
	bool SaveProjectFile(const String& path);
	bool OpenSolutionFile(const String& path);
	bool SaveSolutionFile(const String& path);

	// Save-as variants (ask user for path).
	void SaveGraphAs();
	void SaveProjectAs();
	void SaveSolutionAs();

	// Trigger domain validate + refresh diagnostics pane.
	void ValidateGraph();

	// Trigger domain compile/run; result shown in diagnostics.
	void CompileGraph();
	void RunGraph();

	// Surface diagnostics from external source (e.g. file watcher).
	void SetDiagnostics(const Vector<WorkbenchDiagnostic>& diags);

	// Script runtime — optional; set by domain in OnDomainInit.
	// Ownership is NOT transferred; caller keeps the object alive.
	void              SetScriptRuntime(IScriptRuntime& rt) { script_runtime = &rt; }
	IScriptRuntime*   GetScriptRuntime()                   { return script_runtime; }

	// Read-only accessors used by domain OnDomainInit / validators.
	Node::NodeViewportCtrl&  GetViewport()  { return viewport; }
	Node::Graph&             GetGraph()     { return graph; }
	const Node::Graph&       GetGraph() const { return graph; }
	const WorkbenchSolution& GetSolution()  const { return sln; }
	const WorkbenchProject&  GetProject()   const { return prj; }
	String                   GetSolutionPath() const { return current_sln_path; }
	String                   GetProjectPath()  const { return current_prj_path; }
	String                   GetGraphPath()    const { return current_graph_path; }

	// Per-domain layout persistence file name.
	virtual String GetLayoutFileName() const;

	// Status bar helper.
	void SetStatus(const String& s);

private:
	// ---- panes ----
	ParentCtrl    project_panel;
	TreeArrayCtrl solution_tree;
	Button        btn_open_item;
	Button        btn_new_graph;
	Button        btn_new_folder;

	ParentCtrl  palette_panel;
	Splitter    palette_split;
	ArrayCtrl   category_list;
	ArrayCtrl   node_list;
	Button      btn_add_node;

	ParentCtrl  diagnostics_panel;
	ArrayCtrl   diagnostics_list;
	Button      btn_diag_clear;
	Button      btn_diag_verify;

	Node::Graph              graph;
	Node::EditorState        editor;
	Node::HistoryStack       history;
	Node::CommandDispatcher  dispatcher;
	Node::NodeViewportCtrl   viewport;

	// ---- dock handles ----
	DockableCtrl* dock_project     = nullptr;
	DockableCtrl* dock_palette     = nullptr;
	DockableCtrl* dock_diagnostics = nullptr;

	// ---- menu/status ----
	MenuBar   menu;
	StatusBar status;

	// ---- domain ----
	INodeWorkbenchDomain* domain         = nullptr;
	IScriptRuntime*       script_runtime = nullptr;

	// ---- project model (Task 03) ----
	WorkbenchSolution sln;
	WorkbenchProject  prj;
	String current_sln_path;
	String current_prj_path;
	String current_graph_path;

	// last diagnostics shown
	Vector<WorkbenchDiagnostic> last_diags;

	// ---- menu builders ----
	void MainMenu(Bar& bar);
	void MenuFile(Bar& bar);
	void MenuRun(Bar& bar);
	void MenuView(Bar& bar);
	void MenuDomain(Bar& bar);

	// ---- palette ----
	void RebuildPalette();
	void RefreshCategoryList();
	void RefreshNodeList();

	// ---- project tree ----
	void RefreshProjectTree();
	void OpenSelectedProjectTreeItem();
	void OnProjectTreeMenu(Bar& bar);
	void ActionAddProjectToSolution();
	void ActionAddGraphToProject();
	void ActionSetStartupGraph();
	void ActionRemoveTreeItem();
	void ActionRenameTreeItem();

	// ---- diagnostics pane ----
	void RefreshDiagnosticsPane();

	// ---- file I/O internals ----
	void ActionNewGraph();
	void ActionOpenGraph();
	void ActionSaveGraph();
	void ActionNewProject();
	void ActionOpenProject();
	void ActionNewSolution();
	void ActionOpenSolution();
	void ActionNewFromTemplate();
};

END_UPP_NAMESPACE

#endif
