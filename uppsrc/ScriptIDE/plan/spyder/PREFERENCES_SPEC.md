# PREFERENCES_SPEC.md

## Migration Note (ScriptCommon Split, 2026-03-10)

This spec remains valid for GUI behavior, but ownership is split:
- `IDESettings` schema/data moves to `uppsrc/ScriptCommon`.
- Preferences widgets/pages remain in `uppsrc/ScriptIDE`.
- Runtime services referenced by pages (e.g., run/path/plugin core) should be accessed through ScriptIDE adapters over ScriptCommon.

# Preferences Windows Specification

## UI structure, data model, runtime bindings and U++ implementation notes

This document specifies the **Preferences** system for the Spyder-like
IDE, adapted to Ultimate++ (`DockWindow`, `DockableCtrl`, `TopWindow`,
`CtrlLib` widgets) and the custom ByteVM-based Python frontend.

It covers:

-   exact preferences navigation structure captured from the UI
-   expected data model behind each page
-   how settings should bind to runtime systems
-   how settings should update GUI state
-   recommended U++ implementation structure

This is both a **GUI specification** and an **internal wiring
specification**.

------------------------------------------------------------------------

# 1. Preferences Dialog Overview

The Preferences window is a modal or modeless `TopWindow` containing:

-   left navigation panel
-   right content panel
-   page-local tabs where needed
-   bottom action buttons

Recommended U++ base class:

``` cpp
class PreferencesWindow : public TopWindow {
public:
    typedef PreferencesWindow CLASSNAME;

    Splitter main_split;
    ArrayCtrl nav;
    ParentCtrl page_host;

    Button reset_defaults;
    Button ok;
    Button cancel;
    Button apply;

    void LoadFromSettings();
    void SaveToSettings();
    void ApplyToRuntime();
};
```

------------------------------------------------------------------------

# 2. Exact Navigation Tree

Captured navigation sections:

``` text
Preferences
├─ Appearance
├─ Application
├─ Python interpreter
├─ Keyboard shortcuts
├─ Code Analysis
├─ Completion and linting
├─ Debugger
├─ Editor
├─ Files
├─ Help
├─ History
├─ IPython console
├─ Profiler
├─ Run
├─ Status bar
├─ Variable explorer
├─ Working directory
└─ Plugins
```

Each entry should map to:

-   one settings page class
-   one settings storage section
-   one apply/reset strategy
-   one owning subsystem

Recommended registry pattern:

``` cpp
struct PrefPageSpec {
    String id;
    String title;
    Image icon;
    PreferencesPage* page;
};
```

------------------------------------------------------------------------

# 3. Global Settings Object

Recommended master configuration object:

``` cpp
struct IDESettings {
    AppearanceSettings appearance;
    ApplicationSettings application;
    PythonInterpreterSettings python;
    ShortcutSettings shortcuts;
    CodeAnalysisSettings code_analysis;
    CompletionLintingSettings completion;
    DebuggerSettings debugger;
    EditorSettings editor;
    FilesSettings files;
    HelpSettings help;
    HistorySettings history;
    IPythonConsoleSettings console;
    ProfilerSettings profiler;
    RunSettings run;
    StatusBarSettings statusbar;
    VariableExplorerSettings variable_explorer;
    WorkingDirectorySettings working_directory;
    PluginSettings plugins;
};
```

Persistence options in U++:

-   `LoadFromJsonFile / StoreAsJsonFile`
-   `ConfigFile(...)`
-   or a custom `.cfg` / `.json`

Recommended path:

``` text
GetConfigFolder() / ide_settings.json
```

------------------------------------------------------------------------

# 4. Page Base Class

All pages should inherit from a common base:

``` cpp
class PreferencesPage : public ParentCtrl {
public:
    virtual void Load(const IDESettings& cfg) = 0;
    virtual void Save(IDESettings& cfg) const = 0;
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) = 0;
    virtual void SetDefaults() = 0;
    virtual bool IsModified() const = 0;
};
```

Why this matters:

