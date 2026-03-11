# Pane Service Extraction Result

## Purpose
Define exact pane logic extraction from ScriptIDE controls into ScriptCommon services usable by ScriptIDE GUI adapters, ScriptCLI commands, and MCP methods.

## Extraction Rule
For each pane/control:
- Keep view/widget behavior in ScriptIDE (`Ctrl`, `DockableCtrl`, layouts, event hookups).
- Move state/query/transform/business logic to ScriptCommon service.

## Service Map

| Current ScriptIDE UI | ScriptCommon Service | Key Responsibilities | ScriptIDE Adapter |
|---|---|---|---|
| `FilesPane.*` | `FilesService` | list directory tree, filter rules, active path state | `FilesPaneAdapter` |
| `FindInFilesPane.*` | `SearchService` | indexed/non-indexed text search, result paging | `FindInFilesPaneAdapter` |
| `OutlinePane.*` | `OutlineService` | symbol extraction/projection for file/module | `OutlinePaneAdapter` |
| `DebuggerPane.*` | `DebugProjectionService` | normalized stack/breakpoint/locals view models | `DebuggerPaneAdapter` |
| `VariableExplorer.*` | `VariableService` | variable filtering, formatting, sorting | `VariableExplorerAdapter` |
| `PlotsPane.*` | `PlotService` | plot metadata/history/state model | `PlotsPaneAdapter` |
| `HelpPane.*` | `HelpService` | help lookup/query cache | `HelpPaneAdapter` |
| `HistoryPane.*` | `HistoryService` | command/output history model | `HistoryPaneAdapter` |
| `PythonConsole.*` | `ConsoleService` | command execution pipeline + buffer model | `PythonConsoleAdapter` |

## Minimal Service Interface Shape

```cpp
struct ServiceStatus {
    bool ok = true;
    String error;
};

class SearchService {
public:
    struct Query { String text; String root; bool case_sensitive = false; };
    struct Match { String file; int line = 0; int col = 0; String preview; };
    ServiceStatus Search(const Query& q, Vector<Match>& out, int limit = 500);
};
```

Conventions:
- Service APIs use Core/ByteVM/plain structs only.
- No `Ctrl`, `Bar`, `DockableCtrl`, `TopWindow`, or layout types.

## MCP Mapping
Service outputs should map directly to MCP result payloads:
- `FilesService` -> `files.list`
- `SearchService` -> `search.find`
- `OutlineService` -> `outline.get`
- `VariableService` -> `variables.list`
- `HistoryService` -> `history.list`
- `ConsoleService` -> `script.run` / `script.eval` (if added)

## CLI Mapping
Service outputs also back CLI commands:
- `scriptcli run`, `lint`, `plugin test`
- future: `scriptcli files`, `scriptcli search`, `scriptcli outline`

## Migration Order
1. Extract `SearchService`, `FilesService`, `OutlineService` first (low UI coupling).
2. Extract `VariableService`, `HistoryService`, `HelpService` next.
3. Extract `ConsoleService` and `DebugProjectionService` after plugin/runtime split is stable.
4. Extract `PlotService` last (depends on final plot data strategy).

## Acceptance
- [x] Pane-to-service map complete.
- [x] Service boundary defined with headless types.
- [x] MCP/CLI integration mapping documented.
