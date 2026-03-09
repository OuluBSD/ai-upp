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

PREF_PAGE(Appearance)
PREF_PAGE(Application)
PREF_PAGE(PythonInterpreter)
PREF_PAGE(KeyboardShortcuts)
PREF_PAGE(CodeAnalysis)
PREF_PAGE(CompletionLinting)
PREF_PAGE(Debugger)
PREF_PAGE(Editor)
PREF_PAGE(Files)
PREF_PAGE(Help)
PREF_PAGE(History)
PREF_PAGE(IPythonConsole)
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
