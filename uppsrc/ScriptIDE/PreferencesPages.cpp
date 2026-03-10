#include "ScriptIDE.h"

namespace Upp {

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
        interface_theme.SetData(cfg.appearance.interface_theme); syntax_theme.SetData(cfg.appearance.syntax_theme);
        monospace_font.SetData(cfg.appearance.monospace_font_face); monospace_size.SetData(cfg.appearance.monospace_font_size);
        interface_font.SetData(cfg.appearance.interface_font_face); interface_size.SetData(cfg.appearance.interface_font_size);
        use_system_font.SetData(cfg.appearance.use_system_interface_font);
        UpdatePreviews();
    }
    virtual void Save(IDESettings& cfg) const override {
        cfg.appearance.interface_theme = ~interface_theme; cfg.appearance.syntax_theme = ~syntax_theme;
        cfg.appearance.monospace_font_face = ~monospace_font; cfg.appearance.monospace_font_size = ~monospace_size;
        cfg.appearance.interface_font_face = ~interface_font; cfg.appearance.interface_font_size = ~interface_size;
        cfg.appearance.use_system_interface_font = ~use_system_font;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override { if(ctx.main_window) ctx.main_window->ApplySettings(); }
    virtual void SetDefaults() override { AppearanceSettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const AppearanceSettings& cfg) {
        interface_theme.SetData(cfg.interface_theme); syntax_theme.SetData(cfg.syntax_theme);
        monospace_font.SetData(cfg.monospace_font_face); monospace_size.SetData(cfg.monospace_font_size);
        interface_font.SetData(cfg.interface_font_face); interface_size.SetData(cfg.interface_font_size);
        use_system_font.SetData(cfg.use_system_interface_font);
        UpdatePreviews();
    }
    void UpdatePreviews() {
        Font mf = Font(Font::FindFaceNameIndex(~monospace_font), ~monospace_size);
        Font ifnt = Font(Font::FindFaceNameIndex(~interface_font), ~interface_size);
        editor_lbl.SetFont(mf);
        interface_font_lbl.SetFont(ifnt);
    }
};

class ApplicationPage : public PreferencesPage {
public:
    TabCtrl tabs;
    struct InterfaceTab : public WithApplicationInterfaceTabLayout<ParentCtrl> { InterfaceTab() { CtrlLayout(*this); } } interface_tab;
    struct AdvancedTab : public WithApplicationAdvancedTabLayout<ParentCtrl> { AdvancedTab() { CtrlLayout(*this); language.Add("English"); rendering_engine.Add("Default"); } } advanced_tab;
    ApplicationPage() { Add(tabs.SizePos()); tabs.Add(interface_tab, "Interface"); tabs.Add(advanced_tab, "Advanced settings"); }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.application); }
    virtual void Save(IDESettings& cfg) const override {
        cfg.application.hidpi_mode = ~interface_tab.hidpi_mode; cfg.application.custom_scale = ~interface_tab.custom_scale;
        cfg.application.show_friendly_empty_messages = ~interface_tab.show_friendly_empty;
        cfg.application.vertical_tabs_in_panes = ~interface_tab.vertical_tabs;
        cfg.application.custom_margin = ~interface_tab.custom_margin;
        cfg.application.pane_margin = ~interface_tab.pane_margin;
        cfg.application.custom_cursor_blink = ~interface_tab.cursor_blinking;
        cfg.application.cursor_blink_ms = ~interface_tab.blink_ms;
        cfg.application.language = ~advanced_tab.language;
        cfg.application.rendering_engine = ~advanced_tab.rendering_engine; cfg.application.single_instance = ~advanced_tab.single_instance;
        cfg.application.prompt_on_exit = ~advanced_tab.prompt_exit;
        cfg.application.show_internal_errors = ~advanced_tab.show_internal_errors;
        cfg.application.check_updates_on_startup = ~advanced_tab.check_updates;
        cfg.application.stable_releases_only = ~advanced_tab.stable_only;
        cfg.application.disable_ctrl_wheel_zoom = ~advanced_tab.disable_wheel_zoom;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { ApplicationSettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const ApplicationSettings& cfg) {
        interface_tab.hidpi_mode.SetData(cfg.hidpi_mode); interface_tab.custom_scale.SetData(cfg.custom_scale);
        interface_tab.show_friendly_empty.SetData(cfg.show_friendly_empty_messages);
        interface_tab.vertical_tabs.SetData(cfg.vertical_tabs_in_panes);
        interface_tab.custom_margin.SetData(cfg.custom_margin);
        interface_tab.pane_margin.SetData(cfg.pane_margin);
        interface_tab.cursor_blinking.SetData(cfg.custom_cursor_blink);
        interface_tab.blink_ms.SetData(cfg.cursor_blink_ms);
        advanced_tab.language.SetData(cfg.language);
        advanced_tab.rendering_engine.SetData(cfg.rendering_engine); advanced_tab.single_instance.SetData(cfg.single_instance);
        advanced_tab.prompt_exit.SetData(cfg.prompt_on_exit);
        advanced_tab.show_internal_errors.SetData(cfg.show_internal_errors);
        advanced_tab.check_updates.SetData(cfg.check_updates_on_startup);
        advanced_tab.stable_only.SetData(cfg.stable_releases_only);
        advanced_tab.disable_wheel_zoom.SetData(cfg.disable_ctrl_wheel_zoom);
    }
};

