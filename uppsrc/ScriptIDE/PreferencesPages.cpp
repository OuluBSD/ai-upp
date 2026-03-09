#include "ScriptIDE.h"

namespace Upp {

#define PREF_PAGE(name) \
class name##Page : public PreferencesPage { \
public: \
    Label placeholder; \
    name##Page() { Add(placeholder.SetLabel(#name " Settings Placeholder").SizePos()); } \
    virtual void Load(const IDESettings& cfg) override {} \
    virtual void Save(IDESettings& cfg) const override {} \
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {} \
    virtual void SetDefaults() override {} \
    virtual bool IsModified() const override { return false; } \
};

class AppearancePage : public WithAppearancePageLayout<PreferencesPage> {
public:
    typedef AppearancePage CLASSNAME;
    AppearancePage() {
        CtrlLayout(*this);

        interface_theme.Add("Default");
        interface_theme.Add("Dark");
        interface_theme.Add("Light");

        syntax_theme.Add("Default");
        syntax_theme.Add("Spyder");
        syntax_theme.Add("Monokai");

        for(int i = 0; i < Font::GetFaceCount(); i++) {
            if(Font::GetFaceInfo(i) & Font::FIXEDPITCH)
                monospace_font.Add(Font::GetFaceName(i));
            interface_font.Add(Font::GetFaceName(i));
        }
    }


    virtual void Load(const IDESettings& cfg) override {
        interface_theme.SetData(cfg.appearance.interface_theme);
        syntax_theme.SetData(cfg.appearance.syntax_theme);
        monospace_font.SetData(cfg.appearance.monospace_font_face);
        monospace_size.SetData(cfg.appearance.monospace_font_size);
        interface_font.SetData(cfg.appearance.interface_font_face);
        interface_size.SetData(cfg.appearance.interface_font_size);
        use_system_font.SetData(cfg.appearance.use_system_interface_font);
    }

    virtual void Save(IDESettings& cfg) const override {
        cfg.appearance.interface_theme = ~interface_theme;
        cfg.appearance.syntax_theme = ~syntax_theme;
        cfg.appearance.monospace_font_face = ~monospace_font;
        cfg.appearance.monospace_font_size = ~monospace_size;
        cfg.appearance.interface_font_face = ~interface_font;
        cfg.appearance.interface_font_size = ~interface_size;
        cfg.appearance.use_system_interface_font = ~use_system_font;
    }

    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {
        if(ctx.main_window) ctx.main_window->ApplySettings();
    }

    virtual void SetDefaults() override {
        interface_theme.SetData("Default");
        syntax_theme.SetData("Default");
        monospace_font.SetData("Courier New");
        monospace_size.SetData(10);
        interface_font.SetData("Arial");
        interface_size.SetData(10);
        use_system_font.SetData(true);
    }

    virtual bool IsModified() const override { return true; }
};

class EditorPage : public WithEditorDisplayTabLayout<PreferencesPage> {
public:
    typedef EditorPage CLASSNAME;
    EditorPage() { CtrlLayout(*this); }

    virtual void Load(const IDESettings& cfg) override {
        show_tab_bar.SetData(cfg.editor.show_tab_bar);
        show_path.SetData(cfg.editor.show_full_path_above_editor);
        show_selector.SetData(cfg.editor.show_class_function_selector);
        allow_scroll_past_eof.SetData(cfg.editor.allow_scroll_past_eof);
        show_indent_guides.SetData(cfg.editor.show_indent_guides);
        show_folding.SetData(cfg.editor.show_code_folding);
        show_line_numbers.SetData(cfg.editor.show_line_numbers);
        show_breakpoints.SetData(cfg.editor.show_debugger_breakpoints);
        show_annotations.SetData(cfg.editor.show_code_annotations);
        highlight_current_line.SetData(cfg.editor.highlight_current_line);
        highlight_current_cell.SetData(cfg.editor.highlight_current_cell);
        highlight_occurrences.SetData(cfg.editor.highlight_selected_occurrences);
        occurrence_delay.SetData(cfg.editor.occurrence_highlight_delay_ms);
    }

