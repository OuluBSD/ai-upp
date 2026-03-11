#ifndef _ScriptIDE_PluginInterfacesGUI_h_
#define _ScriptIDE_PluginInterfacesGUI_h_

// No includes here - they are in ScriptIDE.h or Main package header

class PythonIDE;

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
	virtual void   ActivateUI() {}
	virtual void   DeactivateUI() {}
	
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

class IPluginContextGUI : public IPluginContext {
public:
	virtual ~IPluginContextGUI() {}
	virtual PythonIDE& GetIDE() = 0;
	virtual void       RegisterDockPane(const String& id, const String& title, Ctrl& ctrl) = 0;
	virtual void       UnregisterDockPane(const String& id) = 0;
};

class IFileTypeHandler {
public:
	virtual ~IFileTypeHandler() {}
	virtual String         GetExtension() const = 0;
	virtual String         GetFileDescription() const = 0;
	virtual IDocumentHost* CreateDocumentHost() = 0;
};

class IDockPaneProvider {
public:
	virtual ~IDockPaneProvider() {}
	virtual int    GetPaneCount() const = 0;
	virtual String GetPaneID(int index) const = 0;
	virtual String GetPaneTitle(int index) const = 0;
	virtual Ctrl&  GetPaneCtrl(int index) = 0;
};

class IPluginRegistryGUI : public IPluginRegistry {
public:
	virtual ~IPluginRegistryGUI() {}
	virtual void RegisterFileTypeHandler(IFileTypeHandler& handler) = 0;
	virtual void RegisterDockPaneProvider(IDockPaneProvider& provider) = 0;
};

#endif