-   **Load** populates controls from current config
-   **Save** writes UI state back into config
-   **Apply** propagates config changes into live runtime / GUI
-   **SetDefaults** restores page defaults
-   **IsModified** enables smart Apply handling

Without this, settings devolve into scattered callback folklore.

------------------------------------------------------------------------

# 5. IDE Context for Apply()

Recommended live context object:

``` cpp
struct IDEContext {
    IDEWindow* main_window = nullptr;
    EditorManager* editor = nullptr;
    PaneManager* panes = nullptr;
    ToolbarManager* toolbars = nullptr;
    LayoutManager* layouts = nullptr;
    ByteVMInterface* runtime = nullptr;
    RunManager* run = nullptr;
    DebugManager* debug = nullptr;
    ProfilerManager* profiler = nullptr;
    SearchEngine* search = nullptr;
    PythonPathService* pythonpath = nullptr;
    EnvironmentService* environment = nullptr;
    RemoteConnectionManager* remote = nullptr;
    LintManager* lint = nullptr;
    CompletionManager* completion = nullptr;
    StatusBarController* statusbar = nullptr;
    PluginManager* plugins = nullptr;
};
```

This gives each settings page a controlled way to apply changes.

------------------------------------------------------------------------

# 6. Appearance Page

## Captured UI

``` text
Appearance
├─ Main interface
│  └─ Interface theme dropdown
├─ Syntax highlighting theme
│  ├─ Theme dropdown
│  ├─ Edit selected theme
│  ├─ Reset to defaults
│  ├─ Create new theme
│  └─ Delete theme
├─ Fonts
│  ├─ Monospace font + size
│  ├─ Interface font + size
│  └─ Use system default interface font
└─ Previews
   ├─ Editor preview
   └─ Interface font preview
```

## Data model

``` cpp
struct AppearanceSettings {
    String interface_theme;
    String syntax_theme;
    String monospace_font_face;
    int    monospace_font_size = 10;
    String interface_font_face;
    int    interface_font_size = 10;
    bool   use_system_interface_font = true;
};
```

## Apply behavior

Connects to:

-   main window palette / theme manager
-   all editors (`CodeEditor`)
-   preview widgets
-   pane fonts
-   menu/toolbars/status bar

``` cpp
void AppearancePage::Apply(IDEContext& ctx,
                           const IDESettings& old_cfg,
                           const IDESettings& new_cfg)
{
    ctx.main_window->ApplyTheme(new_cfg.appearance.interface_theme);
    ctx.editor->ApplyEditorTheme(new_cfg.appearance.syntax_theme);
    ctx.editor->ApplyEditorFont(new_cfg.appearance.monospace_font_face,
                                new_cfg.appearance.monospace_font_size);
    ctx.main_window->ApplyInterfaceFont(...);
}
```

Some changes may require full redraw; theme changes often do.

------------------------------------------------------------------------

# 7. Application Page

## Captured UI

Tabs:

``` text
Application
├─ Interface
│  ├─ HiDPI scaling options
│  ├─ Friendly empty pane message
│  ├─ Vertical tabs in panes
│  ├─ Custom pane margin
│  └─ Cursor blinking
└─ Advanced settings
   ├─ Language
   ├─ Rendering engine
   ├─ Use single instance
   ├─ Prompt when exiting
   ├─ Show internal errors
   ├─ Check for updates on startup
   ├─ Stable releases only
   └─ Disable zoom with Ctrl/Cmd + wheel
```

## Data model

``` cpp
struct ApplicationSettings {
    int  hidpi_mode = 0; // 0 normal, 1 auto, 2 custom
    double custom_scale = 1.0;

    bool show_friendly_empty_messages = true;
    bool vertical_tabs_in_panes = false;
    int pane_margin = 0;
    bool custom_cursor_blink = false;
    int cursor_blink_ms = 1000;

    String language = "English";
    String rendering_engine = "Default";
    bool single_instance = true;
    bool prompt_on_exit = false;
    bool show_internal_errors = true;
    bool check_updates_on_startup = true;
    bool stable_releases_only = true;
    bool disable_ctrl_wheel_zoom = false;
};
```