class PythonInterpreterPage : public WithPythonInterpreterPageLayout<PreferencesPage> {
public:
    PythonInterpreterPage() {
        CtrlLayout(*this);
        browse_interpreter.WhenAction = [=] { FileSel fs; if(fs.ExecuteOpen("Select Interpreter")) interpreter_path.SetData(fs.Get()); };
        browse_conda.WhenAction = [=] { FileSel fs; if(fs.ExecuteOpen("Select Conda Executable")) conda_path.SetData(fs.Get()); };
    }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.python); }
    virtual void Save(IDESettings& cfg) const override {
        cfg.python.use_internal = ((int)~interpreter_mode == 0);
        cfg.python.interpreter_path = ~interpreter_path;
        cfg.python.use_custom_conda = ~use_custom_conda;
        cfg.python.conda_executable = ~conda_path;
        cfg.python.umr_enabled = ~umr_enabled;
        cfg.python.umr_verbose = ~umr_verbose;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { PythonInterpreterSettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const PythonInterpreterSettings& cfg) {
        interpreter_mode.SetData(cfg.use_internal ? 0 : 1);
        interpreter_path.SetData(cfg.interpreter_path);
        use_custom_conda.SetData(cfg.use_custom_conda);
        conda_path.SetData(cfg.conda_executable);
        umr_enabled.SetData(cfg.umr_enabled);
        umr_verbose.SetData(cfg.umr_verbose);
    }
};

class KeyboardShortcutsPage : public WithShortcutsPageLayout<PreferencesPage> {
public:
    KeyboardShortcutsPage() {
        CtrlLayout(*this);
        list.AddColumn("Context"); list.AddColumn("Name"); list.AddColumn("Shortcut");
        search.WhenAction = [=] { DoSearch(); };
        search_btn.WhenAction = [=] { DoSearch(); };
    }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.shortcuts); original_items <<= cfg.shortcuts.items; }
    virtual void Save(IDESettings& cfg) const override {
        cfg.shortcuts.items <<= original_items;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { ShortcutSettings d; LoadSettings(d); original_items <<= d.items; }
    virtual bool IsModified() const override { return true; }
private:
    mutable Vector<ShortcutItem> original_items;
    void LoadSettings(const ShortcutSettings& cfg) {
        list.Clear();
        for(const auto& item : cfg.items)
            list.Add(item.context, item.action_id, GetKeyDesc(item.key));
    }
    void DoSearch() {
        String s = ToLower((String)~search);
        list.Clear();
        for(const auto& item : original_items) {
            if(s.IsEmpty() || ToLower(item.context).Find(s) >= 0 || ToLower(item.action_id).Find(s) >= 0)
                list.Add(item.context, item.action_id, GetKeyDesc(item.key));
        }
    }
};

class CodeAnalysisPage : public WithCodeAnalysisPageLayout<PreferencesPage> {
public:
    CodeAnalysisPage() { CtrlLayout(*this); }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.code_analysis); }
    virtual void Save(IDESettings& cfg) const override {
        cfg.code_analysis.save_before_analysis = ~save_before;
        cfg.code_analysis.history_results = ~history_count;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { CodeAnalysisSettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const CodeAnalysisSettings& cfg) {
        save_before.SetData(cfg.save_before_analysis);
        history_count.SetData(cfg.history_results);
        results_path.SetLabel(cfg.results_path);
    }
};

