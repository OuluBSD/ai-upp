# ScriptIDE to ScriptCommon Migration Map

## Purpose
Make legacy ScriptIDE plan tracks compatible with the active `script-cli` split initiative.

## Active Rule
If a legacy plan references non-GUI core code in `uppsrc/ScriptIDE`, map it to `uppsrc/ScriptCommon` unless the item is explicitly UI/control code.

## Initial File Moves
- `uppsrc/ScriptIDE/RunManager.h/.cpp` -> `uppsrc/ScriptCommon/RunManager.h/.cpp`
- `uppsrc/ScriptIDE/Linter.h/.cpp` -> `uppsrc/ScriptCommon/Linter.h/.cpp`
- `uppsrc/ScriptIDE/PathManager.h/.cpp` -> `uppsrc/ScriptCommon/PathManager.h/.cpp`
- `uppsrc/ScriptIDE/IDESettings.h` -> `uppsrc/ScriptCommon/IDESettings.h`

## Planned Splits
- `PluginInterfaces.h`: split into core contracts in ScriptCommon + GUI adapter contracts in ScriptIDE.
- `PluginManager.h/.cpp`: split registry/runtime core to ScriptCommon + UI integration manager in ScriptIDE.
- Pane files (`*Pane*`, `VariableExplorer`, `FilesPane`, etc.): extract logic/services to ScriptCommon and keep control/view wrappers in ScriptIDE.

## Legacy Plan Interpretation
- `07-spyder-exact/phase3-menubar-functions/02-implement-run-debug-logic.md`:
  `RunManager` ownership is ScriptCommon; ScriptIDE calls via adapter.
- `07-spyder-exact/phase2-preferences-system/*` and `spyder/PREFERENCES_*`:
  `IDESettings` schema ownership is ScriptCommon; page widgets remain in ScriptIDE.
- `08-plugin-system/*` and `plugin-system/*`:
  plugin core contracts/registry move to ScriptCommon; docking/menu/document-host UI remains ScriptIDE.

## MCP Alignment
- Reuse architecture pattern from `uppsrc/MCP` + `uppsrc/ide/MCP`:
  headless core handlers in ScriptCommon, process/transport host in ScriptCLI.

## Source of Truth
- Active sequence and status: `uppsrc/ScriptIDE/plan/cookie.txt`
- Active split track: `uppsrc/ScriptIDE/plan/script-cli/`