## Apply behavior

Connects to:

-   main window global UI policy
-   docking/tab appearance
-   editor wheel zoom behavior
-   app startup policy
-   error reporting policy

Some fields are **restart-sensitive**, especially rendering engine and
language.

Recommended pattern:

``` cpp
enum ApplyRequirement {
    APPLY_LIVE,
    APPLY_REOPEN_WINDOW,
    APPLY_RESTART_APP
};
```

Each setting should declare its apply cost.

------------------------------------------------------------------------

# 8. Python Interpreter Page

## Captured UI

``` text
Python interpreter
├─ Internal interpreter
├─ Selected interpreter path
├─ Custom Conda/Mamba/Micromamba executable
└─ User Module Reloader (UMR)
   ├─ Enable UMR
   ├─ Print list of reloaded modules
   └─ Select excluded modules
```

## Data model

``` cpp
struct PythonInterpreterSettings {
    bool use_internal = true;
    String interpreter_path;
    bool use_custom_conda = false;
    String conda_executable;

    bool umr_enabled = true;
    bool umr_verbose = true;
    Vector<String> umr_excluded_modules;
};
```

## Internal connections

This page binds to:

-   `ByteVMInterface`
-   console creation
-   code completion backend environment
-   run/debug/profile environment selection
-   import reload policy

## Apply behavior

-   changing interpreter affects **new consoles**, not necessarily
    running ones
-   UMR settings affect run pipeline before execution

``` cpp
ctx.runtime->SetInterpreterConfig(...);
ctx.run->SetUMRConfig(...);
ctx.completion->SetPythonEnvironment(...);
```

------------------------------------------------------------------------

# 9. Keyboard Shortcuts Page

## Captured UI

-   shortcuts table
-   search field

## Data model

``` cpp
struct ShortcutItem {
    String context;
    String action_id;
    dword key = 0;
};

struct ShortcutSettings {
    Vector<ShortcutItem> items;
};
```

## Internal connections

Every action in the IDE should have a stable ID:

``` cpp
enum ActionID {
    ACT_FILE_NEW,
    ACT_FILE_OPEN,
    ACT_RUN_FILE,
    ACT_DEBUG_STEP_INTO,
    ...
};
```

Or string IDs:

``` cpp
"file.new"
"run.file"
"debug.step_into"
```

Recommended action registry:

``` cpp
class ActionRegistry {
public:
    void Register(const String& id, Callback cb, dword default_key);
    void SetKey(const String& id, dword key);
    dword GetKey(const String& id) const;
};
```

## Apply behavior

-   updates menu accelerator text
-   updates toolbar shortcuts
-   updates editor-level shortcuts
-   persists custom bindings

This page must connect directly to the central action system.\
No action registry, no civilized shortcuts.

------------------------------------------------------------------------

# 10. Code Analysis Page

## Captured UI

``` text
Code Analysis
├─ Save file before analyzing
├─ History result count
└─ Results storage path
```

## Data model

``` cpp
struct CodeAnalysisSettings {
    bool save_before_analysis = true;
    int history_results = 30;
    String results_path;
};
```

## Internal connections

Binds to:

-   static analysis command pipeline
-   code-analysis pane / result store
-   lint history manager

## Apply behavior

``` cpp
ctx.lint->SetHistoryLimit(new_cfg.code_analysis.history_results);
ctx.lint->SetSaveBeforeAnalysis(new_cfg.code_analysis.save_before_analysis);
```

------------------------------------------------------------------------

# 11. Completion and Linting Page

## Captured UI tabs

``` text
Completion and linting
├─ General
├─ Linting
├─ Introspection
├─ Code formatting
├─ Advanced
├─ Other languages
└─ Snippets
```

## Data model

