#ifndef _ScriptIDE_PluginInterfaces_h_
#define _ScriptIDE_PluginInterfaces_h_

class PyVM;
class PythonIDE;
class IPlugin;

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

class IPluginContext {
public:
	virtual ~IPluginContext() {}
	virtual PythonIDE& GetIDE() = 0;
	virtual PyVM*      GetVM() = 0;
	virtual void       RegisterDockPane(const String& id, const String& title, Ctrl& ctrl) = 0;
	virtual void       UnregisterDockPane(const String& id) = 0;
};

class IPlugin {
public:
	virtual ~IPlugin() {}
	virtual String GetID() const = 0;
	virtual String GetName() const = 0;
	virtual String GetDescription() const = 0;
	virtual void   Init(IPluginContext& context) = 0;
	virtual void   Shutdown() = 0;
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

class IPythonBindingProvider {
public:
	virtual ~IPythonBindingProvider() {}
	virtual void SyncBindings(PyVM& vm) = 0;
};

class ICustomExecuteProvider {
public:
	virtual ~ICustomExecuteProvider() {}
	virtual bool CanExecute(const String& path) = 0;
	virtual void Execute(const String& path) = 0;
};

class IPluginRegistry {
public:
	virtual ~IPluginRegistry() {}
	virtual void RegisterFileTypeHandler(IFileTypeHandler& handler) = 0;
	virtual void RegisterDockPaneProvider(IDockPaneProvider& provider) = 0;
	virtual void RegisterPythonBindingProvider(IPythonBindingProvider& provider) = 0;
	virtual void RegisterCustomExecuteProvider(ICustomExecuteProvider& provider) = 0;
};

typedef IPlugin* (*PluginFactory)();

Vector<PluginFactory>& GetInternalPluginFactories();

#define REGISTER_PLUGIN(T) \
static IPlugin* T##_Factory() { return new T(); } \
static bool T##_Registered = []() { \
    GetInternalPluginFactories().Add(&T##_Factory); \
    return true; \
}();

#endif
