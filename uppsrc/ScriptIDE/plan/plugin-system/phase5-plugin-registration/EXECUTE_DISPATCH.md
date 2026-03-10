# Custom Execute Dispatch Mechanism

## Overview
When a user clicks the "Run" button (F5), ScriptIDE normally executes the active Python script. However, some file types (like `.gamestate`) require custom logic to "launch" a scenario or game.

## The `ICustomExecuteProvider` Interface
```cpp
class ICustomExecuteProvider {
public:
    virtual ~ICustomExecuteProvider() {}
    virtual bool CanExecute(const String& path) = 0; // Checks if it handles this file
    virtual void Execute(const String& path) = 0;    // Performs the custom action
};
```

## Dispatch Logic in `PythonIDE::OnRun()`
The core "Run" method is updated to check plugins before defaulting to ByteVM:
1. `PythonIDE` retrieves the path of the active document.
2. It calls `PluginManager::FindCustomExecuteProvider(path)`.
3. If a provider returns `true` for `CanExecute`:
   - `provider->Execute(path)` is called.
   - Standard execution is skipped.
4. If no provider matches:
   - The IDE proceeds with standard Python script compilation and execution.

## Example: `.gamestate` Execution
The Card Game Plugin will register a provider that:
1. Parses the YAML `.gamestate`.
2. Loads the `.xlay` into a new `GameView` tab.
3. Injects bindings and starts the `entry_script` in the `ByteVM`.

## Failure Handling
If `Execute` fails (e.g., malformed YAML), the plugin is responsible for logging an error to the IDE console (`console_pane.WriteError`). The IDE state remains stable.