``` cpp
struct CompletionLintingSettings {
    bool show_completion_details = true;
    bool enable_code_snippets = true;
    bool show_completions_on_the_fly = true;
    int chars_before_completion = 1;
    int completion_detail_delay_ms = 500;
    int provider_timeout_ms = 200;

    bool enable_fallback_provider = true;
    bool enable_lsp_provider = true;
    bool enable_snippet_provider = true;

    String lint_provider = "pyflakes"; // pyflakes, flake8, ruff, none
    bool underline_errors = true;

    bool enable_go_to_definition = true;
    bool follow_imports = true;
    bool show_calltips = true;
    bool enable_hover_hints = true;
    String preload_modules_csv;

    String formatter = "autopep8"; // or black
    bool autoformat_on_save = false;
    int max_line_length = 79;
    bool show_vertical_ruler = true;

    bool lsp_advanced_enabled = false;
    String lsp_module = "pylsp";
    String lsp_address = "127.0.0.1";
    int lsp_port = 2087;
    bool lsp_external_server = false;
    bool lsp_use_stdio = false;

    struct ExternalLSPServer {
        String language;
        String address;
        String command;
    };
    Vector<ExternalLSPServer> external_servers;

    struct Snippet {
        String language;
        String trigger;
        String description;
        String body;
    };
    Vector<Snippet> snippets;
};
```

## Internal connections

This page binds to:

-   completion engine
-   lint manager
-   symbol provider
-   formatter adapter
-   LSP client manager
-   snippet engine
-   editor UX

## GUI connections

Changes influence:

-   editor context menu commands
-   underline rendering
-   calltips
-   go-to-definition
-   hover hints
-   autoformat on save
-   snippet availability

## Apply behavior

``` cpp
ctx.completion->ConfigureProviders(...);
ctx.lint->SetProvider(...);
ctx.editor->SetAutoFormatOnSave(...);
ctx.editor->SetVerticalRuler(...);
ctx.completion->ReloadLanguageServers(...);
```

Some LSP changes may require restarting language servers.

------------------------------------------------------------------------

# 12. Debugger Page

## Captured UI

``` text
Debugger
├─ Prevent editor from closing files while debugging
├─ Stop debugging on first line of files without breakpoints
├─ Ignore Python libraries while debugging
├─ Process execute events while debugging
├─ Use exclamation mark prefix for Pdb commands
├─ Run code while debugging: Lines
└─ Exclude internal frames when inspecting execution
```

## Data model

``` cpp
struct DebuggerSettings {
    bool prevent_close_while_debugging = true;
    bool stop_on_first_line_without_breakpoints = true;
    bool ignore_python_libraries = false;
    bool process_execute_events = true;
    bool use_exclamation_prefix = true;
    String preload_debug_lines;
    bool exclude_internal_frames = true;
};
```

## Internal connections

Binds to:

-   `DebugManager`
-   stack-frame filtering
-   console debugger command parsing
-   editor close policy
-   debugger stepping strategy

## Apply behavior

``` cpp
ctx.debug->SetFrameFilter(...);
ctx.debug->SetPdbCommandPolicy(...);
ctx.editor->SetPreventCloseWhileDebugging(...);
```

------------------------------------------------------------------------

# 13. Editor Page

## Captured tabs

``` text
Editor
├─ Display
├─ Source code
└─ Advanced
```

## Data model

``` cpp
struct EditorSettings {
    bool show_tab_bar = true;
    bool show_full_path_above_editor = true;
    bool show_class_function_selector = false;
    bool allow_scroll_past_eof = false;

    bool show_indent_guides = false;
    bool show_code_folding = true;
    bool show_line_numbers = true;
    bool show_debugger_breakpoints = true;
    bool show_code_annotations = true;

    bool highlight_current_line = true;
    bool highlight_current_cell = true;
    bool highlight_selected_occurrences = true;
    int occurrence_highlight_delay_ms = 1500;

    bool auto_insert_closing_brackets = true;
    bool auto_insert_closing_quotes = true;
    bool auto_insert_colons = true;
    bool auto_unindent_keywords = true;

    bool strip_trailing_spaces_on_save = false;
    bool strip_trailing_spaces_changed_lines = false;
    bool add_missing_newline_eof = false;
    bool strip_blank_lines_eof = false;

    int tab_width = 4;
    bool indent_with_spaces = true;
    bool intelligent_backspace = true;
    bool tab_always_indents = false;

    bool fix_mixed_eol = true;
    bool convert_eol_on_save = false;
    String eol_mode = "LF";

    String new_file_template;
    bool autosave_backup_unsaved = true;
    int autosave_interval_sec = 60;

    String docstring_style = "Numpy";
    bool enable_multicursor = true;
    int multicursor_paste_mode = 1;

    dword mouse_modifier_multicursor = 0;
};
```

