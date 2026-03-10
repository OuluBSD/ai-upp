# Document Hosting Abstraction

## Overview
ScriptIDE's central area must be decoupled from `Upp::CodeEditor` to allow plugins to provide custom views. Hosting also includes managing context-sensitive UI elements like menus and auxiliary panes.

## The `IDocumentHost` Interface
```cpp
class IDocumentHost {
public:
    virtual ~IDocumentHost() {}
    virtual Ctrl&  GetCtrl() = 0;
    virtual bool   Load(const String& path) = 0;
    virtual bool   Save() = 0;
    virtual bool   SaveAs(const String& path) = 0;
    virtual String GetPath() const = 0;
    virtual bool   IsModified() const = 0;
    virtual void   SetFocus() = 0;
    
    // UI Lifecycle
    virtual void   ActivateUI() {}   // Called when tab becomes active
    virtual void   DeactivateUI() {} // Called when tab becomes inactive
    
    // Menu/Toolbar hooks
    virtual void   MainMenu(Bar& bar) {}
    virtual void   Toolbar(Bar& bar) {}

    // Command Routing
    virtual void   Undo() {}
    virtual void   Redo() {}
    virtual void   Cut() {}
    virtual void   Copy() {}
    virtual void   Paste() {}
    virtual void   SelectAll() {}
    virtual void   Find() {}
    virtual void   Replace() {}
};
```

## Contextual Integration
When a document tab is selected:
1. `PythonIDE` calls `DeactivateUI()` on the previously active host.
2. It calls `ActivateUI()` on the new host.
3. The IDE's main menu and toolbar are rebuilt, calling `active_host->MainMenu()` and `active_host->Toolbar()`.
4. The host can use `ActivateUI()` to show specific dockable panes or claim placeholder panes provided by the IDE.

## Placeholder Dockables
The IDE maintains several generic `DockableCtrl` placeholders (e.g., `ContextPaneLeft`, `ContextPaneRight`).
- A plugin can put its specific sub-views (like a Property Grid) into these placeholders.
- When the plugin's tab is deactivated, it must clear the placeholders.
