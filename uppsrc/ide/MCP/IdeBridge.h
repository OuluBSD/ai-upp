#ifndef _ide_MCP_IdeBridge_h_
#define _ide_MCP_IdeBridge_h_

// Thread-safe facade for IDE operations not related to the active debugger.
// All methods post to the GUI thread via RunOnGui() (same pattern as DebugBridge).

class IdeBridge {
public:
	// --- mainconfig ---
	ValueArray  ListMainConfigs() const;   // [{param, name}]
	String      GetMainConfig() const;     // current mainconfigparam
	String      SetMainConfig(const String& param);

	// --- build methods ---
	ValueArray  ListBuildMethods() const;  // [name, ...]
	String      GetBuildMethod() const;
	String      SetBuildMethod(const String& name);

	// --- build mode (debug/release) ---
	String      GetBuildMode() const;      // "debug" or "release"
	String      SetBuildMode(const String& mode); // "debug" | "release"

	// --- packages ---
	ValueArray  ListAssemblyPackages() const;   // all packages in workspace
	String      GetActivePackage() const;
	String      SetActivePackage(const String& name);

	// --- files in active package ---
	ValueArray  ListPackageFiles(const String& pkg) const;
	String      GetActiveFile() const;
	String      SetActiveFile(const String& path);

	// --- editor cursor / content ---
	struct EditorPos { int line = 0; int col = 0; int64 cursor = 0; };
	EditorPos   GetEditorCursor(int editor_idx = 0) const;  // 0=editor, 1=editor2
	String      SetEditorCursor(int line, int col, int editor_idx = 0);
	String      SetEditorCursorPos(int64 pos, int editor_idx = 0);
	int         GetEditorLineCount(int editor_idx = 0) const;
	String      GetEditorLine(int line, int editor_idx = 0) const;
	String      GetEditorPath(int editor_idx = 0) const;
	String      InsertEditorText(const String& text);  // inserts at cursor in editor 0

	// --- console output ---
	String      GetConsoleText() const;       // all console lines
	String      GetConsoleTail(int lines) const; // last N lines

	// --- error list ---
	struct BuildError : Moveable<BuildError> {
		String file; int line = 0; String text; bool is_warning = false;
	};
	Vector<BuildError>  GetErrors() const;
	int                 GetErrorCount() const;
	int                 GetWarningCount() const;

	// --- find in files (opens dialog pre-filled) ---
	String      FindInFiles(const String& pattern, bool replace = false);

	// --- find in editor ---
	String      EditorFindNext();
	String      EditorFindPrev();

	// --- valgrind ---
	String      RunValgrind();

	// --- assist ---
	ValueArray  GetAssistSuggestions() const;  // list at current cursor pos

	// --- assist menu actions ---
	String      AssistGoto();
	String      AssistUsage();
	String      AssistQueryId();
	String      AssistContextGoto();

	// --- workspace / package switching ---
	String      WorkspaceOpen(const String& package);   // SetMain — switch active package
	String      WorkspaceReload();                       // re-SetMain current package
	String      WorkspaceClose();                        // close current, show package selector

	// --- assembly management (no GUI lock needed — reads config files) ---
	struct AssemblyInfo : Moveable<AssemblyInfo> {
		String name;                    // var name (e.g. "default")
		String path;                    // VarFilePath
		Vector<String> upp_dirs;        // UPP assembly paths
		String output_dir;
		String include;
	};
	Vector<AssemblyInfo>  ListAssemblies() const;
	String                GetActiveAssembly() const;    // GetVarsName()
	String                SwitchAssembly(const String& name); // LoadVars + refresh
	String                GetAssemblyPath(const String& name) const; // VarFilePath(name)

	// --- package listing per assembly path ---
	// Returns packages in the given assembly (or current if empty).
	// filter: "main" | "nonmain" | "all" (default "all")
	// search: substring filter on package name
	ValueArray  ListPackagesInAssembly(const String& vars_name,
	                                   const String& filter,
	                                   const String& search) const;

	// --- UppHub ---
	String      OpenUppHub();   // opens UppHub dialog on GUI thread

private:
	bool RunOnGui(Function<void()> fn, int timeout_ms = 8000) const;
};

extern IdeBridge sIdeBridge;

#endif