## Internal connections

This is one of the deepest pages. It binds to:

-   `CodeEditor`
-   document save pipeline
-   autosave service
-   docstring generator
-   editor tab UI
-   breakpoint gutter
-   symbol selector
-   multi-cursor logic

## Apply behavior

``` cpp
ctx.editor->ApplyDisplaySettings(...);
ctx.editor->ApplyIndentationSettings(...);
ctx.editor->ApplySavePipelineSettings(...);
ctx.editor->ApplyMulticursorSettings(...);
ctx.editor->SetNewFileTemplate(...);
```

This page has extensive **live apply** support.

------------------------------------------------------------------------

# 14. Files Page

## Captured tabs

``` text
Files
├─ General
└─ File associations
```

## Data model

``` cpp
struct FileAssociation {
    String extension;
    Vector<String> applications;
    int default_index = -1;
};

struct FilesSettings {
    bool show_hidden_files = false;
    bool single_click_open = false;
    String filter_patterns_csv;
    Vector<FileAssociation> associations;
};
```

## Internal connections

Binds to:

-   Files pane tree filtering
-   file-open routing
-   external application launching
-   project tree open behavior

## Apply behavior

``` cpp
ctx.panes->GetFilesPane()->SetShowHiddenFiles(...);
ctx.panes->GetFilesPane()->SetFilterPatterns(...);
ctx.main_window->GetFileOpenRouter()->SetAssociations(...);
```

------------------------------------------------------------------------

# 15. Help Page

## Captured UI

``` text
Help
├─ Automatic connections
│  ├─ Editor
│  └─ IPython Console
├─ Render mathematical equations
└─ Wrap lines
```

## Data model

``` cpp
struct HelpSettings {
    bool auto_connect_editor = false;
    bool auto_connect_console = false;
    bool render_math = true;
    bool wrap_lines = true;
};
```

## Internal connections

Binds to:

-   Help pane auto-inspection
-   object inspection hooks
-   rich/plain rendering
-   source view wrapping

## Apply behavior

``` cpp
ctx.panes->GetHelpPane()->SetAutoConnectEditor(...);
ctx.panes->GetHelpPane()->SetAutoConnectConsole(...);
ctx.panes->GetHelpPane()->SetRenderMath(...);
ctx.panes->GetHelpPane()->SetWrapLines(...);
```

------------------------------------------------------------------------

# 16. History Page

## Captured UI

``` text
History
├─ Wrap lines
├─ Show line numbers
└─ Scroll automatically to last entry
```

## Data model

``` cpp
struct HistorySettings {
    bool wrap_lines = true;
    bool show_line_numbers = false;
    bool scroll_to_last_entry = true;
};
```

## Internal connections

Binds to:

-   History pane rendering
-   history append behavior

## Apply behavior

``` cpp
ctx.panes->GetHistoryPane()->SetWrapLines(...);
ctx.panes->GetHistoryPane()->SetLineNumbers(...);
ctx.panes->GetHistoryPane()->SetAutoScroll(...);
```

------------------------------------------------------------------------

# 17. IPython Console Page

## Captured tabs

``` text
IPython console
├─ Interface
├─ Plotting
├─ Startup
└─ Advanced
```

## Data model