class CompletionLintingPage : public PreferencesPage {
public:
    TabCtrl tabs;
    struct GeneralTab : public WithCompletionGeneralTabLayout<ParentCtrl> { GeneralTab() { CtrlLayout(*this); } } general_tab;
    struct LintingTab : public WithCompletionLintingTabLayout<ParentCtrl> { LintingTab() { CtrlLayout(*this); } } linting_tab;
    struct IntrospectionTab : public WithCompletionIntrospectionTabLayout<ParentCtrl> { IntrospectionTab() { CtrlLayout(*this); } } introspection_tab;
    struct FormattingTab : public WithCompletionFormattingTabLayout<ParentCtrl> { FormattingTab() { CtrlLayout(*this); formatter.Add("autopep8"); formatter.Add("black"); } } formatting_tab;
    struct AdvancedTab : public WithCompletionAdvancedTabLayout<ParentCtrl> { AdvancedTab() { CtrlLayout(*this); } } advanced_tab;
    struct OtherLanguagesTab : public WithCompletionOtherLanguagesTabLayout<ParentCtrl> { OtherLanguagesTab() { CtrlLayout(*this); servers.AddColumn("Language"); servers.AddColumn("Address"); servers.AddColumn("Command"); } } other_tab;
    struct SnippetsTab : public WithCompletionSnippetsTabLayout<ParentCtrl> { SnippetsTab() { CtrlLayout(*this); snippet_lang.Add("Python"); snippet_lang.Add("C++"); snippets.AddColumn("Trigger"); snippets.AddColumn("Description"); } } snippets_tab;

    CompletionLintingPage() {
        Add(tabs.SizePos());
        tabs.Add(general_tab, "General");
        tabs.Add(linting_tab, "Linting");
        tabs.Add(introspection_tab, "Introspection");
        tabs.Add(formatting_tab, "Code formatting");
        tabs.Add(advanced_tab, "Advanced");
        tabs.Add(other_tab, "Other languages");
        tabs.Add(snippets_tab, "Snippets");

        snippets_tab.snippet_lang.WhenAction = [=] { SyncSnippets(); LoadSnippetList(); };
    }

    virtual void Load(const IDESettings& cfg) override { all_snippets <<= cfg.completion.snippets; LoadSettings(cfg.completion); }
    virtual void Save(IDESettings& cfg) const override {
        SyncSnippets();
        cfg.completion.show_completion_details = ~general_tab.show_details;
        cfg.completion.enable_code_snippets = ~general_tab.enable_snippets;
        cfg.completion.show_completions_on_the_fly = ~general_tab.completions_fly;
        cfg.completion.chars_before_completion = ~general_tab.chars_before;
        cfg.completion.completion_detail_delay_ms = ~general_tab.detail_delay;
        cfg.completion.provider_timeout_ms = ~general_tab.timeout;
        cfg.completion.enable_fallback_provider = ~general_tab.enable_fallback;
        cfg.completion.enable_lsp_provider = ~general_tab.enable_lsp;
        cfg.completion.enable_snippet_provider = ~general_tab.enable_snippets_provider;

        cfg.completion.lint_provider = ~linting_tab.provider;
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

        cfg.completion.external_servers.Clear();
        for(int i = 0; i < other_tab.servers.GetCount(); i++) {
            ExternalLSPServer& s = cfg.completion.external_servers.Add();
            s.language = other_tab.servers.Get(i, 0);
            s.address = other_tab.servers.Get(i, 1);
            s.command = other_tab.servers.Get(i, 2);
        }

        cfg.completion.snippets <<= all_snippets;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { CompletionLintingSettings d; all_snippets <<= d.snippets; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    mutable Vector<Snippet> all_snippets;
    void LoadSettings(const CompletionLintingSettings& cfg) {
        general_tab.show_details.SetData(cfg.show_completion_details);
        general_tab.enable_snippets.SetData(cfg.enable_code_snippets);
        general_tab.completions_fly.SetData(cfg.show_completions_on_the_fly);
        general_tab.chars_before.SetData(cfg.chars_before_completion);
        general_tab.detail_delay.SetData(cfg.completion_detail_delay_ms);
        general_tab.timeout.SetData(cfg.provider_timeout_ms);
        general_tab.enable_fallback.SetData(cfg.enable_fallback_provider);
        general_tab.enable_lsp.SetData(cfg.enable_lsp_provider);
        general_tab.enable_snippets_provider.SetData(cfg.enable_snippet_provider);

        linting_tab.provider.SetData(cfg.lint_provider);
        linting_tab.underline_errors.SetData(cfg.underline_errors);

        introspection_tab.enable_goto.SetData(cfg.enable_go_to_definition);
        introspection_tab.follow_imports.SetData(cfg.follow_imports);
        introspection_tab.show_calltips.SetData(cfg.show_calltips);
        introspection_tab.enable_hover.SetData(cfg.enable_hover_hints);
        introspection_tab.preload_modules.SetData(cfg.preload_modules_csv);

        formatting_tab.formatter.SetData(cfg.formatter);
        formatting_tab.autoformat_save.SetData(cfg.autoformat_on_save);
        formatting_tab.max_line_length.SetData(cfg.max_line_length);
        formatting_tab.show_ruler.SetData(cfg.show_vertical_ruler);

        advanced_tab.lsp_advanced.SetData(cfg.lsp_advanced_enabled);
        advanced_tab.lsp_module.SetData(cfg.lsp_module);
        advanced_tab.lsp_address.SetData(cfg.lsp_address);
        advanced_tab.lsp_port.SetData(cfg.lsp_port);
        advanced_tab.lsp_external.SetData(cfg.lsp_external_server);
        advanced_tab.lsp_stdio.SetData(cfg.lsp_use_stdio);

        other_tab.servers.Clear();
        for(const auto& s : cfg.external_servers)
            other_tab.servers.Add(s.language, s.address, s.command);

        LoadSnippetList();
    }
    void SyncSnippets() const {
        String lang = ~snippets_tab.snippet_lang;
        if(lang.IsEmpty()) return;
        // Remove existing snippets for this language
        for(int i = all_snippets.GetCount() - 1; i >= 0; i--)
            if(all_snippets[i].language == lang)
                all_snippets.Remove(i);
        // Add current snippets from list
        for(int i = 0; i < snippets_tab.snippets.GetCount(); i++) {
            Snippet& s = all_snippets.Add();
            s.language = lang;
            s.trigger = snippets_tab.snippets.Get(i, 0);
            s.description = snippets_tab.snippets.Get(i, 1);
        }
    }
    void LoadSnippetList() {
        String lang = ~snippets_tab.snippet_lang;
        snippets_tab.snippets.Clear();
        for(const auto& s : all_snippets)
            if(s.language == lang)
                snippets_tab.snippets.Add(s.trigger, s.description);
    }
};

class DebuggerPage : public WithDebuggerPageLayout<PreferencesPage> {
public:
    DebuggerPage() { CtrlLayout(*this); }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.debugger); }
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
    virtual void SetDefaults() override { DebuggerSettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const DebuggerSettings& cfg) {
        prevent_close.SetData(cfg.prevent_close_while_debugging);
        stop_first_line.SetData(cfg.stop_on_first_line_without_breakpoints);
        ignore_libs.SetData(cfg.ignore_python_libraries);
        process_events.SetData(cfg.process_execute_events);
        exclamation_prefix.SetData(cfg.use_exclamation_prefix);
        debug_lines.SetData(cfg.preload_debug_lines);
        exclude_internal.SetData(cfg.exclude_internal_frames);
    }
};

