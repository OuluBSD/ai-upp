#ifndef _ScriptIDE_GameStatePlugin_h_
#define _ScriptIDE_GameStatePlugin_h_

class GameStatePlugin : public IPlugin, 
                        public IFileTypeHandler, 
                        public IDockPaneProvider, 
                        public IPythonBindingProvider, 
                        public ICustomExecuteProvider {
public:
	GameStatePlugin();
	virtual ~GameStatePlugin();

	// IPlugin
	virtual String GetID() const override { return "GameStatePlugin"; }
	virtual String GetName() const override { return "Game State Viewer"; }
	virtual String GetDescription() const override { return "Provides .gamestate visualization and simulation."; }
	virtual void   Init(IPluginContext& context) override;
	virtual void   Shutdown() override;

	// IFileTypeHandler
	virtual String         GetExtension() const override { return ".gamestate"; }
	virtual String         GetFileDescription() const override { return "Game State JSON"; }
	virtual IDocumentHost* CreateDocumentHost() override;

	// IDockPaneProvider
	virtual int    GetPaneCount() const override { return 1; }
	virtual String GetPaneID(int index) const override { return "GameStateStats"; }
	virtual String GetPaneTitle(int index) const override { return "Game Stats"; }
	virtual Ctrl&  GetPaneCtrl(int index) override { return stats_view; }

	// IPythonBindingProvider
	virtual void SyncBindings(PyVM& vm) override;

	// ICustomExecuteProvider
	virtual bool CanExecute(const String& path) override;
	virtual void Execute(const String& path) override;

private:
	IPluginContext* context = nullptr;
	RichTextView    stats_view;
	
	void UpdateStats(const String& json);
	static PyValue GetScore(const Vector<PyValue>& args, void* user_data);
};

class GameStateDocumentHost : public IDocumentHost, public Ctrl {
public:
	GameStateDocumentHost();
	virtual ~GameStateDocumentHost();

	virtual Ctrl&  GetCtrl() override { return *this; }
	virtual bool   Load(const String& path) override;
	virtual bool   Save() override;
	virtual bool   SaveAs(const String& path) override { return false; } // Read-only for now
	virtual String GetPath() const override { return path; }
	virtual bool   IsModified() const override { return false; }
	virtual void   SetFocus() override { Ctrl::SetFocus(); }
	virtual void   Undo() override {}
	virtual void   Redo() override {}
	virtual void   Cut() override {}
	virtual void   Copy() override {}
	virtual void   Paste() override {}
	virtual void   SelectAll() override {}
	virtual void   Find() override {}
	virtual void   Replace() override {}

private:
	String path;
	RichTextView view;
	
	virtual void Paint(Draw& w) override { w.DrawRect(GetSize(), SWhite); }
};

#endif