    virtual void Save(IDESettings& cfg) const override {
        cfg.editor.show_tab_bar = ~show_tab_bar;
        cfg.editor.show_full_path_above_editor = ~show_path;
        cfg.editor.show_class_function_selector = ~show_selector;
        cfg.editor.allow_scroll_past_eof = ~allow_scroll_past_eof;
        cfg.editor.show_indent_guides = ~show_indent_guides;
        cfg.editor.show_code_folding = ~show_folding;
        cfg.editor.show_line_numbers = ~show_line_numbers;
        cfg.editor.show_debugger_breakpoints = ~show_breakpoints;
        cfg.editor.show_code_annotations = ~show_annotations;
        cfg.editor.highlight_current_line = ~highlight_current_line;
        cfg.editor.highlight_current_cell = ~highlight_current_cell;
        cfg.editor.highlight_selected_occurrences = ~highlight_occurrences;
        cfg.editor.occurrence_highlight_delay_ms = ~occurrence_delay;
    }

    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {
        if(ctx.main_window) ctx.main_window->ApplySettings();
    }

    virtual void SetDefaults() override {
        show_tab_bar.SetData(true);
        show_path.SetData(true);
        show_selector.SetData(false);
        allow_scroll_past_eof.SetData(false);
        show_indent_guides.SetData(false);
        show_folding.SetData(true);
        show_line_numbers.SetData(true);
        show_breakpoints.SetData(true);
        show_annotations.SetData(true);
        highlight_current_line.SetData(true);
        highlight_current_cell.SetData(true);
        highlight_occurrences.SetData(true);
        occurrence_delay.SetData(1500);
    }

    virtual bool IsModified() const override { return true; }
};

class IPythonConsolePage : public WithConsoleInterfaceTabLayout<PreferencesPage> {
public:
    typedef IPythonConsolePage CLASSNAME;
    IPythonConsolePage() {
        CtrlLayout(*this);
        completion_display.Add("Graphical");
        completion_display.Add("Terminal");
        completion_display.Add("Plain Text");
    }

    virtual void Load(const IDESettings& cfg) override {
        show_welcome.SetData(cfg.console.show_welcome_message);
        show_calltips.SetData(cfg.console.show_calltips);
        show_elapsed_time.SetData(cfg.console.show_elapsed_time);
        confirm_closing.SetData(cfg.console.confirm_before_closing);
        confirm_restarting.SetData(cfg.console.confirm_before_restarting);
        confirm_remove_vars.SetData(cfg.console.confirm_before_removing_variables);
        completion_display.SetData(cfg.console.completion_display);
        use_jedi.SetData(cfg.console.use_jedi_completion);
        use_greedy.SetData(cfg.console.use_greedy_completion);
        buffer_lines.SetData(cfg.console.output_buffer_lines);
        render_sympy.SetData(cfg.console.render_sympy_math);
    }

    virtual void Save(IDESettings& cfg) const override {
        cfg.console.show_welcome_message = ~show_welcome;
        cfg.console.show_calltips = ~show_calltips;
        cfg.console.show_elapsed_time = ~show_elapsed_time;
        cfg.console.confirm_before_closing = ~confirm_closing;
        cfg.console.confirm_before_restarting = ~confirm_restarting;
        cfg.console.confirm_before_removing_variables = ~confirm_remove_vars;
        cfg.console.completion_display = ~completion_display;
        cfg.console.use_jedi_completion = ~use_jedi;
        cfg.console.use_greedy_completion = ~use_greedy;
        cfg.console.output_buffer_lines = ~buffer_lines;
        cfg.console.render_sympy_math = ~render_sympy;
    }

    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}

    virtual void SetDefaults() override {
        show_welcome.SetData(true);
        show_calltips.SetData(true);
        show_elapsed_time.SetData(false);
        confirm_closing.SetData(false);
        confirm_restarting.SetData(true);
        confirm_remove_vars.SetData(true);
        completion_display.SetData("Graphical");
        use_jedi.SetData(false);
        use_greedy.SetData(false);
        buffer_lines.SetData(5000);
        render_sympy.SetData(false);
    }

    virtual bool IsModified() const override { return true; }
};

class ApplicationPage : public PreferencesPage {
public:
    typedef ApplicationPage CLASSNAME;
    
    TabCtrl tabs;
    
    struct InterfaceTab : public WithApplicationInterfaceTabLayout<ParentCtrl> {
        InterfaceTab() { CtrlLayout(*this); }
    } interface_tab;
    
    struct AdvancedTab : public WithApplicationAdvancedTabLayout<ParentCtrl> {
        AdvancedTab() {
            CtrlLayout(*this);
            language.Add("English");
            language.Add("French");
            language.Add("Spanish");
            
            rendering_engine.Add("Default");
            rendering_engine.Add("Software");
            rendering_engine.Add("OpenGL");
        }
    } advanced_tab;