class EditorPage : public PreferencesPage {
public:
    TabCtrl tabs;
    struct DisplayTab : public WithEditorDisplayTabLayout<ParentCtrl> { DisplayTab() { CtrlLayout(*this); } } display_tab;
    struct SourceCodeTab : public WithEditorSourceCodeTabLayout<ParentCtrl> { SourceCodeTab() { CtrlLayout(*this); eol_mode.Add("LF"); eol_mode.Add("CRLF"); } } source_tab;
    struct AdvancedTab : public WithEditorAdvancedTabLayout<ParentCtrl> { AdvancedTab() { CtrlLayout(*this); docstring_style.Add("Numpy"); docstring_style.Add("Google"); docstring_style.Add("Sphinx"); } } advanced_tab;
    EditorPage() {
        Add(tabs.SizePos());
        tabs.Add(display_tab, "Display");
        tabs.Add(source_tab, "Source code");
        tabs.Add(advanced_tab, "Advanced");
    }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.editor); }
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
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override { if(ctx.main_window) ctx.main_window->ApplySettings(); }
    virtual void SetDefaults() override { EditorSettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const EditorSettings& cfg) {
        display_tab.show_tab_bar.SetData(cfg.show_tab_bar);
        display_tab.show_path.SetData(cfg.show_full_path_above_editor);
        display_tab.show_selector.SetData(cfg.show_class_function_selector);
        display_tab.allow_scroll_past_eof.SetData(cfg.allow_scroll_past_eof);
        display_tab.show_indent_guides.SetData(cfg.show_indent_guides);
        display_tab.show_folding.SetData(cfg.show_code_folding);
        display_tab.show_line_numbers.SetData(cfg.show_line_numbers);
        display_tab.show_breakpoints.SetData(cfg.show_debugger_breakpoints);
        display_tab.show_annotations.SetData(cfg.show_code_annotations);
        display_tab.highlight_current_line.SetData(cfg.highlight_current_line);
        display_tab.highlight_current_cell.SetData(cfg.highlight_current_cell);
        display_tab.highlight_occurrences.SetData(cfg.highlight_selected_occurrences);
        display_tab.occurrence_delay.SetData(cfg.occurrence_highlight_delay_ms);

        source_tab.auto_close_brackets.SetData(cfg.auto_insert_closing_brackets);
        source_tab.auto_close_quotes.SetData(cfg.auto_insert_closing_quotes);
        source_tab.auto_colons.SetData(cfg.auto_insert_colons);
        source_tab.auto_unindent.SetData(cfg.auto_unindent_keywords);
        source_tab.strip_trailing.SetData(cfg.strip_trailing_spaces_on_save);
        source_tab.strip_changed.SetData(cfg.strip_trailing_spaces_changed_lines);
        source_tab.add_eof_newline.SetData(cfg.add_missing_newline_eof);
        source_tab.strip_eof_blank.SetData(cfg.strip_blank_lines_eof);
        source_tab.tab_width.SetData(cfg.tab_width);
        source_tab.indent_spaces.SetData(cfg.indent_with_spaces);
        source_tab.intelligent_backspace.SetData(cfg.intelligent_backspace);
        source_tab.tab_always_indents.SetData(cfg.tab_always_indents);
        source_tab.fix_mixed_eol.SetData(cfg.fix_mixed_eol);
        source_tab.convert_eol_on_save.SetData(cfg.convert_eol_on_save);
        source_tab.eol_mode.SetData(cfg.eol_mode);

        advanced_tab.autosave_enabled.SetData(cfg.autosave_backup_unsaved);
        advanced_tab.autosave_interval.SetData(cfg.autosave_interval_sec);
        advanced_tab.docstring_style.SetData(cfg.docstring_style);
        advanced_tab.enable_multicursor.SetData(cfg.enable_multicursor);
        advanced_tab.multicursor_paste.SetData(cfg.multicursor_paste_mode);
    }
};