``` cpp
struct IPythonConsoleSettings {
    bool show_welcome_message = true;
    bool show_calltips = true;
    bool show_elapsed_time = false;

    bool confirm_before_closing = false;
    bool confirm_before_restarting = true;
    bool confirm_before_removing_variables = true;

    String completion_display = "Graphical";
    bool use_jedi_completion = false;
    bool use_greedy_completion = false;

    int output_buffer_lines = 5000;
    bool render_sympy_math = false;

    bool matplotlib_support = true;
    bool auto_import_numpy_matplotlib = false;
    String graphics_backend = "Inline";

    String inline_format = "PNG";
    double inline_resolution_dpi = 144.0;
    int inline_width_in = 6;
    int inline_height_in = 4;
    double inline_font_points = 10.0;
    double inline_bottom_edge = 0.11;
    bool inline_tight_layout = true;

    String startup_code;
    bool execute_startup_file = false;
    String startup_file;

    String autocall_mode = "Off";
    bool use_autoreload = true;
    String input_prompt;
    String output_prompt;
    bool hide_subprocess_windows = true;
};
```

## Internal connections

Binds to:

-   console creation defaults
-   kernel startup configuration
-   plotting backend bridge
-   prompt renderer
-   output ring buffer
-   startup script injection

## Apply behavior

Mostly affects **new consoles**, but some display settings can be
applied live to existing consoles.

``` cpp
ctx.runtime->SetConsoleDefaults(...);
ctx.panes->GetConsolePane()->ApplyDisplayDefaults(...);
ctx.panes->GetPlotsPane()->SetInlinePlotDefaults(...);
```

------------------------------------------------------------------------

# 18. Profiler Page

## Captured UI

``` text
Profiler
├─ Open profiler when profiling finishes
└─ Maximum number of items displayed with large local time
```

## Data model

``` cpp
struct ProfilerSettings {
    bool open_on_finish = true;
    int max_hot_items = 15;
};
```

## Internal connections

Binds to:

-   profiler completion behavior
-   profiler pane rendering filters

## Apply behavior

``` cpp
ctx.profiler->SetAutoOpenOnFinish(...);
ctx.panes->GetProfilerPane()->SetHotItemLimit(...);
```

------------------------------------------------------------------------

# 19. Run Page

## Captured UI

``` text
Run
├─ Runner dropdown
├─ Configuration presets table
├─ Add / Edit / Delete / Duplicate / Reset
└─ Editor interactions
   ├─ Save all files before running script
   └─ Copy full cell contents to console when running code cells
```

## Data model

``` cpp
struct RunPreset {
    String name;
    String file_extension;
    String context;
    // plus execution config fields
};

struct RunSettings {
    Vector<RunPreset> presets;
    bool save_all_before_run = true;
    bool copy_full_cell_to_console = false;
};
```

## Internal connections

Binds to:

-   run configuration dialog
-   run/profile/debug preset registry
-   cell execution strategy

## Apply behavior

``` cpp
ctx.run->SetPresets(...);
ctx.run->SetSaveAllBeforeRun(...);
ctx.run->SetCopyFullCellToConsole(...);
```

------------------------------------------------------------------------

# 20. Status Bar Page

## Captured UI

``` text
Status bar
├─ Show memory usage every N ms
├─ Show CPU usage every N ms
└─ Show clock
```

## Data model

``` cpp
struct StatusBarSettings {
    bool show_memory = true;
    int memory_poll_ms = 2000;
    bool show_cpu = false;
    int cpu_poll_ms = 2000;
    bool show_clock = false;
};
```

## Internal connections

Binds to:

-   `StatusBarController`
-   periodic timers
-   dock/status widgets

## Apply behavior

``` cpp
ctx.statusbar->SetMemoryVisible(...);
ctx.statusbar->SetMemoryPollMs(...);
ctx.statusbar->SetCpuVisible(...);
ctx.statusbar->SetClockVisible(...);
```

------------------------------------------------------------------------

# 21. Variable Explorer Page

## Captured UI

``` text
Variable explorer
├─ Exclude private references
├─ Exclude capitalized references
├─ Exclude all-uppercase references
├─ Exclude unsupported data types
├─ Exclude callables and modules
└─ Show arrays min/max
```