    ApplicationPage() {
        Add(tabs.SizePos());
        tabs.Add(interface_tab, "Interface");
        tabs.Add(advanced_tab, "Advanced settings");
    }

    virtual void Load(const IDESettings& cfg) override {
        interface_tab.hidpi_mode.SetData(cfg.application.hidpi_mode);
        interface_tab.custom_scale.SetData(cfg.application.custom_scale);
        interface_tab.show_friendly_empty.SetData(cfg.application.show_friendly_empty_messages);
        interface_tab.vertical_tabs.SetData(cfg.application.vertical_tabs_in_panes);
        interface_tab.custom_margin.SetData(cfg.application.pane_margin > 0);
        interface_tab.pane_margin.SetData(cfg.application.pane_margin);
        interface_tab.cursor_blinking.SetData(cfg.application.custom_cursor_blink);
        interface_tab.blink_ms.SetData(cfg.application.cursor_blink_ms);
        
        advanced_tab.language.SetData(cfg.application.language);
        advanced_tab.rendering_engine.SetData(cfg.application.rendering_engine);
        advanced_tab.single_instance.SetData(cfg.application.single_instance);
        advanced_tab.prompt_exit.SetData(cfg.application.prompt_on_exit);
        advanced_tab.show_internal_errors.SetData(cfg.application.show_internal_errors);
        advanced_tab.check_updates.SetData(cfg.application.check_updates_on_startup);
        advanced_tab.stable_only.SetData(cfg.application.stable_releases_only);
        advanced_tab.disable_wheel_zoom.SetData(cfg.application.disable_ctrl_wheel_zoom);
    }

    virtual void Save(IDESettings& cfg) const override {
        cfg.application.hidpi_mode = ~interface_tab.hidpi_mode;
        cfg.application.custom_scale = ~interface_tab.custom_scale;
        cfg.application.show_friendly_empty_messages = ~interface_tab.show_friendly_empty;
        cfg.application.vertical_tabs_in_panes = ~interface_tab.vertical_tabs;
        cfg.application.pane_margin = ~interface_tab.custom_margin ? (int)~interface_tab.pane_margin : 0;
        cfg.application.custom_cursor_blink = ~interface_tab.cursor_blinking;
        cfg.application.cursor_blink_ms = ~interface_tab.blink_ms;
        
        cfg.application.language = ~advanced_tab.language;
        cfg.application.rendering_engine = ~advanced_tab.rendering_engine;
        cfg.application.single_instance = ~advanced_tab.single_instance;
        cfg.application.prompt_on_exit = ~advanced_tab.prompt_exit;
        cfg.application.show_internal_errors = ~advanced_tab.show_internal_errors;
        cfg.application.check_updates_on_startup = ~advanced_tab.check_updates;
        cfg.application.stable_releases_only = ~advanced_tab.stable_only;
        cfg.application.disable_ctrl_wheel_zoom = ~advanced_tab.disable_wheel_zoom;
    }

    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}

    virtual void SetDefaults() override {
        interface_tab.hidpi_mode.SetData(0);
        interface_tab.custom_scale.SetData(1.0);
        advanced_tab.language.SetData("English");
    }

    virtual bool IsModified() const override { return true; }
};

class PythonInterpreterPage : public WithPythonInterpreterPageLayout<PreferencesPage> {
public:
    typedef PythonInterpreterPage CLASSNAME;
    PythonInterpreterPage() {
        CtrlLayout(*this);
        browse_interpreter.WhenAction = [=] {
            FileSel fs;
            if(fs.ExecuteOpen("Select Interpreter")) interpreter_path.SetData(fs.Get());
        };
        browse_conda.WhenAction = [=] {
            FileSel fs;
            if(fs.ExecuteOpen("Select Conda Executable")) conda_path.SetData(fs.Get());
        };
    }

    virtual void Load(const IDESettings& cfg) override {
        interpreter_mode.SetData(cfg.python.use_internal ? 0 : 1);
        interpreter_path.SetData(cfg.python.interpreter_path);
        use_custom_conda.SetData(cfg.python.use_custom_conda);
        conda_path.SetData(cfg.python.conda_executable);
        umr_enabled.SetData(cfg.python.umr_enabled);
        umr_verbose.SetData(cfg.python.umr_verbose);
    }

