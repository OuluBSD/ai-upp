#ifndef _NodeWorkbench_NodeWorkbench_h_
#define _NodeWorkbench_NodeWorkbench_h_

#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>
#include <Node/Ctrl/Ctrl.h>

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// Domain registration interface
// Implement and pass to NodeWorkbenchWindow::RegisterDomain() to plug in
// domain-specific node types, file extensions and toolbar actions.
// All methods have default no-op implementations so stubs compile as-is.
// ---------------------------------------------------------------------------
class INodeWorkbenchDomain {
public:
	virtual ~INodeWorkbenchDomain() {}

	// Human-readable name shown in the title bar and menus.
	virtual String  GetDomainName() const { return "Generic"; }

	// Called once after the window is fully constructed.
	// Use it to register node types on the viewport, add menu items, etc.
	virtual void    OnDomainInit(class NodeWorkbenchWindow& /*host*/) {}

	// Called when the window is about to close (save domain state here).
	virtual void    OnDomainClose(class NodeWorkbenchWindow& /*host*/) {}

	// Called after a graph file is loaded.
	virtual void    OnGraphLoaded(class NodeWorkbenchWindow& /*host*/,
	                               const String& /*path*/) {}

	// Called before a graph file is saved.
	virtual void    OnGraphSaving(class NodeWorkbenchWindow& /*host*/,
	                               const String& /*path*/) {}

	// Return additional menu entries to add to the domain sub-menu.
	// Bar& is the Bar passed by NodeWorkbenchWindow::MenuDomain().
	virtual void    BuildDomainMenu(Bar& /*bar*/) {}

	// Return file-type filter string used in open/save dialogs, e.g.
	//   "Graph files (*.grf)\t*.grf\nAll files\t*.*"
	// Empty string disables the domain file dialogs.
	virtual String  GetGraphFileFilter() const { return "Graph files (*.grf)\t*.grf"; }
	virtual String  GetProjectFileFilter() const { return "Project files (*.grfproj)\t*.grfproj"; }
	virtual String  GetSolutionFileFilter() const { return "Solution files (*.slnx *.sln)\t*.slnx *.sln"; }
};

// ---------------------------------------------------------------------------
// NodeWorkbenchWindow — generic DockWindow host
// ---------------------------------------------------------------------------
class NodeWorkbenchWindow : public DockWindow {
public:
	typedef NodeWorkbenchWindow CLASSNAME;

	NodeWorkbenchWindow();
	virtual void DockInit() override;
	virtual void Close() override;

	// Register a domain provider. Must be called before the window is opened.
	// Ownership is NOT transferred; the caller must keep the object alive.
	void RegisterDomain(INodeWorkbenchDomain& domain);

	// Access the graph viewport (used by domain providers in OnDomainInit).
	Node::NodeViewportCtrl& GetViewport() { return viewport; }

	// File operations — delegates to domain filter strings.
	bool OpenGraphFile(const String& path);
	bool SaveGraphFile(const String& path);
	bool OpenProjectFile(const String& path);
	bool SaveProjectFile(const String& path);
	bool OpenSolutionFile(const String& path);
	bool SaveSolutionFile(const String& path);

	// Layout file name — override to store a different file per domain.
	virtual String GetLayoutFileName() const;

	// Status bar helper.
	void SetStatus(const String& s);

private:
	// ---- panes ----
	// Solution / project tree
	ParentCtrl    project_panel;
	TreeArrayCtrl solution_tree;
	Button        btn_open_item;
	Button        btn_new_graph;
	Button        btn_new_folder;

	// Node palette
	ParentCtrl  palette_panel;
	Splitter    palette_split;
	ArrayCtrl   category_list;
	ArrayCtrl   node_list;
	Button      btn_add_node;

	// Diagnostics
	ParentCtrl  diagnostics_panel;
	ArrayCtrl   diagnostics_list;
	Button      btn_diag_clear;

	// Graph viewport (central widget, also dockable for multi-graph)
	Node::Graph              graph;
	Node::EditorState        editor;
	Node::HistoryStack       history;
	Node::CommandDispatcher  dispatcher;
	Node::NodeViewportCtrl   viewport;

	// ---- dock handles ----
	DockableCtrl* dock_project     = nullptr;
	DockableCtrl* dock_palette     = nullptr;
	DockableCtrl* dock_diagnostics = nullptr;
	DockableCtrl* dock_viewport    = nullptr;

	// ---- menu/status ----
	MenuBar   menu;
	StatusBar status;

	// ---- domain ----
	INodeWorkbenchDomain* domain = nullptr;

	// ---- state ----
	String current_sln_path;
	String current_prj_path;
	String current_graph_path;

	// ---- menu builders ----
	void MainMenu(Bar& bar);
	void MenuFile(Bar& bar);
	void MenuView(Bar& bar);
	void MenuDomain(Bar& bar);

	// ---- project tree helpers ----
	void RefreshProjectTree();
	void OpenSelectedProjectTreeItem();
	void OnProjectTreeMenu(Bar& bar);

	// ---- palette helpers ----
	void RefreshCategoryList();
	void RefreshNodeList();

	// ---- file I/O internals ----
	void ActionNewGraph();
	void ActionOpenGraph();
	void ActionSaveGraph();
	void ActionOpenProject();
	void ActionOpenSolution();
};

END_UPP_NAMESPACE

#endif