class FilesPage : public PreferencesPage {
public:
    TabCtrl tabs;
    struct GeneralTab : public WithFilesGeneralTabLayout<ParentCtrl> { GeneralTab() { CtrlLayout(*this); } } general_tab;
    struct AssociationsTab : public WithFilesAssociationsTabLayout<ParentCtrl> { AssociationsTab() { CtrlLayout(*this); file_types.AddColumn("Extension"); apps.AddColumn("Application"); } } associations_tab;
    FilesPage() { Add(tabs.SizePos()); tabs.Add(general_tab, "General"); tabs.Add(associations_tab, "File associations"); }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.files); original_associations <<= cfg.files.associations; }
    virtual void Save(IDESettings& cfg) const override {
        cfg.files.show_hidden_files = ~general_tab.show_hidden;
        cfg.files.single_click_open = ~general_tab.single_click;
        cfg.files.filter_patterns_csv = ~general_tab.filter_patterns;
        cfg.files.associations <<= original_associations;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { FilesSettings d; LoadSettings(d); original_associations <<= d.associations; }
    virtual bool IsModified() const override { return true; }
private:
    mutable Vector<FileAssociation> original_associations;
    void LoadSettings(const FilesSettings& cfg) {
        general_tab.show_hidden.SetData(cfg.show_hidden_files);
        general_tab.single_click.SetData(cfg.single_click_open);
        general_tab.filter_patterns.SetData(cfg.filter_patterns_csv);
        associations_tab.file_types.Clear();
        for(const auto& a : cfg.associations) associations_tab.file_types.Add(a.extension);
    }
};

class HelpPage : public WithHelpPageLayout<PreferencesPage> {
public:
    HelpPage() { CtrlLayout(*this); }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.help); }
    virtual void Save(IDESettings& cfg) const override {
        cfg.help.auto_connect_editor = ~connect_editor;
        cfg.help.auto_connect_console = ~connect_console;
        cfg.help.render_math = ~render_math;
        cfg.help.wrap_lines = ~wrap_lines;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { HelpSettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const HelpSettings& cfg) {
        connect_editor.SetData(cfg.auto_connect_editor);
        connect_console.SetData(cfg.auto_connect_console);
        render_math.SetData(cfg.render_math);
        wrap_lines.SetData(cfg.wrap_lines);
    }
};

class HistoryPage : public WithHistoryPageLayout<PreferencesPage> {
public:
    HistoryPage() { CtrlLayout(*this); }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.history); }
    virtual void Save(IDESettings& cfg) const override {
        cfg.history.wrap_lines = ~wrap_lines;
        cfg.history.show_line_numbers = ~show_line_numbers;
        cfg.history.scroll_to_last_entry = ~scroll_to_last;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { HistorySettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const HistorySettings& cfg) {
        wrap_lines.SetData(cfg.wrap_lines);
        show_line_numbers.SetData(cfg.show_line_numbers);
        scroll_to_last.SetData(cfg.scroll_to_last_entry);
    }
};

