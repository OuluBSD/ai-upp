#ifndef _ScriptIDE_IDESettings_h_
#define _ScriptIDE_IDESettings_h_

struct AppearanceSettings {
    String interface_theme = "Default";
    String syntax_theme = "Default";
    String monospace_font_face = "Courier New";
    int    monospace_font_size = 10;
    String interface_font_face = "Arial";
    int    interface_font_size = 10;
    bool   use_system_interface_font = true;

    void Serialize(Stream& s) {
        s % interface_theme % syntax_theme % monospace_font_face % monospace_font_size
          % interface_font_face % interface_font_size % use_system_interface_font;
    }
};

struct ApplicationSettings {
    int  hidpi_mode = 0;
    double custom_scale = 1.0;

    bool show_friendly_empty_messages = true;
    bool vertical_tabs_in_panes = false;
    bool custom_margin = false;
    int  pane_margin = 0;
    bool custom_cursor_blink = false;
    int  cursor_blink_ms = 1000;

    String language = "English";
    String rendering_engine = "Default";
    bool single_instance = true;
    bool prompt_on_exit = false;
    bool show_internal_errors = true;
    bool check_updates_on_startup = true;
    bool stable_releases_only = true;
    bool disable_ctrl_wheel_zoom = false;

    void Serialize(Stream& s) {
        s % hidpi_mode % custom_scale % show_friendly_empty_messages % vertical_tabs_in_panes
          % custom_margin % pane_margin % custom_cursor_blink % cursor_blink_ms % language % rendering_engine
          % single_instance % prompt_on_exit % show_internal_errors % check_updates_on_startup
          % stable_releases_only % disable_ctrl_wheel_zoom;
    }
};

struct PythonInterpreterSettings {
    bool use_internal = true;
    String interpreter_path;
    bool use_custom_conda = false;
    String conda_executable;

    bool umr_enabled = true;
    bool umr_verbose = true;
    Vector<String> umr_excluded_modules;

    PythonInterpreterSettings() {}
    PythonInterpreterSettings(const PythonInterpreterSettings& b) { *this = b; }
    PythonInterpreterSettings& operator=(const PythonInterpreterSettings& b) {
        use_internal = b.use_internal;
        interpreter_path = b.interpreter_path;
        use_custom_conda = b.use_custom_conda;
        conda_executable = b.conda_executable;
        umr_enabled = b.umr_enabled;
        umr_verbose = b.umr_verbose;
        umr_excluded_modules <<= b.umr_excluded_modules;
        return *this;
    }

    void Serialize(Stream& s) {
        s % use_internal % interpreter_path % use_custom_conda % conda_executable
          % umr_enabled % umr_verbose % umr_excluded_modules;
    }
};

struct ShortcutItem : Moveable<ShortcutItem> {
    String context;
    String action_id;
    dword key = 0;

    void Serialize(Stream& s) { s % context % action_id % key; }
};

struct ShortcutSettings {
    Vector<ShortcutItem> items;
    
    ShortcutSettings() {}
    ShortcutSettings(const ShortcutSettings& b) { *this = b; }
    ShortcutSettings& operator=(const ShortcutSettings& b) { items <<= b.items; return *this; }

    void Serialize(Stream& s) { s % items; }
};

struct CodeAnalysisSettings {
    bool save_before_analysis = true;
    int history_results = 30;
    String results_path;

    void Serialize(Stream& s) { s % save_before_analysis % history_results % results_path; }
};

struct ExternalLSPServer : Moveable<ExternalLSPServer> {
    String language;
    String address;
    String command;
    void Serialize(Stream& s) { s % language % address % command; }
};

