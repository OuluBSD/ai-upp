# Task: Custom Execute Dispatch Plan

## Goal
Define how plugins can intercept and override the "Run" / "Execute" command in ScriptIDE for specific file types like `.gamestate`.

## Background / Rationale
When a user clicks "Run" on a Python script, it executes in the ByteVM. However, when they click "Run" on a `.gamestate` file, the IDE shouldn't try to compile the JSON. Instead, the associated plugin must intercept the command, parse the JSON, set up the layout, and then invoke the Python VM on the specified entry script.

## Scope
- Defining the `ICustomExecuteProvider` interface.
- Defining how `PythonIDE::OnRun()` queries the `PluginManager` before defaulting to standard Python execution.

## Non-goals
- Implementing the execution logic for `.form` files (they are likely not executable directly).

## Dependencies
- `01-plugin-lifecycle-manifest.md`
- `01-gamestate-schema-design.md`

## Concrete Investigation Steps
1. Review `PythonIDE::OnRun()` in `uppsrc/ScriptIDE/PythonIDE.cpp`.
2. Design a registry method in `PluginManager` where plugins can register their intent to handle execution for certain extensions or paths.
3. Define the fallback behavior if a custom execute provider fails.

## Affected Subsystems
- `uppsrc/ScriptIDE/PluginInterfaces.h`
- `uppsrc/ScriptIDE/PythonIDE.cpp` (Run command dispatch)

## Implementation Direction
Create an architectural plan for the execute dispatch. Sketch the `ICustomExecuteProvider::Execute(const String& path)` method.

## Risks
- A buggy plugin could silently swallow Run commands for all files if the routing logic is too broad.

## Acceptance Criteria
- [ ] Documented `ICustomExecuteProvider` interface.
- [ ] Defined dispatch logic in `PythonIDE::OnRun()`.
- [ ] Defined behavior for execution failures.