class IPythonConsolePage : public PreferencesPage {
public:
    TabCtrl tabs;
    struct InterfaceTab : public WithConsoleInterfaceTabLayout<ParentCtrl> { InterfaceTab() { CtrlLayout(*this); completion_display.Add("Graphical"); completion_display.Add("Terminal"); } } interface_tab;
    struct PlottingTab : public WithConsolePlottingTabLayout<ParentCtrl> { PlottingTab() { CtrlLayout(*this); backend.Add("Inline"); backend.Add("Automatic"); backend.Add("Tkinter"); backend.Add("Qt5"); format.Add("PNG"); format.Add("SVG"); format.Add("JPG"); } } plotting_tab;
    struct StartupTab : public WithConsoleStartupTabLayout<ParentCtrl> { StartupTab() { CtrlLayout(*this); browse_startup_file.WhenAction = [=] { FileSel fs; if(fs.ExecuteOpen("Select Startup File")) startup_file_path.SetData(fs.Get()); }; } } startup_tab;
    struct AdvancedTab : public WithConsoleAdvancedTabLayout<ParentCtrl> { AdvancedTab() { CtrlLayout(*this); autocall.Add("Off"); autocall.Add("On"); autocall.Add("Smart"); } } advanced_tab;

    IPythonConsolePage() {
        Add(tabs.SizePos());
        tabs.Add(interface_tab, "Interface");
        tabs.Add(plotting_tab, "Plotting");
        tabs.Add(startup_tab, "Startup");
        tabs.Add(advanced_tab, "Advanced");
    }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.console); }
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
        cfg.console.inline_resolution_dpi = ScanDouble((String)~plotting_tab.resolution);
        cfg.console.inline_width_in = ScanDouble((String)~plotting_tab.width);
        cfg.console.inline_height_in = ScanDouble((String)~plotting_tab.height);
        cfg.console.inline_font_points = ScanDouble((String)~plotting_tab.font_size);
        cfg.console.inline_bottom_edge = ScanDouble((String)~plotting_tab.bottom_edge);
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
    virtual void SetDefaults() override { IPythonConsoleSettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const IPythonConsoleSettings& cfg) {
        interface_tab.show_welcome.SetData(cfg.show_welcome_message);
        interface_tab.show_calltips.SetData(cfg.show_calltips);
        interface_tab.show_elapsed_time.SetData(cfg.show_elapsed_time);
        interface_tab.confirm_closing.SetData(cfg.confirm_before_closing);
        interface_tab.confirm_restarting.SetData(cfg.confirm_before_restarting);
        interface_tab.confirm_remove_vars.SetData(cfg.confirm_before_removing_variables);
        interface_tab.completion_display.SetData(cfg.completion_display);
        interface_tab.use_jedi.SetData(cfg.use_jedi_completion);
        interface_tab.use_greedy.SetData(cfg.use_greedy_completion);
        interface_tab.buffer_lines.SetData(cfg.output_buffer_lines);
        interface_tab.render_sympy.SetData(cfg.render_sympy_math);

        plotting_tab.activate_support.SetData(cfg.matplotlib_support);
        plotting_tab.auto_import.SetData(cfg.auto_import_numpy_matplotlib);
        plotting_tab.backend.SetData(cfg.graphics_backend);
        plotting_tab.format.SetData(cfg.inline_format);
        plotting_tab.resolution.SetData(FormatDouble(cfg.inline_resolution_dpi));
        plotting_tab.width.SetData(FormatDouble(cfg.inline_width_in));
        plotting_tab.height.SetData(FormatDouble(cfg.inline_height_in));
        plotting_tab.font_size.SetData(FormatDouble(cfg.inline_font_points));
        plotting_tab.bottom_edge.SetData(FormatDouble(cfg.inline_bottom_edge));
        plotting_tab.tight_layout.SetData(cfg.inline_tight_layout);

        startup_tab.startup_lines.SetData(cfg.startup_code);
        startup_tab.execute_file.SetData(cfg.execute_startup_file);
        startup_tab.startup_file_path.SetData(cfg.startup_file);

        advanced_tab.autocall.SetData(cfg.autocall_mode);
        advanced_tab.use_autoreload.SetData(cfg.use_autoreload);
        advanced_tab.input_prompt.SetData(cfg.input_prompt);
        advanced_tab.output_prompt.SetData(cfg.output_prompt);
        advanced_tab.hide_cmd_output.SetData(cfg.hide_subprocess_windows);
    }
};

class ProfilerPage : public WithProfilerPageLayout<PreferencesPage> {
public:
    ProfilerPage() { CtrlLayout(*this); }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.profiler); }
    virtual void Save(IDESettings& cfg) const override {
        cfg.profiler.open_on_finish = ~open_on_finish;
        cfg.profiler.max_hot_items = ~max_items;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { ProfilerSettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const ProfilerSettings& cfg) {
        open_on_finish.SetData(cfg.open_on_finish);
        max_items.SetData(cfg.max_hot_items);
    }
};

