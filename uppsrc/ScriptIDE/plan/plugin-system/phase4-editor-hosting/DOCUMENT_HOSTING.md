# Document Hosting Abstraction

## Overview
ScriptIDE's central area must be decoupled from `Upp::CodeEditor` to allow plugins to provide custom views (e.g., game boards, visual designers).

## The `IDocumentHost` Interface
This interface defines the minimum requirements for any control to be hosted in an IDE tab.

```cpp
class IDocumentHost {
public:
    virtual ~IDocumentHost() {}
    virtual Ctrl&  GetCtrl() = 0;              // Returns the U++ widget
    virtual bool   Load(const String& path) = 0;
    virtual bool   Save() = 0;
    virtual bool   SaveAs(const String& path) = 0;
    virtual String GetPath() const = 0;
    virtual bool   IsModified() const = 0;
    virtual void   SetFocus() = 0;
    
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

## Routing Logic in `PythonIDE`
1. User requests to open a file (e.g., `table.xlay`).
2. `PythonIDE` queries `PluginManager::FindFileTypeHandler(".xlay")`.
3. If a handler is found, it calls `handler->CreateDocumentHost()`.
4. If no handler exists, it defaults to the standard `SourceDocumentHost` (wrapping `PythonEditor`).
5. The returned `IDocumentHost` is stored in the `FileInfo` struct and its `Ctrl` is added to the `editor_area`.

## Capability Hiding
UI elements (like the "Undo" toolbar button) will query the active `IDocumentHost` to see if the action is supported. Custom views may leave these as no-ops if they don't support text-like editing.
