#ifndef _ScriptIDE_CardGamePlugin_h_
#define _ScriptIDE_CardGamePlugin_h_

#include <Core/Core.h>
#include <CtrlLib/CtrlLib.h>

NAMESPACE_UPP

class CardGamePlugin : public IPlugin, 
                       public IFileTypeHandler, 
                       public ICustomExecuteProvider,
                       public IPythonBindingProvider {
public:
	CardGamePlugin();
	virtual ~CardGamePlugin();

	// IPlugin
	virtual String GetID() const override { return "CardGamePlugin"; }
	virtual String GetName() const override { return "Card Game Engine"; }
	virtual String GetDescription() const override { return "Provides .gamestate rendering and .form layout support."; }
	virtual void   Init(IPluginContext& context) override;
	virtual void   Shutdown() override;

	// IFileTypeHandler
	virtual String         GetExtension() const override { return ".gamestate"; }
	virtual String         GetFileDescription() const override { return "Game State JSON/YAML"; }
	virtual IDocumentHost* CreateDocumentHost() override;

	// ICustomExecuteProvider
	virtual bool CanExecute(const String& path) override;
	virtual void Execute(const String& path) override;

	// IPythonBindingProvider
	virtual void SyncBindings(PyVM& vm) override;

private:
	IPluginContext* context = nullptr;
};

class CardGameDocumentHost : public IDocumentHost, public Ctrl {
public:
	CardGameDocumentHost();
	virtual ~CardGameDocumentHost();

	virtual Ctrl&  GetCtrl() override { return *this; }
	virtual bool   Load(const String& path) override;
	virtual bool   Save() override { return true; }
	virtual bool   SaveAs(const String& path) override { return true; }
	virtual String GetPath() const override { return path; }
	virtual bool   IsModified() const override { return false; }
	virtual void   SetFocus() override { Ctrl::SetFocus(); }
	
	virtual void   ActivateUI() override;
	virtual void   DeactivateUI() override;
	virtual void   MainMenu(Bar& bar) override;
	virtual void   Toolbar(Bar& bar) override;

	virtual void   Undo() override {}
	virtual void   Redo() override {}
	virtual void   Cut() override {}
	virtual void   Copy() override {}
	virtual void   Paste() override {}
	virtual void   SelectAll() override {}
	virtual void   Find() override {}
	virtual void   Replace() override {}

	void SetLayout(const String& form_path);
	void ClearSprites();
	void SetSprite(const String& id, const String& asset_path, int x, int y);
	void MoveSprite(const String& id, int x, int y);

private:
	String path;
	String form_path;
	
	struct Sprite {
		Image img;
		Rect  rect;
	};
	ArrayMap<String, Sprite> sprites;
	Color background_color = Color(40, 160, 40);

	RichTextView game_log;

	virtual void Paint(Draw& w) override;
	virtual void LeftDown(Point p, dword flags) override;
};

END_UPP_NAMESPACE

#endif
