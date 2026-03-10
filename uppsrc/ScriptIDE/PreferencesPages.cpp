#include "ScriptIDE.h"

NAMESPACE_UPP

// --- AppearancePage ---

class AppearancePage : public WithAppearancePageLayout<PreferencesPage> {
public:
    typedef AppearancePage CLASSNAME;
    AppearancePage() {
        CtrlLayout(*this);
        interface_theme.Add("Default"); interface_theme.Add("Dark"); interface_theme.Add("Light");
        syntax_theme.Add("Default"); syntax_theme.Add("Spyder"); syntax_theme.Add("Monokai");
        for(int i = 0; i < Font::GetFaceCount(); i++) {
            if(Font::GetFaceInfo(i) & Font::FIXEDPITCH) monospace_font.Add(Font::GetFaceName(i));
            interface_font.Add(Font::GetFaceName(i));
        }
        auto update_previews = [=] {
            Font mf = Font(Font::FindFaceNameIndex(~monospace_font), ~monospace_size);
            Font ifnt = Font(Font::FindFaceNameIndex(~interface_font), ~interface_size);
            editor_lbl.SetFont(mf);
            interface_font_lbl.SetFont(ifnt);
        };
        monospace_font.WhenAction = update_previews;
        monospace_size.WhenAction = update_previews;
        interface_font.WhenAction = update_previews;
        interface_size.WhenAction = update_previews;
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
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- ApplicationPage ---

class ApplicationPage : public PreferencesPage {
public:
    TabCtrl tabs;
    struct InterfaceTab : public WithApplicationInterfaceTabLayout<ParentCtrl> { InterfaceTab() { CtrlLayout(*this); } } interface_tab;
    struct AdvancedTab : public WithApplicationAdvancedTabLayout<ParentCtrl> { 
        AdvancedTab() { 
            CtrlLayout(*this); 
            language.Add("English"); language.Add("French"); language.Add("Spanish");
            rendering_engine.Add("Default"); rendering_engine.Add("Software"); rendering_engine.Add("OpenGL");
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
        interface_tab.custom_margin.SetData(cfg.application.custom_margin);
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
        cfg.application.custom_margin = ~interface_tab.custom_margin;
        cfg.application.pane_margin = ~interface_tab.pane_margin;
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
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {
        if(ctx.main_window) ctx.main_window->ApplySettings();
    }
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- PythonInterpreterPage ---

class PythonInterpreterPage : public WithPythonInterpreterPageLayout<PreferencesPage> {
public:
    typedef PythonInterpreterPage CLASSNAME;
    PythonInterpreterPage() {
        CtrlLayout(*this);
        browse_interpreter.WhenAction = [=] { FileSel fs; if(fs.ExecuteOpen("Select Interpreter")) interpreter_path.SetData(fs.Get()); };
        browse_conda.WhenAction = [=] { FileSel fs; if(fs.ExecuteOpen("Select Conda Executable")) conda_path.SetData(fs.Get()); };
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
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- KeyboardShortcutsPage ---

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
    virtual void Save(IDESettings& cfg) const override {
        cfg.shortcuts.items.Clear();
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- CodeAnalysisPage ---

class CodeAnalysisPage : public WithCodeAnalysisPageLayout<PreferencesPage> {
public:
    typedef CodeAnalysisPage CLASSNAME;
    CodeAnalysisPage() {
        CtrlLayout(*this);
    }
    virtual void Load(const IDESettings& cfg) override {
        save_before.SetData(cfg.code_analysis.save_before_analysis);
        history_count.SetData(cfg.code_analysis.history_results);
        results_path.SetData(cfg.code_analysis.results_path);
    }
    virtual void Save(IDESettings& cfg) const override {
        cfg.code_analysis.save_before_analysis = ~save_before;
        cfg.code_analysis.history_results = ~history_count;
        cfg.code_analysis.results_path = ~results_path;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- CompletionPage ---

class CompletionPage : public PreferencesPage {
public:
    TabCtrl tabs;
    struct GeneralTab : public WithCompletionGeneralTabLayout<ParentCtrl> { GeneralTab() { CtrlLayout(*this); } } general_tab;
    struct LintingTab : public WithCompletionLintingTabLayout<ParentCtrl> { LintingTab() { CtrlLayout(*this); } } linting_tab;
    struct IntrospectionTab : public WithCompletionIntrospectionTabLayout<ParentCtrl> { IntrospectionTab() { CtrlLayout(*this); } } introspection_tab;
    struct FormattingTab : public WithCompletionFormattingTabLayout<ParentCtrl> { FormattingTab() { CtrlLayout(*this); formatter.Add("autopep8"); formatter.Add("black"); } } formatting_tab;
    struct AdvancedTab : public WithCompletionAdvancedTabLayout<ParentCtrl> { AdvancedTab() { CtrlLayout(*this); } } advanced_tab;

    CompletionPage() {
        Add(tabs.SizePos());
        tabs.Add(general_tab, "General");
        tabs.Add(linting_tab, "Linting");
        tabs.Add(introspection_tab, "Introspection");
        tabs.Add(formatting_tab, "Code formatting");
        tabs.Add(advanced_tab, "Advanced");
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

        linting_tab.provider.SetData(cfg.completion.lint_provider == "pyflakes" ? 0 : 3);
        linting_tab.underline_errors.SetData(cfg.completion.underline_errors);

        introspection_tab.enable_goto.SetData(cfg.completion.enable_go_to_definition);
        introspection_tab.follow_imports.SetData(cfg.completion.follow_imports);
        introspection_tab.show_calltips.SetData(cfg.completion.show_calltips);
        introspection_tab.enable_hover.SetData(cfg.completion.enable_hover_hints);
        introspection_tab.preload_modules.SetData(cfg.completion.preload_modules_csv);

        formatting_tab.formatter.SetData(cfg.completion.formatter);
        formatting_tab.autoformat_save.SetData(cfg.completion.autoformat_on_save);
        formatting_tab.max_line_length.SetData(cfg.completion.max_line_length);
        formatting_tab.show_ruler.SetData(cfg.completion.show_vertical_ruler);

        advanced_tab.lsp_advanced.SetData(cfg.completion.lsp_advanced_enabled);
        advanced_tab.lsp_module.SetData(cfg.completion.lsp_module);
        advanced_tab.lsp_address.SetData(cfg.completion.lsp_address);
        advanced_tab.lsp_port.SetData(cfg.completion.lsp_port);
        advanced_tab.lsp_external.SetData(cfg.completion.lsp_external_server);
        advanced_tab.lsp_stdio.SetData(cfg.completion.lsp_use_stdio);
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

        cfg.completion.lint_provider = ((int)~linting_tab.provider == 0 ? "pyflakes" : "disabled");
        cfg.completion.underline_errors = ~linting_tab.underline_errors;

        cfg.completion.enable_go_to_definition = ~introspection_tab.enable_goto;
        cfg.completion.follow_imports = ~introspection_tab.follow_imports;
        cfg.completion.show_calltips = ~introspection_tab.show_calltips;
        cfg.completion.enable_hover_hints = ~introspection_tab.enable_hover;
        cfg.completion.preload_modules_csv = ~introspection_tab.preload_modules;

        cfg.completion.formatter = ~formatting_tab.formatter;
        cfg.completion.autoformat_on_save = ~formatting_tab.autoformat_save;
        cfg.completion.max_line_length = ~formatting_tab.max_line_length;
        cfg.completion.show_vertical_ruler = ~formatting_tab.show_ruler;

        cfg.completion.lsp_advanced_enabled = ~advanced_tab.lsp_advanced;
        cfg.completion.lsp_module = ~advanced_tab.lsp_module;
        cfg.completion.lsp_address = ~advanced_tab.lsp_address;
        cfg.completion.lsp_port = ~advanced_tab.lsp_port;
        cfg.completion.lsp_external_server = ~advanced_tab.lsp_external;
        cfg.completion.lsp_use_stdio = ~advanced_tab.lsp_stdio;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- DebuggerPage ---

class DebuggerPage : public WithDebuggerPageLayout<PreferencesPage> {
public:
    typedef DebuggerPage CLASSNAME;
    DebuggerPage() {
        CtrlLayout(*this);
    }
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
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- EditorPage ---

class EditorPage : public PreferencesPage {
public:
    TabCtrl tabs;
    struct DisplayTab : public WithEditorDisplayTabLayout<ParentCtrl> { DisplayTab() { CtrlLayout(*this); } } display_tab;
    struct SourceTab : public WithEditorSourceCodeTabLayout<ParentCtrl> { SourceTab() { CtrlLayout(*this); eol_mode.Add("LF"); eol_mode.Add("CRLF"); } } source_tab;
    struct AdvancedTab : public WithEditorAdvancedTabLayout<ParentCtrl> { AdvancedTab() { CtrlLayout(*this); docstring_style.Add("Numpy"); docstring_style.Add("Google"); } } advanced_tab;

    EditorPage() {
        Add(tabs.SizePos());
        tabs.Add(display_tab, "Display");
        tabs.Add(source_tab, "Source code");
        tabs.Add(advanced_tab, "Advanced");
    }
    virtual void Load(const IDESettings& cfg) override {
        display_tab.show_tab_bar.SetData(cfg.editor.show_tab_bar);
        display_tab.show_path.SetData(cfg.editor.show_full_path_above_editor);
        display_tab.show_selector.SetData(cfg.editor.show_class_function_selector);
        display_tab.allow_scroll_past_eof.SetData(cfg.editor.allow_scroll_past_eof);
        display_tab.show_indent_guides.SetData(cfg.editor.show_indent_guides);
        display_tab.show_folding.SetData(cfg.editor.show_code_folding);
        display_tab.show_line_numbers.SetData(cfg.editor.show_line_numbers);
        display_tab.show_breakpoints.SetData(cfg.editor.show_debugger_breakpoints);
        display_tab.show_annotations.SetData(cfg.editor.show_code_annotations);
        display_tab.highlight_current_line.SetData(cfg.editor.highlight_current_line);
        display_tab.highlight_current_cell.SetData(cfg.editor.highlight_current_cell);
        display_tab.highlight_occurrences.SetData(cfg.editor.highlight_selected_occurrences);
        display_tab.occurrence_delay.SetData(cfg.editor.occurrence_highlight_delay_ms);

        source_tab.auto_close_brackets.SetData(cfg.editor.auto_insert_closing_brackets);
        source_tab.auto_close_quotes.SetData(cfg.editor.auto_insert_closing_quotes);
        source_tab.auto_colons.SetData(cfg.editor.auto_insert_colons);
        source_tab.auto_unindent.SetData(cfg.editor.auto_unindent_keywords);
        source_tab.strip_trailing.SetData(cfg.editor.strip_trailing_spaces_on_save);
        source_tab.strip_changed.SetData(cfg.editor.strip_trailing_spaces_changed_lines);
        source_tab.add_eof_newline.SetData(cfg.editor.add_missing_newline_eof);
        source_tab.strip_eof_blank.SetData(cfg.editor.strip_blank_lines_eof);
        source_tab.tab_width.SetData(cfg.editor.tab_width);
        source_tab.indent_spaces.SetData(cfg.editor.indent_with_spaces);
        source_tab.intelligent_backspace.SetData(cfg.editor.intelligent_backspace);
        source_tab.tab_always_indents.SetData(cfg.editor.tab_always_indents);
        source_tab.fix_mixed_eol.SetData(cfg.editor.fix_mixed_eol);
        source_tab.convert_eol_on_save.SetData(cfg.editor.convert_eol_on_save);
        source_tab.eol_mode.SetData(cfg.editor.eol_mode);

        advanced_tab.autosave_enabled.SetData(cfg.editor.autosave_backup_unsaved);
        advanced_tab.autosave_interval.SetData(cfg.editor.autosave_interval_sec);
        advanced_tab.docstring_style.SetData(cfg.editor.docstring_style);
        advanced_tab.enable_multicursor.SetData(cfg.editor.enable_multicursor);
        advanced_tab.multicursor_paste.SetData(cfg.editor.multicursor_paste_mode);
    }
    virtual void Save(IDESettings& cfg) const override {
        cfg.editor.show_tab_bar = ~display_tab.show_tab_bar;
        cfg.editor.show_full_path_above_editor = ~display_tab.show_path;
        cfg.editor.show_class_function_selector = ~display_tab.show_selector;
        cfg.editor.allow_scroll_past_eof = ~display_tab.allow_scroll_past_eof;
        cfg.editor.show_indent_guides = ~display_tab.show_indent_guides;
        cfg.editor.show_code_folding = ~display_tab.show_folding;
        cfg.editor.show_line_numbers = ~display_tab.show_line_numbers;
        cfg.editor.show_debugger_breakpoints = ~display_tab.show_breakpoints;
        cfg.editor.show_code_annotations = ~display_tab.show_annotations;
        cfg.editor.highlight_current_line = ~display_tab.highlight_current_line;
        cfg.editor.highlight_current_cell = ~display_tab.highlight_current_cell;
        cfg.editor.highlight_selected_occurrences = ~display_tab.highlight_occurrences;
        cfg.editor.occurrence_highlight_delay_ms = ~display_tab.occurrence_delay;

        cfg.editor.auto_insert_closing_brackets = ~source_tab.auto_close_brackets;
        cfg.editor.auto_insert_closing_quotes = ~source_tab.auto_close_quotes;
        cfg.editor.auto_insert_colons = ~source_tab.auto_colons;
        cfg.editor.auto_unindent_keywords = ~source_tab.auto_unindent;
        cfg.editor.strip_trailing_spaces_on_save = ~source_tab.strip_trailing;
        cfg.editor.strip_trailing_spaces_changed_lines = ~source_tab.strip_changed;
        cfg.editor.add_missing_newline_eof = ~source_tab.add_eof_newline;
        cfg.editor.strip_blank_lines_eof = ~source_tab.strip_eof_blank;
        cfg.editor.tab_width = ~source_tab.tab_width;
        cfg.editor.indent_with_spaces = ~source_tab.indent_spaces;
        cfg.editor.intelligent_backspace = ~source_tab.intelligent_backspace;
        cfg.editor.tab_always_indents = ~source_tab.tab_always_indents;
        cfg.editor.fix_mixed_eol = ~source_tab.fix_mixed_eol;
        cfg.editor.convert_eol_on_save = ~source_tab.convert_eol_on_save;
        cfg.editor.eol_mode = ~source_tab.eol_mode;

        cfg.editor.autosave_backup_unsaved = ~advanced_tab.autosave_enabled;
        cfg.editor.autosave_interval_sec = ~advanced_tab.autosave_interval;
        cfg.editor.docstring_style = ~advanced_tab.docstring_style;
        cfg.editor.enable_multicursor = ~advanced_tab.enable_multicursor;
        cfg.editor.multicursor_paste_mode = ~advanced_tab.multicursor_paste;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {
        if(ctx.main_window) ctx.main_window->ApplySettings();
    }
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- FilesPage ---

class FilesPage : public PreferencesPage {
public:
    TabCtrl tabs;
    struct GeneralTab : public WithFilesGeneralTabLayout<ParentCtrl> { GeneralTab() { CtrlLayout(*this); } } general_tab;
    struct AssociationsTab : public WithFilesAssociationsTabLayout<ParentCtrl> { AssociationsTab() { CtrlLayout(*this); } } associations_tab;

    FilesPage() {
        Add(tabs.SizePos());
        tabs.Add(general_tab, "General");
        tabs.Add(associations_tab, "File associations");
    }
    virtual void Load(const IDESettings& cfg) override {
        general_tab.show_hidden.SetData(cfg.files.show_hidden_files);
        general_tab.single_click.SetData(cfg.files.single_click_open);
        general_tab.filter_patterns.SetData(cfg.files.filter_patterns_csv);
    }
    virtual void Save(IDESettings& cfg) const override {
        cfg.files.show_hidden_files = ~general_tab.show_hidden;
        cfg.files.single_click_open = ~general_tab.single_click;
        cfg.files.filter_patterns_csv = ~general_tab.filter_patterns;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {
        if(ctx.main_window) ctx.main_window->ApplySettings();
    }
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- HelpPage ---

class HelpPage : public WithHelpPageLayout<PreferencesPage> {
public:
    typedef HelpPage CLASSNAME;
    HelpPage() {
        CtrlLayout(*this);
    }
    virtual void Load(const IDESettings& cfg) override {
        connect_editor.SetData(cfg.help.auto_connect_editor);
        connect_console.SetData(cfg.help.auto_connect_console);
        render_math.SetData(cfg.help.render_math);
        wrap_lines.SetData(cfg.help.wrap_lines);
    }
    virtual void Save(IDESettings& cfg) const override {
        cfg.help.auto_connect_editor = ~connect_editor;
        cfg.help.auto_connect_console = ~connect_console;
        cfg.help.render_math = ~render_math;
        cfg.help.wrap_lines = ~wrap_lines;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- HistoryPage ---

class HistoryPage : public WithHistoryPageLayout<PreferencesPage> {
public:
    typedef HistoryPage CLASSNAME;
    HistoryPage() {
        CtrlLayout(*this);
    }
    virtual void Load(const IDESettings& cfg) override {
        wrap_lines.SetData(cfg.history.wrap_lines);
        show_line_numbers.SetData(cfg.history.show_line_numbers);
        scroll_to_last.SetData(cfg.history.scroll_to_last_entry);
    }
    virtual void Save(IDESettings& cfg) const override {
        cfg.history.wrap_lines = ~wrap_lines;
        cfg.history.show_line_numbers = ~show_line_numbers;
        cfg.history.scroll_to_last_entry = ~scroll_to_last;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- IPythonConsolePage ---

class IPythonConsolePage : public PreferencesPage {
public:
    TabCtrl tabs;
    struct InterfaceTab : public WithConsoleInterfaceTabLayout<ParentCtrl> { InterfaceTab() { CtrlLayout(*this); completion_display.Add("Graphical"); completion_display.Add("Terminal"); } } interface_tab;
    struct PlottingTab : public WithConsolePlottingTabLayout<ParentCtrl> { PlottingTab() { CtrlLayout(*this); backend.Add("Inline"); backend.Add("Automatic"); format.Add("PNG"); format.Add("SVG"); } } plotting_tab;
    struct StartupTab : public WithConsoleStartupTabLayout<ParentCtrl> { StartupTab() { CtrlLayout(*this); } } startup_tab;
    struct AdvancedTab : public WithConsoleAdvancedTabLayout<ParentCtrl> { AdvancedTab() { CtrlLayout(*this); autocall.Add("Off"); autocall.Add("On"); } } advanced_tab;

    IPythonConsolePage() {
        Add(tabs.SizePos());
        tabs.Add(interface_tab, "Interface");
        tabs.Add(plotting_tab, "Plotting");
        tabs.Add(startup_tab, "Startup");
        tabs.Add(advanced_tab, "Advanced");
    }
    virtual void Load(const IDESettings& cfg) override {
        interface_tab.show_welcome.SetData(cfg.console.show_welcome_message);
        interface_tab.show_calltips.SetData(cfg.console.show_calltips);
        interface_tab.show_elapsed_time.SetData(cfg.console.show_elapsed_time);
        interface_tab.confirm_closing.SetData(cfg.console.confirm_before_closing);
        interface_tab.confirm_restarting.SetData(cfg.console.confirm_before_restarting);
        interface_tab.confirm_remove_vars.SetData(cfg.console.confirm_before_removing_variables);
        interface_tab.completion_display.SetData(cfg.console.completion_display);
        interface_tab.use_jedi.SetData(cfg.console.use_jedi_completion);
        interface_tab.use_greedy.SetData(cfg.console.use_greedy_completion);
        interface_tab.buffer_lines.SetData(cfg.console.output_buffer_lines);
        interface_tab.render_sympy.SetData(cfg.console.render_sympy_math);

        plotting_tab.activate_support.SetData(cfg.console.matplotlib_support);
        plotting_tab.auto_import.SetData(cfg.console.auto_import_numpy_matplotlib);
        plotting_tab.backend.SetData(cfg.console.graphics_backend);
        plotting_tab.format.SetData(cfg.console.inline_format);
        plotting_tab.resolution.SetData(AsString(cfg.console.inline_resolution_dpi));
        plotting_tab.width.SetData(cfg.console.inline_width_in);
        plotting_tab.height.SetData(cfg.console.inline_height_in);
        plotting_tab.font_size.SetData(AsString(cfg.console.inline_font_points));
        plotting_tab.bottom_edge.SetData(AsString(cfg.console.inline_bottom_edge));
        plotting_tab.tight_layout.SetData(cfg.console.inline_tight_layout);

        startup_tab.startup_lines.SetData(cfg.console.startup_code);
        startup_tab.execute_file.SetData(cfg.console.execute_startup_file);
        startup_tab.startup_file_path.SetData(cfg.console.startup_file);

        advanced_tab.autocall.SetData(cfg.console.autocall_mode);
        advanced_tab.use_autoreload.SetData(cfg.console.use_autoreload);
        advanced_tab.input_prompt.SetData(cfg.console.input_prompt);
        advanced_tab.output_prompt.SetData(cfg.console.output_prompt);
        advanced_tab.hide_cmd_output.SetData(cfg.console.hide_subprocess_windows);
    }
    virtual void Save(IDESettings& cfg) const override {
        cfg.console.show_welcome_message = ~interface_tab.show_welcome;
        cfg.console.show_calltips = ~interface_tab.show_calltips;
        cfg.console.show_elapsed_time = ~interface_tab.show_elapsed_time;
        cfg.console.confirm_before_closing = ~interface_tab.confirm_closing;
        cfg.console.confirm_before_restarting = ~interface_tab.confirm_restarting;
        cfg.console.confirm_before_removing_variables = ~interface_tab.confirm_remove_vars;
        cfg.console.completion_display = ~interface_tab.completion_display;
        cfg.console.use_jedi_completion = ~interface_tab.use_jedi;
        cfg.console.use_greedy_completion = ~interface_tab.use_greedy;
        cfg.console.output_buffer_lines = ~interface_tab.buffer_lines;
        cfg.console.render_sympy_math = ~interface_tab.render_sympy;

        cfg.console.matplotlib_support = ~plotting_tab.activate_support;
        cfg.console.auto_import_numpy_matplotlib = ~plotting_tab.auto_import;
        cfg.console.graphics_backend = ~plotting_tab.backend;
        cfg.console.inline_format = ~plotting_tab.format;
        cfg.console.inline_resolution_dpi = ScanDouble(AsString(~plotting_tab.resolution));
        cfg.console.inline_width_in = ~plotting_tab.width;
        cfg.console.inline_height_in = ~plotting_tab.height;
        cfg.console.inline_font_points = ScanDouble(AsString(~plotting_tab.font_size));
        cfg.console.inline_bottom_edge = ScanDouble(AsString(~plotting_tab.bottom_edge));
        cfg.console.inline_tight_layout = ~plotting_tab.tight_layout;

        cfg.console.startup_code = ~startup_tab.startup_lines;
        cfg.console.execute_startup_file = ~startup_tab.execute_file;
        cfg.console.startup_file = ~startup_tab.startup_file_path;

        cfg.console.autocall_mode = ~advanced_tab.autocall;
        cfg.console.use_autoreload = ~advanced_tab.use_autoreload;
        cfg.console.input_prompt = ~advanced_tab.input_prompt;
        cfg.console.output_prompt = ~advanced_tab.output_prompt;
        cfg.console.hide_subprocess_windows = ~advanced_tab.hide_cmd_output;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- ProfilerPage ---

class ProfilerPage : public WithProfilerPageLayout<PreferencesPage> {
public:
    typedef ProfilerPage CLASSNAME;
    ProfilerPage() {
        CtrlLayout(*this);
    }
    virtual void Load(const IDESettings& cfg) override {
        open_on_finish.SetData(cfg.profiler.open_on_finish);
        max_items.SetData(cfg.profiler.max_hot_items);
    }
    virtual void Save(IDESettings& cfg) const override {
        cfg.profiler.open_on_finish = ~open_on_finish;
        cfg.profiler.max_hot_items = ~max_items;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- RunPage ---

class RunPage : public WithRunPageLayout<PreferencesPage> {
public:
    typedef RunPage CLASSNAME;
    RunPage() {
        CtrlLayout(*this);
        runner.Add("Internal");
        runner.Add("External terminal");
        presets.AddColumn("Name");
        presets.AddColumn("File extension");
        presets.AddColumn("Context");
    }
    virtual void Load(const IDESettings& cfg) override {
        runner.SetData(cfg.run.selected_runner);
        save_all_before_run.SetData(cfg.run.save_all_before_run);
        copy_to_console.SetData(cfg.run.copy_full_cell_to_console);
        presets.Clear();
        for(const auto& p : cfg.run.presets)
            presets.Add(p.name, p.file_extension, p.context);
    }
    virtual void Save(IDESettings& cfg) const override {
        cfg.run.selected_runner = ~runner;
        cfg.run.save_all_before_run = ~save_all_before_run;
        cfg.run.copy_full_cell_to_console = ~copy_to_console;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- StatusBarPage ---

class StatusBarPage : public WithStatusBarPageLayout<PreferencesPage> {
public:
    typedef StatusBarPage CLASSNAME;
    StatusBarPage() {
        CtrlLayout(*this);
    }
    virtual void Load(const IDESettings& cfg) override {
        show_memory.SetData(cfg.statusbar.show_memory);
        memory_interval.SetData(cfg.statusbar.memory_poll_ms);
        show_cpu.SetData(cfg.statusbar.show_cpu);
        cpu_interval.SetData(cfg.statusbar.cpu_poll_ms);
        show_clock.SetData(cfg.statusbar.show_clock);
    }
    virtual void Save(IDESettings& cfg) const override {
        cfg.statusbar.show_memory = ~show_memory;
        cfg.statusbar.memory_poll_ms = ~memory_interval;
        cfg.statusbar.show_cpu = ~show_cpu;
        cfg.statusbar.cpu_poll_ms = ~cpu_interval;
        cfg.statusbar.show_clock = ~show_clock;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {
        if(ctx.main_window) ctx.main_window->ApplySettings();
    }
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- VariableExplorerPage ---

class VariableExplorerPage : public WithVariableExplorerPageLayout<PreferencesPage> {
public:
    typedef VariableExplorerPage CLASSNAME;
    VariableExplorerPage() {
        CtrlLayout(*this);
    }
    virtual void Load(const IDESettings& cfg) override {
        exclude_private.SetData(cfg.variable_explorer.exclude_private);
        exclude_capitalized.SetData(cfg.variable_explorer.exclude_capitalized);
        exclude_uppercase.SetData(cfg.variable_explorer.exclude_uppercase);
        exclude_unsupported.SetData(cfg.variable_explorer.exclude_unsupported);
        exclude_callables.SetData(cfg.variable_explorer.exclude_callables);
        show_minmax.SetData(cfg.variable_explorer.show_array_minmax);
    }
    virtual void Save(IDESettings& cfg) const override {
        cfg.variable_explorer.exclude_private = ~exclude_private;
        cfg.variable_explorer.exclude_capitalized = ~exclude_capitalized;
        cfg.variable_explorer.exclude_uppercase = ~exclude_uppercase;
        cfg.variable_explorer.exclude_unsupported = ~exclude_unsupported;
        cfg.variable_explorer.exclude_callables = ~exclude_callables;
        cfg.variable_explorer.show_array_minmax = ~show_minmax;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {
        if(ctx.main_window) ctx.main_window->UpdateVariableExplorer();
    }
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- WorkingDirectoryPage ---

class WorkingDirectoryPage : public WithWorkingDirectoryPageLayout<PreferencesPage> {
public:
    typedef WorkingDirectoryPage CLASSNAME;
    WorkingDirectoryPage() {
        CtrlLayout(*this);
        browse_startup.WhenAction = [=] { FileSel fs; if(fs.ExecuteSelectDir()) startup_path.SetData(fs.Get()); };
        browse_console.WhenAction = [=] { FileSel fs; if(fs.ExecuteSelectDir()) console_path.SetData(fs.Get()); };
    }
    virtual void Load(const IDESettings& cfg) override {
        startup_mode.SetData(cfg.working_directory.startup_mode);
        startup_path.SetData(cfg.working_directory.startup_directory);
        console_mode.SetData(cfg.working_directory.new_console_mode);
        console_path.SetData(cfg.working_directory.new_console_directory);
    }
    virtual void Save(IDESettings& cfg) const override {
        cfg.working_directory.startup_mode = ~startup_mode;
        cfg.working_directory.startup_directory = ~startup_path;
        cfg.working_directory.new_console_mode = ~console_mode;
        cfg.working_directory.new_console_directory = ~console_path;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

// --- PluginsPage ---

class PluginsPage : public WithPluginsPageLayout<PreferencesPage> {
public:
    typedef PluginsPage CLASSNAME;
    PluginsPage() {
        CtrlLayout(*this);
        list.AddColumn("ID");
        list.AddColumn("Enabled").Ctrls<Option>();
    }
    virtual void Load(const IDESettings& cfg) override {
        list.Clear();
        for(const auto& ps : cfg.plugins.states)
            list.Add(ps.id, ps.enabled);
    }
    virtual void Save(IDESettings& cfg) const override {
        cfg.plugins.states.Clear();
        for(int i = 0; i < list.GetCount(); i++) {
            auto& ps = cfg.plugins.states.Add();
            ps.id = list.Get(i, 0);
            ps.enabled = list.Get(i, 1);
        }
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {
        PythonIDE* ide = ctx.main_window;
        if(!ide) return;
        
        for(const auto& ps : new_cfg.plugins.states) {
            bool old_enabled = true;
            for(const auto& ops : old_cfg.plugins.states)
                if(ops.id == ps.id) { old_enabled = ops.enabled; break; }
            
            if(ps.enabled != old_enabled) {
                if(ps.enabled)
                    ide->plugin_manager->EnablePlugin(ps.id);
                else
                    ide->plugin_manager->DisablePlugin(ps.id);
            }
        }
    }
    virtual void SetDefaults() override { Load(IDESettings()); }
    virtual bool IsModified() const override { return false; }
};

void PythonIDE::OnSettings()
{
	IDEContext ctx;
	ctx.main_window = this;
	
	PreferencesWindow dlg(ctx, settings);
    
    AppearancePage appearance;
    ApplicationPage application;
    PythonInterpreterPage python;
    KeyboardShortcutsPage shortcuts;
    CodeAnalysisPage code_analysis;
    CompletionPage completion;
    DebuggerPage debugger;
    EditorPage editor;
    FilesPage files;
    HelpPage help;
    HistoryPage history;
    IPythonConsolePage console;
    ProfilerPage profiler;
    RunPage run;
    StatusBarPage statusbar_p;
    VariableExplorerPage var_explorer_p;
    WorkingDirectoryPage work_dir;
    PluginsPage plugins;

    dlg.AddPage("appearance", "Appearance", Image(), &appearance);
    dlg.AddPage("application", "Application", Image(), &application);
    dlg.AddPage("python", "Python interpreter", Image(), &python);
    dlg.AddPage("shortcuts", "Keyboard shortcuts", Image(), &shortcuts);
    dlg.AddPage("code_analysis", "Code Analysis", Image(), &code_analysis);
    dlg.AddPage("completion", "Completion and linting", Image(), &completion);
    dlg.AddPage("debugger", "Debugger", Image(), &debugger);
    dlg.AddPage("editor", "Editor", Image(), &editor);
    dlg.AddPage("files", "Files", Image(), &files);
    dlg.AddPage("help", "Help", Image(), &help);
    dlg.AddPage("history", "History", Image(), &history);
    dlg.AddPage("console", "IPython console", Image(), &console);
    dlg.AddPage("profiler", "Profiler", Image(), &profiler);
    dlg.AddPage("run", "Run", Image(), &run);
    dlg.AddPage("statusbar", "Status bar", Image(), &statusbar_p);
    dlg.AddPage("variable_explorer", "Variable explorer", Image(), &var_explorer_p);
    dlg.AddPage("working_directory", "Working directory", Image(), &work_dir);
	dlg.AddPage("plugins", "Plugins", Image(), &plugins);

	if(dlg.Run() == IDOK) {
		ApplySettings();
		StoreToFile(settings, ConfigFile("ide_settings.bin"));
	}
}

END_UPP_NAMESPACE