## Data model

``` cpp
struct VariableExplorerSettings {
    bool exclude_private = true;
    bool exclude_capitalized = false;
    bool exclude_uppercase = false;
    bool exclude_unsupported = false;
    bool exclude_callables = true;
    bool show_array_minmax = false;
};
```

## Internal connections

Binds to:

-   Variable Explorer filtering
-   namespace render policy

## Apply behavior

``` cpp
ctx.panes->GetVariableExplorerPane()->SetFilterPolicy(...);
ctx.panes->GetVariableExplorerPane()->Refresh();
```

------------------------------------------------------------------------

# 22. Working Directory Page

## Captured UI

``` text
Working directory
├─ Startup
│  ├─ Project (if open) or home
│  └─ Custom directory
└─ New consoles
   ├─ Project (if open) or home
   ├─ Current console directory
   └─ Custom directory
```

## Data model

``` cpp
struct WorkingDirectorySettings {
    int startup_mode = 0;      // 0 project/home, 1 custom
    String startup_directory;

    int new_console_mode = 1;  // 0 project/home, 1 current console dir, 2 custom
    String new_console_directory;
};
```

## Internal connections

Binds to:

-   initial app working directory
-   new console launch location
-   Files pane root path
-   CWD toolbar behavior

## Apply behavior

``` cpp
ctx.main_window->SetWorkingDirectoryPolicy(...);
ctx.runtime->SetNewConsoleWorkingDirectoryPolicy(...);
ctx.panes->GetFilesPane()->ApplyWorkingDirectoryPolicy(...);
```

------------------------------------------------------------------------

# 23. Plugins Page

## Captured UI

List of plugins with:

-   icon
-   plugin name
-   description
-   built-in marker
-   enabled checkbox

Captured plugin examples:

-   Code Analysis
-   Completion and linting
-   Debugger
-   Editor
-   External terminal
-   Files
-   Find
-   Help
-   History
-   Interactive tours
-   IPython console
-   Online help
-   Outline Explorer
-   Plots
-   Profiler
-   Projects
-   PYTHONPATH manager
-   Remote client
-   Run
-   Status bar
-   Switcher
-   Variable explorer

## Data model

``` cpp
struct PluginState {
    String id;
    bool enabled = true;
};

struct PluginSettings {
    Vector<PluginState> states;
};
```

## Internal connections

Binds to:

-   `PluginManager`
-   pane registration
-   menu contribution
-   toolbar contribution
-   preferences contribution

## Apply behavior

Disabling plugins may require:

-   hiding panes
-   removing menus / toolbars
-   rejecting actions
-   possibly restart for hard unload

Recommended strategy:

``` cpp
class PluginManager {
public:
    void EnablePlugin(const String& id, bool enable);
    bool RequiresRestart(const String& id) const;
};
```

------------------------------------------------------------------------

# 24. Buttons: OK / Cancel / Apply / Reset to defaults

## Button behavior

### OK

-   save UI values into config
-   apply changes
-   persist config to disk
-   close dialog

### Apply

-   save UI values into config
-   apply changes
-   persist config to disk
-   keep dialog open

### Cancel

-   discard unsaved page edits
-   close dialog

### Reset to defaults

Two options are defensible:

1.  global reset for all pages
2.  reset current page only

Captured UI suggests a **global reset button** at dialog level, but a
page-local default reset is also useful.

Recommended behavior: - dialog-level button resets **all pages** -
page-specific buttons remain where explicitly captured

------------------------------------------------------------------------

# 25. Modified State Tracking

Preferences dialog should track if any page is dirty.

Recommended implementation:

``` cpp
class PreferencesWindow : public TopWindow {
    bool modified = false;

    void MarkModified() {
        modified = true;
        apply.Enable();
    }
};
```

Each control should connect to `MarkModified()`.

For example:

``` cpp
theme_drop << [=] { MarkModified(); };
show_line_numbers << [=] { MarkModified(); };
tab_width.WhenAction = [=] { MarkModified(); };
```

