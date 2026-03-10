# Task: Define Non-CodeEditor Document Host

## Goal
Design the abstraction layer that allows ScriptIDE's central tab area to host arbitrary U++ `Ctrl`s, breaking the hardcoded dependency on `Upp::CodeEditor`.

## Background / Rationale
To support `.gamestate` rendering and `.form` visual editing, the IDE's main editor area must be capable of displaying custom views provided by plugins, rather than assuming every file is a text document needing a `CodeEditor`.

## Scope
- Designing an `IDocumentHost` (or similar) interface that encapsulates the active editor view.
- Defining how the IDE instantiates these views based on file extension (File-type routing).
- Defining the behavior of standard IDE actions (Save, Undo, Find) when a non-text editor is active.

## Non-goals
- Implementing the custom `.gamestate` view (that belongs in the reference plugin).

## Dependencies
- `01-plugin-lifecycle-manifest.md`

## Concrete Investigation Steps
1. Review `uppsrc/ScriptIDE/PythonIDE.cpp` to see how `CodeEditor` is currently embedded and accessed.
2. Draft an abstraction interface that handles basic lifecycle (Load, Save, SetFocus) and capabilities (IsModified, Undo/Redo support).
3. Determine how the `FileTabs` component will store a pointer to the abstract host instead of a direct `CodeEditor` reference.

## Affected Subsystems
- `uppsrc/ScriptIDE/PythonIDE.h/cpp`
- `uppsrc/ScriptIDE/PluginInterfaces.h`

## Implementation Direction
Create an architecture plan detailing the `IDocumentHost` interface and the changes required in `PythonIDE` to support a list of generic document hosts instead of a list of `CodeEditor` pointers.

## Risks
- Losing standard text-editor features (like search or formatting) if the abstraction is too thin or misapplied to Python files.

## Acceptance Criteria
- [ ] Documented `IDocumentHost` (or equivalent) abstraction interface.
- [ ] Defined plan for routing file extensions to specific plugin factories.
- [ ] Documented strategy for handling unsupported editor actions on custom views.