    virtual void Save(IDESettings& cfg) const override {
        cfg.python.use_internal = ((int)~interpreter_mode == 0);
        cfg.python.interpreter_path = ~interpreter_path;
        cfg.python.use_custom_conda = ~use_custom_conda;
        cfg.python.conda_executable = ~conda_path;
        cfg.python.umr_enabled = ~umr_enabled;
        cfg.python.umr_verbose = ~umr_verbose;
    }

    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override {
        interpreter_mode.SetData(0);
        umr_enabled.SetData(true);
    }
    virtual bool IsModified() const override { return true; }
};

class KeyboardShortcutsPage : public WithShortcutsPageLayout<PreferencesPage> {
public:
    typedef KeyboardShortcutsPage CLASSNAME;
    KeyboardShortcutsPage() {
        CtrlLayout(*this);
        list.AddColumn("Context");
        list.AddColumn("Name");
        list.AddColumn("Shortcut");
    }

    virtual void Load(const IDESettings& cfg) override {
        list.Clear();
        for(const auto& item : cfg.shortcuts.items)
            list.Add(item.context, item.action_id, GetKeyDesc(item.key));
    }

    virtual void Save(IDESettings& cfg) const override {}
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override {}
    virtual bool IsModified() const override { return true; }
};
class CodeAnalysisPage : public WithCodeAnalysisPageLayout<PreferencesPage> {
public:
    typedef CodeAnalysisPage CLASSNAME;
    CodeAnalysisPage() { CtrlLayout(*this); }

    virtual void Load(const IDESettings& cfg) override {
        save_before.SetData(cfg.code_analysis.save_before_analysis);
        history_count.SetData(cfg.code_analysis.history_results);
        results_path.SetLabel(cfg.code_analysis.results_path);
    }

    virtual void Save(IDESettings& cfg) const override {
        cfg.code_analysis.save_before_analysis = ~save_before;
        cfg.code_analysis.history_results = ~history_count;
    }

    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override {
        save_before.SetData(true);
        history_count.SetData(30);
    }
    virtual bool IsModified() const override { return true; }
};

class CompletionLintingPage : public PreferencesPage {
public:
    typedef CompletionLintingPage CLASSNAME;
    TabCtrl tabs;

    struct GeneralTab : public WithCompletionGeneralTabLayout<ParentCtrl> {
        GeneralTab() { CtrlLayout(*this); }
    } general_tab;

    struct LintingTab : public WithCompletionLintingTabLayout<ParentCtrl> {
        LintingTab() { CtrlLayout(*this); }
    } linting_tab;

    CompletionLintingPage() {
        Add(tabs.SizePos());
        tabs.Add(general_tab, "General");
        tabs.Add(linting_tab, "Linting");
    }

    virtual void Load(const IDESettings& cfg) override {
        general_tab.show_details.SetData(cfg.completion.show_completion_details);
        general_tab.enable_snippets.SetData(cfg.completion.enable_code_snippets);
        general_tab.completions_fly.SetData(cfg.completion.show_completions_on_the_fly);
        general_tab.chars_before.SetData(cfg.completion.chars_before_completion);
        general_tab.detail_delay.SetData(cfg.completion.completion_detail_delay_ms);
        general_tab.timeout.SetData(cfg.completion.provider_timeout_ms);
        general_tab.enable_fallback.SetData(cfg.completion.enable_fallback_provider);
        general_tab.enable_lsp.SetData(cfg.completion.enable_lsp_provider);
        general_tab.enable_snippets_provider.SetData(cfg.completion.enable_snippet_provider);

        linting_tab.provider.SetData(
            cfg.completion.lint_provider == "pyflakes" ? 0 :
            cfg.completion.lint_provider == "flake8" ? 1 :
            cfg.completion.lint_provider == "ruff" ? 2 : 3
        );
        linting_tab.underline_errors.SetData(cfg.completion.underline_errors);
    }

    virtual void Save(IDESettings& cfg) const override {
        cfg.completion.show_completion_details = ~general_tab.show_details;
        cfg.completion.enable_code_snippets = ~general_tab.enable_snippets;
        cfg.completion.show_completions_on_the_fly = ~general_tab.completions_fly;
        cfg.completion.chars_before_completion = ~general_tab.chars_before;
        cfg.completion.completion_detail_delay_ms = ~general_tab.detail_delay;
        cfg.completion.provider_timeout_ms = ~general_tab.timeout;
        cfg.completion.enable_fallback_provider = ~general_tab.enable_fallback;
        cfg.completion.enable_lsp_provider = ~general_tab.enable_lsp;
        cfg.completion.enable_snippet_provider = ~general_tab.enable_snippets_provider;

        int p = ~linting_tab.provider;
        cfg.completion.lint_provider = (p == 0 ? "pyflakes" : p == 1 ? "flake8" : p == 2 ? "ruff" : "none");
        cfg.completion.underline_errors = ~linting_tab.underline_errors;
    }

    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override {
        general_tab.show_details.SetData(true);
        linting_tab.provider.SetData(0);
    }
    virtual bool IsModified() const override { return true; }
};