struct Snippet : Moveable<Snippet> {
    String language;
    String trigger;
    String description;
    String body;
    void Serialize(Stream& s) { s % language % trigger % description % body; }
};

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

    String lint_provider = "pyflakes";
    bool underline_errors = true;

    bool enable_go_to_definition = true;
    bool follow_imports = true;
    bool show_calltips = true;
    bool enable_hover_hints = true;
    String preload_modules_csv;

    String formatter = "autopep8";
    bool autoformat_on_save = false;
    int max_line_length = 79;
    bool show_vertical_ruler = true;

    bool lsp_advanced_enabled = false;
    String lsp_module = "pylsp";
    String lsp_address = "127.0.0.1";
    int lsp_port = 2087;
    bool lsp_external_server = false;
    bool lsp_use_stdio = false;

    Vector<ExternalLSPServer> external_servers;
    Vector<Snippet> snippets;

    CompletionLintingSettings() {}
    CompletionLintingSettings(const CompletionLintingSettings& b) { *this = b; }
    CompletionLintingSettings& operator=(const CompletionLintingSettings& b) {
        show_completion_details = b.show_completion_details;
        enable_code_snippets = b.enable_code_snippets;
        show_completions_on_the_fly = b.show_completions_on_the_fly;
        chars_before_completion = b.chars_before_completion;
        completion_detail_delay_ms = b.completion_detail_delay_ms;
        provider_timeout_ms = b.provider_timeout_ms;
        enable_fallback_provider = b.enable_fallback_provider;
        enable_lsp_provider = b.enable_lsp_provider;
        enable_snippet_provider = b.enable_snippet_provider;
        lint_provider = b.lint_provider;
        underline_errors = b.underline_errors;
        enable_go_to_definition = b.enable_go_to_definition;
        follow_imports = b.follow_imports;
        show_calltips = b.show_calltips;
        enable_hover_hints = b.enable_hover_hints;
        preload_modules_csv = b.preload_modules_csv;
        formatter = b.formatter;
        autoformat_on_save = b.autoformat_on_save;
        max_line_length = b.max_line_length;
        show_vertical_ruler = b.show_vertical_ruler;
        lsp_advanced_enabled = b.lsp_advanced_enabled;
        lsp_module = b.lsp_module;
        lsp_address = b.lsp_address;
        lsp_port = b.lsp_port;
        lsp_external_server = b.lsp_external_server;
        lsp_use_stdio = b.lsp_use_stdio;
        external_servers <<= b.external_servers;
        snippets <<= b.snippets;
        return *this;
    }

    void Serialize(Stream& s) {
        s % show_completion_details % enable_code_snippets % show_completions_on_the_fly
          % chars_before_completion % completion_detail_delay_ms % provider_timeout_ms
          % enable_fallback_provider % enable_lsp_provider % enable_snippet_provider
          % lint_provider % underline_errors % enable_go_to_definition % follow_imports
          % show_calltips % enable_hover_hints % preload_modules_csv % formatter
          % autoformat_on_save % max_line_length % show_vertical_ruler % lsp_advanced_enabled
          % lsp_module % lsp_address % lsp_port % lsp_external_server % lsp_use_stdio
          % external_servers % snippets;
    }
};

struct DebuggerSettings {
    bool prevent_close_while_debugging = true;
    bool stop_on_first_line_without_breakpoints = true;
    bool ignore_python_libraries = false;
    bool process_execute_events = true;
    bool use_exclamation_prefix = true;
    String preload_debug_lines;
    bool exclude_internal_frames = true;

    void Serialize(Stream& s) {
        s % prevent_close_while_debugging % stop_on_first_line_without_breakpoints
          % ignore_python_libraries % process_execute_events % use_exclamation_prefix
          % preload_debug_lines % exclude_internal_frames;
    }
};

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
    bool show_spaces = false;

    bool wrap_lines = true;
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

    void Serialize(Stream& s) {
        s % show_tab_bar % show_full_path_above_editor % show_class_function_selector
          % allow_scroll_past_eof % show_indent_guides % show_code_folding % show_line_numbers
          % show_debugger_breakpoints % show_code_annotations % show_spaces % highlight_current_line
          % highlight_current_cell % highlight_selected_occurrences % occurrence_highlight_delay_ms
          % auto_insert_closing_brackets % auto_insert_closing_quotes % auto_insert_colons
          % auto_unindent_keywords % strip_trailing_spaces_on_save % strip_trailing_spaces_changed_lines
          % add_missing_newline_eof % strip_blank_lines_eof % tab_width % indent_with_spaces
          % intelligent_backspace % tab_always_indents % fix_mixed_eol % convert_eol_on_save
          % eol_mode % new_file_template % autosave_backup_unsaved % autosave_interval_sec
          % docstring_style % enable_multicursor % multicursor_paste_mode % mouse_modifier_multicursor;
    }
};

struct FileAssociation : Moveable<FileAssociation> {
    String extension;
    Vector<String> applications;
    int default_index = -1;

    FileAssociation() {}
    FileAssociation(const FileAssociation& b) { *this = b; }
    FileAssociation& operator=(const FileAssociation& b) {
        extension = b.extension;
        applications <<= b.applications;
        default_index = b.default_index;
        return *this;
    }

    void Serialize(Stream& s) { s % extension % applications % default_index; }
};

struct FilesSettings {
    bool show_hidden_files = false;
    bool single_click_open = false;
    String filter_patterns_csv;
    Vector<FileAssociation> associations;