------------------------------------------------------------------------

# 26. Suggested U++ Page Classes

``` cpp
class AppearancePage : public PreferencesPage { ... };
class ApplicationPage : public PreferencesPage { ... };
class PythonInterpreterPage : public PreferencesPage { ... };
class KeyboardShortcutsPage : public PreferencesPage { ... };
class CodeAnalysisPage : public PreferencesPage { ... };
class CompletionLintingPage : public PreferencesPage { ... };
class DebuggerPage : public PreferencesPage { ... };
class EditorPage : public PreferencesPage { ... };
class FilesPage : public PreferencesPage { ... };
class HelpPage : public PreferencesPage { ... };
class HistoryPage : public PreferencesPage { ... };
class IPythonConsolePage : public PreferencesPage { ... };
class ProfilerPage : public PreferencesPage { ... };
class RunPage : public PreferencesPage { ... };
class StatusBarPage : public PreferencesPage { ... };
class VariableExplorerPage : public PreferencesPage { ... };
class WorkingDirectoryPage : public PreferencesPage { ... };
class PluginsPage : public PreferencesPage { ... };
```

------------------------------------------------------------------------

# 27. Recommended Runtime Wiring Graph

``` text
PreferencesWindow
├─ loads IDESettings
├─ hosts PreferencesPage instances
├─ Save() writes into IDESettings
└─ Apply() dispatches to IDEContext
       │
       ├─ IDEWindow
       ├─ EditorManager
       ├─ PaneManager
       ├─ ToolbarManager
       ├─ RunManager
       ├─ DebugManager
       ├─ ProfilerManager
       ├─ ByteVMInterface
       ├─ CompletionManager
       ├─ LintManager
       ├─ StatusBarController
       ├─ PluginManager
       └─ RemoteConnectionManager
```

This is the bridge between **GUI controls**, **persistent config**, and
**live systems**.

------------------------------------------------------------------------

# 28. Strong Recommendation for U++

Do not let page controls directly mutate subsystems one by one in
arbitrary callbacks.

Use this flow instead:

``` text
UI controls
   ↓
temporary page state
   ↓
IDESettings
   ↓
Apply(settings delta)
   ↓
live subsystems
```

That keeps the preferences system testable and prevents a swamp of side
effects.

------------------------------------------------------------------------

# 29. Files / Classes recommended in source tree

``` text
src/
├─ Settings/
│  ├─ IDESettings.h
│  ├─ IDESettings.cpp
│  ├─ PreferencesWindow.h
│  ├─ PreferencesWindow.cpp
│  ├─ PreferencesPage.h
│  ├─ AppearancePage.cpp
│  ├─ ApplicationPage.cpp
│  ├─ PythonInterpreterPage.cpp
│  ├─ KeyboardShortcutsPage.cpp
│  ├─ CodeAnalysisPage.cpp
│  ├─ CompletionLintingPage.cpp
│  ├─ DebuggerPage.cpp
│  ├─ EditorPage.cpp
│  ├─ FilesPage.cpp
│  ├─ HelpPage.cpp
│  ├─ HistoryPage.cpp
│  ├─ IPythonConsolePage.cpp
│  ├─ ProfilerPage.cpp
│  ├─ RunPage.cpp
│  ├─ StatusBarPage.cpp
│  ├─ VariableExplorerPage.cpp
│  ├─ WorkingDirectoryPage.cpp
│  └─ PluginsPage.cpp
```

------------------------------------------------------------------------

# 30. Final Summary

The Preferences system is not just a dialog.\
It is the **control plane** for the entire IDE.

It connects three worlds:

1.  **GUI state**
2.  **persistent configuration**
3.  **live runtime / services**

A good U++ implementation should therefore provide:

-   a clean page registry
-   a typed settings model
-   an apply-delta mechanism
-   a central runtime context
-   strong ownership boundaries

That way, the settings system becomes a predictable instrument panel
instead of a haunted electrical cabinet.