class DebuggerPage : public WithDebuggerPageLayout<PreferencesPage> {
public:
    typedef DebuggerPage CLASSNAME;
    DebuggerPage() { CtrlLayout(*this); }

    virtual void Load(const IDESettings& cfg) override {
        prevent_close.SetData(cfg.debugger.prevent_close_while_debugging);
        stop_first_line.SetData(cfg.debugger.stop_on_first_line_without_breakpoints);
        ignore_libs.SetData(cfg.debugger.ignore_python_libraries);
        process_events.SetData(cfg.debugger.process_execute_events);
        exclamation_prefix.SetData(cfg.debugger.use_exclamation_prefix);
        debug_lines.SetData(cfg.debugger.preload_debug_lines);
        exclude_internal.SetData(cfg.debugger.exclude_internal_frames);
    }

    virtual void Save(IDESettings& cfg) const override {
        cfg.debugger.prevent_close_while_debugging = ~prevent_close;
        cfg.debugger.stop_on_first_line_without_breakpoints = ~stop_first_line;
        cfg.debugger.ignore_python_libraries = ~ignore_libs;
        cfg.debugger.process_execute_events = ~process_events;
        cfg.debugger.use_exclamation_prefix = ~exclamation_prefix;
        cfg.debugger.preload_debug_lines = ~debug_lines;
        cfg.debugger.exclude_internal_frames = ~exclude_internal;
    }

    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override {
        prevent_close.SetData(true);
        exclude_internal.SetData(true);
    }
    virtual bool IsModified() const override { return true; }
};
PREF_PAGE(Files)
PREF_PAGE(Help)
PREF_PAGE(History)
PREF_PAGE(Profiler)
PREF_PAGE(Run)
PREF_PAGE(StatusBar)
PREF_PAGE(VariableExplorer)
PREF_PAGE(WorkingDirectory)
PREF_PAGE(Plugins)

void PythonIDE::OnSettings()
{
	IDEContext ctx;
	ctx.main_window = this;
	
	PreferencesWindow dlg(ctx, settings);
	
	AppearancePage appearance;
	ApplicationPage application;
	PythonInterpreterPage python;
	KeyboardShortcutsPage shortcuts;
	CodeAnalysisPage analysis;
	CompletionLintingPage completion;
	DebuggerPage debugger;
	EditorPage editor;
	FilesPage files;
	HelpPage help;
	HistoryPage history;
	IPythonConsolePage console;
	ProfilerPage profiler;
	RunPage run;
	StatusBarPage statusbar;
	VariableExplorerPage var_explorer;
	WorkingDirectoryPage work_dir;
	PluginsPage plugins;

	dlg.AddPage("appearance", "Appearance", Image(), &appearance);
	dlg.AddPage("application", "Application", Image(), &application);
	dlg.AddPage("python", "Python interpreter", Image(), &python);
	dlg.AddPage("shortcuts", "Keyboard shortcuts", Image(), &shortcuts);
	dlg.AddPage("analysis", "Code Analysis", Image(), &analysis);
	dlg.AddPage("completion", "Completion and linting", Image(), &completion);
	dlg.AddPage("debugger", "Debugger", Image(), &debugger);
	dlg.AddPage("editor", "Editor", Image(), &editor);
	dlg.AddPage("files", "Files", Image(), &files);
	dlg.AddPage("help", "Help", Image(), &help);
	dlg.AddPage("history", "History", Image(), &history);
	dlg.AddPage("console", "IPython console", Image(), &console);
	dlg.AddPage("profiler", "Profiler", Image(), &profiler);
	dlg.AddPage("run", "Run", Image(), &run);
	dlg.AddPage("statusbar", "Status bar", Image(), &statusbar);
	dlg.AddPage("variable_explorer", "Variable explorer", Image(), &var_explorer);
	dlg.AddPage("work_dir", "Working directory", Image(), &work_dir);
	dlg.AddPage("plugins", "Plugins", Image(), &plugins);

	if(dlg.Execute() == IDOK) {
		ApplySettings();
		StoreToFile(settings, ConfigFile("ide_settings.bin"));
	}
}

}