class RunPage : public WithRunPageLayout<PreferencesPage> {
public:
    RunPage() {
        CtrlLayout(*this);
        runner.Add("Internal");
        runner.Add("External Console");
        runner.Add("Dedicated Terminal");
        presets.AddColumn("Name");
        presets.AddColumn("File Extension");
        presets.AddColumn("Context");
    }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.run); }
    virtual void Save(IDESettings& cfg) const override {
        cfg.run.selected_runner = ~runner;
        cfg.run.save_all_before_run = ~save_all_before_run;
        cfg.run.copy_full_cell_to_console = ~copy_to_console;
        cfg.run.presets.Clear();
        for(int i = 0; i < presets.GetCount(); i++) {
            RunPreset& p = cfg.run.presets.Add();
            p.name = presets.Get(i, 0);
            p.file_extension = presets.Get(i, 1);
            p.context = presets.Get(i, 2);
        }
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { RunSettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const RunSettings& cfg) {
        runner.SetData(cfg.selected_runner);
        save_all_before_run.SetData(cfg.save_all_before_run);
        copy_to_console.SetData(cfg.copy_full_cell_to_console);
        presets.Clear();
        for(const auto& p : cfg.presets) presets.Add(p.name, p.file_extension, p.context);
    }
};

class StatusBarPage : public WithStatusBarPageLayout<PreferencesPage> {
public:
    StatusBarPage() { CtrlLayout(*this); }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.statusbar); }
    virtual void Save(IDESettings& cfg) const override {
        cfg.statusbar.show_memory = ~show_memory;
        cfg.statusbar.memory_poll_ms = ~memory_interval;
        cfg.statusbar.show_cpu = ~show_cpu;
        cfg.statusbar.cpu_poll_ms = ~cpu_interval;
        cfg.statusbar.show_clock = ~show_clock;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { StatusBarSettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const StatusBarSettings& cfg) {
        show_memory.SetData(cfg.show_memory);
        memory_interval.SetData(cfg.memory_poll_ms);
        show_cpu.SetData(cfg.show_cpu);
        cpu_interval.SetData(cfg.cpu_poll_ms);
        show_clock.SetData(cfg.show_clock);
    }
};

class VariableExplorerPage : public WithVariableExplorerPageLayout<PreferencesPage> {
public:
    VariableExplorerPage() { CtrlLayout(*this); }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.variable_explorer); }
    virtual void Save(IDESettings& cfg) const override {
        cfg.variable_explorer.exclude_private = ~exclude_private;
        cfg.variable_explorer.exclude_capitalized = ~exclude_capitalized;
        cfg.variable_explorer.exclude_uppercase = ~exclude_uppercase;
        cfg.variable_explorer.exclude_unsupported = ~exclude_unsupported;
        cfg.variable_explorer.exclude_callables = ~exclude_callables;
        cfg.variable_explorer.show_array_minmax = ~show_minmax;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override { if(ctx.main_window) ctx.main_window->UpdateVariableExplorer(); }
    virtual void SetDefaults() override { VariableExplorerSettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const VariableExplorerSettings& cfg) {
        exclude_private.SetData(cfg.exclude_private);
        exclude_capitalized.SetData(cfg.exclude_capitalized);
        exclude_uppercase.SetData(cfg.exclude_uppercase);
        exclude_unsupported.SetData(cfg.exclude_unsupported);
        exclude_callables.SetData(cfg.exclude_callables);
        show_minmax.SetData(cfg.show_array_minmax);
    }
};

class WorkingDirectoryPage : public WithWorkingDirectoryPageLayout<PreferencesPage> {
public:
    WorkingDirectoryPage() {
        CtrlLayout(*this);
        browse_startup.WhenAction = [=] { FileSel fs; if(fs.ExecuteSelectDir("Select Directory")) startup_path.SetData(fs.Get()); };
        browse_console.WhenAction = [=] { FileSel fs; if(fs.ExecuteSelectDir("Select Directory")) console_path.SetData(fs.Get()); };
    }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.working_directory); }
    virtual void Save(IDESettings& cfg) const override {
        cfg.working_directory.startup_mode = ~startup_mode;
        cfg.working_directory.startup_directory = ~startup_path;
        cfg.working_directory.new_console_mode = ~console_mode;
        cfg.working_directory.new_console_directory = ~console_path;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { WorkingDirectorySettings d; LoadSettings(d); }
    virtual bool IsModified() const override { return true; }
private:
    void LoadSettings(const WorkingDirectorySettings& cfg) {
        startup_mode.SetData(cfg.startup_mode);
        startup_path.SetData(cfg.startup_directory);
        console_mode.SetData(cfg.new_console_mode);
        console_path.SetData(cfg.new_console_directory);
    }
};

