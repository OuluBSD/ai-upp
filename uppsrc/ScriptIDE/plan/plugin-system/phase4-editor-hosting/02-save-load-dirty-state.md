# Task: Save/Load/Dirty State Behavior for Custom Tabs

## Goal
Define how ScriptIDE handles file persistence and modification tracking for custom, non-text document tabs like `.xlay` layout editors or game state viewers.

## Background / Rationale
When a plugin provides a custom `Ctrl` for a document (e.g., a visual editor), standard `CodeEditor` methods like `IsModified()` or `SaveFile()` do not apply. The IDE needs a unified way to know if a custom document has unsaved changes, prompt the user upon closing, and trigger the plugin to save its internal representation back to disk.

## Scope
- Defining how custom editors signal dirty state changes to the IDE.
- Defining the IDE's interaction with the custom editor during the "Save" and "Save All" actions.
- Defining the reload behavior when the underlying file is modified externally.

## Non-goals
- Implementing specific save logic for `.xlay` (that belongs in the FormEditor adaptation task).

## Dependencies
- `01-non-codeeditor-document-host.md`

## Concrete Investigation Steps
1. Review the proposed `IDocumentHost` interface to ensure it includes virtual methods like `bool IsDirty() const` and `bool Save()`.
2. Determine if an `Event<> WhenDirty` callback is necessary for the plugin to proactively notify the IDE to update the tab title (e.g., adding an asterisk `*`).

## Affected Subsystems
- `uppsrc/ScriptIDE/PythonIDE.cpp` (File management actions)
- `uppsrc/ScriptIDE/PluginInterfaces.h`

## Implementation Direction
Outline the exact sequence of events for a "Save" action triggered on a plugin-hosted tab, detailing the methods called on the `IDocumentHost` interface.

## Risks
- Data loss if the IDE fails to recognize a custom tab is dirty and closes without prompting.

## Acceptance Criteria
- [ ] Documented dirty-state tracking mechanism for custom tabs.
- [ ] Documented save/load sequence for plugin-owned files.
- [ ] Defined tab title update behavior for modified custom documents.