    FilesSettings() {}
    FilesSettings(const FilesSettings& b) { *this = b; }
    FilesSettings& operator=(const FilesSettings& b) {
        show_hidden_files = b.show_hidden_files;
        single_click_open = b.single_click_open;
        filter_patterns_csv = b.filter_patterns_csv;
        associations <<= b.associations;
        return *this;
    }

    void Serialize(Stream& s) {
        s % show_hidden_files % single_click_open % filter_patterns_csv % associations;
    }
};

struct HelpSettings {
    bool auto_connect_editor = false;
    bool auto_connect_console = false;
    bool render_math = true;
    bool wrap_lines = true;

    void Serialize(Stream& s) {
        s % auto_connect_editor % auto_connect_console % render_math % wrap_lines;
    }
};

struct HistorySettings {
    bool wrap_lines = true;
    bool show_line_numbers = false;
    bool scroll_to_last_entry = true;

    void Serialize(Stream& s) {
        s % wrap_lines % show_line_numbers % scroll_to_last_entry;
    }
};

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
    double inline_width_in = 6.0;
    double inline_height_in = 4.0;
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

    void Serialize(Stream& s) {
        s % show_welcome_message % show_calltips % show_elapsed_time % confirm_before_closing
          % confirm_before_restarting % confirm_before_removing_variables % completion_display
          % use_jedi_completion % use_greedy_completion % output_buffer_lines % render_sympy_math
          % matplotlib_support % auto_import_numpy_matplotlib % graphics_backend % inline_format
          % inline_resolution_dpi % inline_width_in % inline_height_in % inline_font_points
          % inline_bottom_edge % inline_tight_layout % startup_code % execute_startup_file
          % startup_file % autocall_mode % use_autoreload % input_prompt % output_prompt
          % hide_subprocess_windows;
    }
};

struct ProfilerSettings {
    bool open_on_finish = true;
    int max_hot_items = 15;

    void Serialize(Stream& s) { s % open_on_finish % max_hot_items; }
};

struct RunPreset : Moveable<RunPreset> {
    String name;
    String file_extension;
    String context;
    void Serialize(Stream& s) { s % name % file_extension % context; }
};

struct RunSettings {
    Vector<RunPreset> presets;
    String selected_runner = "Internal";
    bool save_all_before_run = true;
    bool copy_full_cell_to_console = false;

    RunSettings() {}
    RunSettings(const RunSettings& b) { *this = b; }
    RunSettings& operator=(const RunSettings& b) {
        presets <<= b.presets;
        selected_runner = b.selected_runner;
        save_all_before_run = b.save_all_before_run;
        copy_full_cell_to_console = b.copy_full_cell_to_console;
        return *this;
    }

    void Serialize(Stream& s) { s % presets % selected_runner % save_all_before_run % copy_full_cell_to_console; }
};

struct StatusBarSettings {
    bool show_memory = true;
    int memory_poll_ms = 2000;
    bool show_cpu = false;
    int cpu_poll_ms = 2000;
    bool show_clock = false;

    void Serialize(Stream& s) {
        s % show_memory % memory_poll_ms % show_cpu % cpu_poll_ms % show_clock;
    }
};

struct VariableExplorerSettings {
    bool exclude_private = true;
    bool exclude_capitalized = false;
    bool exclude_uppercase = false;
    bool exclude_unsupported = false;
    bool exclude_callables = true;
    bool show_array_minmax = false;

    void Serialize(Stream& s) {
        s % exclude_private % exclude_capitalized % exclude_uppercase % exclude_unsupported
          % exclude_callables % show_array_minmax;
    }
};

struct WorkingDirectorySettings {
    int startup_mode = 0;
    String startup_directory;

    int new_console_mode = 1;
    String new_console_directory;

    void Serialize(Stream& s) {
        s % startup_mode % startup_directory % new_console_mode % new_console_directory;
    }
};

struct PluginState : Moveable<PluginState> {
    String id;
    bool enabled = true;
    void Serialize(Stream& s) { s % id % enabled; }
};

struct PluginSettings {
    Vector<PluginState> states;

    PluginSettings() {}
    PluginSettings(const PluginSettings& b) { *this = b; }
    PluginSettings& operator=(const PluginSettings& b) { states <<= b.states; return *this; }

    void Serialize(Stream& s) { s % states; }
};

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

    void Serialize(Stream& s) {
        s % appearance % application % python % shortcuts % code_analysis % completion
          % debugger % editor % files % help % history % console % profiler % run
          % statusbar % variable_explorer % working_directory % plugins;
    }
};

#endif