class PluginsPage : public WithPluginsPageLayout<PreferencesPage> {
public:
    PluginsPage() {
        CtrlLayout(*this);
        list.AddColumn("Name"); list.AddColumn("Enabled").Ctrls<Option>();
        search.WhenAction = [=] { DoSearch(); };
    }
    virtual void Load(const IDESettings& cfg) override { LoadSettings(cfg.plugins); original_states <<= cfg.plugins.states; }
    virtual void Save(IDESettings& cfg) const override {
        SyncStates();
        cfg.plugins.states <<= original_states;
    }
    virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {}
    virtual void SetDefaults() override { PluginSettings d; LoadSettings(d); original_states <<= d.states; }
    virtual bool IsModified() const override { return true; }
private:
    mutable Vector<PluginState> original_states;
    void LoadSettings(const PluginSettings& cfg) {
        list.Clear();
        for(const auto& p : cfg.states) list.Add(p.id, p.enabled);
    }
    void SyncStates() const {
        for(int i = 0; i < list.GetCount(); i++) {
            String id = list.Get(i, 0);
            bool enabled = list.Get(i, 1);
            for(auto& p : original_states) {
                if(p.id == id) {
                    p.enabled = enabled;
                    break;
                }
            }
        }
    }
    void DoSearch() {
        SyncStates();
        String s = ToLower((String)~search);
        list.Clear();
        for(const auto& p : original_states) {
            if(s.IsEmpty() || ToLower(p.id).Find(s) >= 0)
                list.Add(p.id, p.enabled);
        }
    }
};

class PluginsPreferencePage : public PreferencesPage {
public:
	PluginsPreferencePage() {
		Add(layout.SizePos());
		CtrlLayout(layout);
		layout.list.AddColumn("Enabled", 15).Ctrls<Option>();
		layout.list.AddColumn("Name", 30).Sorting();
		layout.list.AddColumn("Version", 15).Sorting();
		layout.list.AddColumn("Description", 40);
		layout.list.AllSorting();

		layout.search.WhenAction = [=] { OnSearch(); };
	}

	virtual void Load(const IDESettings& s) override {
		original_states <<= s.plugins.states;
		UpdateList();
	}

	virtual void Save(IDESettings& s) const override {
		s.plugins.states.Clear();
		for(int i = 0; i < layout.list.GetCount(); i++) {
			PluginState& ps = s.plugins.states.Add();
			ps.id = layout.list.Get(i, 1);
			ps.enabled = layout.list.Get(i, 0);
		}
	}

	virtual void Apply(IDEContext& ctx, const IDESettings& old_cfg, const IDESettings& new_cfg) override {
		if(PythonIDE* ide = ctx.main_window) {
			for(const auto& ps : new_cfg.plugins.states) {
				if(ps.enabled)
					ide->plugin_manager->EnablePlugin(ps.id);
				else
					ide->plugin_manager->DisablePlugin(ps.id);
			}
		}
	}

	virtual bool IsModified() const override {
		// Simplified
		return true; 
	}

	virtual void SetDefaults() override {
		// All enabled by default
		for(int i = 0; i < layout.list.GetCount(); i++)
			layout.list.Set(i, 0, true);
	}

private:
	WithPluginsPageLayout<ParentCtrl> layout;
	mutable Vector<PluginState> original_states;

	void UpdateList() {
		layout.list.Clear();
		if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
			const auto& plugins = ide->plugin_manager->GetPlugins();
			for(int i = 0; i < plugins.GetCount(); i++) {
				String id = plugins.GetKey(i);
				const IPlugin& p = *plugins[i].plugin;
				bool enabled = false;
				for(const auto& ps : original_states) {
					if(ps.id == id) {
						enabled = ps.enabled;
						break;
					}
				}
				layout.list.Add(enabled, id, "1.0", p.GetDescription());
			}
		}
	}

	void OnSearch() {
		String text = ToLower((String)~layout.search);
		for(int i = 0; i < layout.list.GetCount(); i++) {
			bool match = ToLower((String)layout.list.Get(i, 1)).Find(text) >= 0 ||
			             ToLower((String)layout.list.Get(i, 3)).Find(text) >= 0;
			layout.list.ShowLine(i, match);
		}
	}
};

void PythonIDE::OnSettings()
{
	IDEContext ctx; ctx.main_window = this;
	PreferencesWindow dlg(ctx, settings);
	AppearancePage appearance; ApplicationPage application; PythonInterpreterPage python; KeyboardShortcutsPage shortcuts;
	CodeAnalysisPage analysis; CompletionLintingPage completion; DebuggerPage debugger; EditorPage editor;
	FilesPage files; HelpPage help; HistoryPage history; IPythonConsolePage console;
	ProfilerPage profiler; RunPage run; StatusBarPage statusbar; VariableExplorerPage var_explorer;
	WorkingDirectoryPage work_dir; PluginsPreferencePage plugins;

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

	if(dlg.Run() == IDOK) {
		ApplySettings();
		StoreToFile(settings, ConfigFile("ide_settings.bin"));
	}
}

END_UPP_NAMESPACE
