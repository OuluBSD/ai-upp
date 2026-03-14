#ifndef _ScriptIDE_PluginInterfacesGUI_h_
#define _ScriptIDE_PluginInterfacesGUI_h_

// No includes here - they are in ScriptIDE.h or Main package header

class PythonIDE;
struct IDEContext;

class IDocumentHost {
public:
	enum RunMode {
		RUNMODE_NONE,
		RUNMODE_RUN,
		RUNMODE_DEBUG,
		RUNMODE_PROFILE,
	};

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

	// Run/Debug lifecycle
	virtual bool   CanRun() const { return false; }
	virtual bool   IsRunning() const { return false; }
	virtual bool   CanPause() const { return false; }
	virtual bool   IsPaused() const { return false; }
	virtual RunMode GetRunMode() const { return RUNMODE_NONE; }
	virtual void   Run() {}
	virtual void   Debug() { Run(); }
	virtual void   Profile() { Run(); }
	virtual void   Pause() {}
	virtual void   Stop() {}
	virtual void   PopulateDebugState(PythonIDE& ide) {}
	virtual String DumpPythonStack() const { return String(); }

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

struct ScriptRunState : Moveable<ScriptRunState> {
	IDocumentHost* host = nullptr;
	String path;
	IDocumentHost::RunMode mode = IDocumentHost::RUNMODE_NONE;
	bool running = false;
	bool paused = false;
	bool can_run = false;
};

class IPluginPreferencesPage {
public:
	virtual ~IPluginPreferencesPage() {}
	virtual Ctrl& GetCtrl() = 0;
	virtual void  LoadConfig() = 0;
	virtual void  SaveConfig() = 0;
	virtual void  ApplyConfig(IDEContext& ctx) = 0;
	virtual void  SetDefaults() = 0;
	virtual bool  IsModified() const = 0;
};

class IPluginPreferencesProvider {
public:
	virtual ~IPluginPreferencesProvider() {}
	virtual int    GetPreferencesPageCount() const = 0;
	virtual String GetPreferencesPageCategory(int index) const { return "Plugins"; }
	virtual String GetPreferencesPageID(int index) const = 0;
	virtual String GetPreferencesPageTitle(int index) const = 0;
	virtual Image  GetPreferencesPageIcon(int index) const = 0;
	virtual IPluginPreferencesPage& GetPreferencesPage(int index) = 0;
};

class IRunStateListener {
public:
	virtual ~IRunStateListener() {}
	virtual void OnRunStateChanged(PythonIDE& ide, const ScriptRunState& state) = 0;
};

class IVideoRenderSource {
public:
	virtual ~IVideoRenderSource() {}
	virtual bool CanRecordVideo() const = 0;
	virtual Size GetRecordFrameSize() const = 0;
	virtual Image CaptureRecordFrame(Size target_size = Size()) const = 0;
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
	enum HostRole {
		HOSTROLE_VIEWER,
		HOSTROLE_EDITOR,
	};

	virtual ~IFileTypeHandler() {}
	virtual String         GetExtension() const = 0;
	virtual String         GetFileDescription() const = 0;
	virtual bool           CanHandle(const String& path) const { return ToLower(GetFileExt(path)) == ToLower(GetExtension()); }
	virtual bool           SupportsHostRole(HostRole role) const { return role == HOSTROLE_EDITOR; }
	virtual IDocumentHost* CreateHost(HostRole role) {
		return role == HOSTROLE_EDITOR ? CreateEditorHost() : CreateViewerHost();
	}
	virtual IDocumentHost* CreateEditorHost() { return CreateDocumentHost(); }
	virtual IDocumentHost* CreateViewerHost() { return nullptr; }
	virtual IDocumentHost* CreateDocumentHost() { return nullptr; }
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
	virtual void RegisterPreferencesProvider(IPluginPreferencesProvider& provider) = 0;
	virtual void RegisterRunStateListener(IRunStateListener& listener) = 0;
};

#define REGISTER_SCRIPTIDE_PLUGIN(T) REGISTER_PLUGIN(T)

#endif
