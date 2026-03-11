# Inventory: ScriptIDE Components for Non-GUI Extraction

## Goal
Complete file-level classification of `uppsrc/ScriptIDE` package files for migration into `ScriptCommon` and `ScriptCLI`.

## Method
- Source of truth: `uppsrc/ScriptIDE/ScriptIDE.upp` `file` section.
- Classification targets:
  - `ScriptCommon`: non-GUI logic/data/services.
  - `ScriptIDE`: GUI controls/views/dialogs/main app shell.
  - `Split`: mixed ownership; core logic extracted to ScriptCommon, UI adapter stays in ScriptIDE.

## File Classification (Complete)

| File | Category | Destination | Reason |
|---|---|---|---|
| `AGENTS.md` | docs | ScriptIDE | package-local instructions |
| `ScriptIDE.h` | umbrella/header wiring | ScriptIDE | includes GUI stack (`CtrlLib`, `CodeEditor`, `Docking`) |
| `PythonIDE.h` | main window | ScriptIDE | GUI composition and docking |
| `PythonIDE.cpp` | main window impl | ScriptIDE | GUI runtime wiring |
| `IDESettings.h` | settings schema/data | ScriptCommon | pure data + serialization |
| `Settings.h` | dialog header | ScriptIDE | `TopWindow` dialog type |
| `Settings.cpp` | dialog impl | ScriptIDE | settings dialog logic |
| `VariableExplorer.h` | pane control | Split | UI control + inspect logic |
| `VariableExplorer.cpp` | pane control impl | Split | extract variable formatting/filter logic |
| `PlotsPane.h` | pane control | Split | UI control + plot state |
| `PlotsPane.cpp` | pane control impl | Split | extract plot metadata/history logic |
| `DebuggerPane.h` | pane control | Split | UI control + debug state projection |
| `DebuggerPane.cpp` | pane control impl | Split | extract state projection/model logic |
| `ProfilerPane.h` | pane control | Split | UI + profiling state |
| `ProfilerPane.cpp` | pane control impl | Split | extract profiling model |
| `FindInFilesPane.h` | pane control | Split | UI + search logic |
| `FindInFilesPane.cpp` | pane control impl | Split | extract search service |
| `OutlinePane.h` | pane control | Split | UI + symbol listing logic |
| `OutlinePane.cpp` | pane control impl | Split | extract outline/symbol service |
| `HelpPane.h` | pane control | Split | UI + help query logic |
| `HelpPane.cpp` | pane control impl | Split | extract help service |
| `PluginInterfaces.h` | plugin contracts | Split | contains both headless and Ctrl/PythonIDE-coupled contracts |
| `PluginManager.h` | plugin manager API | Split | mixed registry/runtime/UI ownership |
| `PluginManager.cpp` | plugin manager impl | Split | includes pane docking + registry operations |
| `HistoryPane.h` | pane control | Split | UI + history state |
| `HistoryPane.cpp` | pane control impl | Split | extract history service |
| `GameStatePlugin.h` | plugin | Split | plugin core + RichTextView UI |
| `GameStatePlugin.cpp` | plugin impl | Split | extract core simulation/binding logic |
| `CardGamePlugin.h` | plugin | Split | game logic and Ctrl UI mixed |
| `CardGamePlugin.cpp` | plugin impl | Split | large mixed UI/logic surface |
| `RunManager.h` | runtime service | ScriptCommon | non-GUI execution service |
| `RunManager.cpp` | runtime service impl | ScriptCommon | uses ByteVM/tokenizer/compiler only |
| `Linter.h` | analysis service | ScriptCommon | non-GUI lint contract |
| `Linter.cpp` | analysis service impl | ScriptCommon | non-GUI compiler-check path |
| `PathManager.h` | environment service | ScriptCommon | non-GUI path model |
| `PathManager.cpp` | environment service impl | ScriptCommon | non-GUI VM path sync |
| `PathManagerDlg.h` | dialog header | ScriptIDE | `TopWindow` UI |
| `PathManagerDlg.cpp` | dialog impl | ScriptIDE | UI behavior |
| `PreferencesPage.h` | GUI page contract | ScriptIDE | page widget abstraction |
| `PreferencesWindow.h` | GUI window | ScriptIDE | preferences top window |
| `PreferencesWindow.cpp` | GUI window impl | ScriptIDE | preferences UI orchestration |
| `PreferencesPages.cpp` | GUI pages | ScriptIDE | all Ctrl page layouts |
| `PythonConsole.h` | console pane | Split | UI editor + console behavior |
| `PythonConsole.cpp` | console pane impl | Split | extract console service/state |
| `FilesPane.h` | files pane | Split | UI tree + file model logic |
| `FilesPane.cpp` | files pane impl | Split | extract filesystem model/navigation logic |
| `CustomFileTabs.h` | tabs widget | ScriptIDE | GUI tab control |
| `CustomFileTabs.cpp` | tabs widget impl | ScriptIDE | GUI behavior |
| `Main.cpp` | app entrypoint | ScriptIDE | `GUI_APP_MAIN` |
| `ScriptIDE.lay` | layout resource | ScriptIDE | GUI resource |
| `ScriptIDE.iml` | image resource | ScriptIDE | GUI resource |

## Initial Extraction Set (Phase 3)
Move first as low-risk, no-GUI files:
1. `IDESettings.h`
2. `RunManager.h/.cpp`
3. `Linter.h/.cpp`
4. `PathManager.h/.cpp`

## Mixed Components Requiring Split (Phase 2+)
1. `PluginInterfaces.h` -> core contracts (`ScriptCommon`) + GUI adapters (`ScriptIDE`).
2. `PluginManager.*` -> plugin registry/runtime core (`ScriptCommon`) + UI integration (`ScriptIDE`).
3. Pane family (`*Pane*`, `VariableExplorer`, `FilesPane`, `PythonConsole`) -> service/model extraction to `ScriptCommon`; Ctrl wrappers remain in ScriptIDE.
4. Feature plugins (`CardGamePlugin`, `GameStatePlugin`) -> headless behavior/bindings in ScriptCommon; rendering/document host UI in ScriptIDE.

## Acceptance Check
- [x] Every `ScriptIDE.upp` file entry classified.
- [x] Initial extraction set listed.
- [x] Mixed split candidates explicitly identified.
