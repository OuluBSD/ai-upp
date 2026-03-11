#ifndef _ScriptIDE_CardGamePlugin_h_
#define _ScriptIDE_CardGamePlugin_h_

// No includes here - they are in ScriptIDE.h or Main package header

class CardGamePluginGUI : public CardGamePlugin {
public:
	CardGamePluginGUI();
	virtual ~CardGamePluginGUI();

	// IPlugin overrides
	virtual void   Init(IPluginContext& context) override;
	virtual void   Shutdown() override;

	// File Type Handlers
	struct GameStateHandler : public IFileTypeHandler {
		CardGamePluginGUI* plugin;
		virtual String         GetExtension() const override { return ".gamestate"; }
		virtual String         GetFileDescription() const override { return "Game State JSON"; }
		virtual IDocumentHost* CreateDocumentHost() override;
	} gamestate_handler;

	struct FormHandler : public IFileTypeHandler {
		CardGamePluginGUI* plugin;
		virtual String         GetExtension() const override { return ".form"; }
		virtual String         GetFileDescription() const override { return "Card Game Layout"; }
		virtual IDocumentHost* CreateDocumentHost() override;
	} form_handler;
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

	void SetLayout(const String& form_path);
	void ClearSprites();
	void SetSprite(const String& id, const String& asset_path, int x, int y);
	void MoveSprite(const String& id, int x, int y, bool animated = false);
	void MoveSpriteToZone(const String& id, const String& zone_id, bool animated = false);
	Rect GetZoneRect(const String& id);
	void Log(const String& msg);

private:
	String path;
	String form_path;
	
	struct Sprite {
		Image img;
		Rect  rect;
		Rect  target_rect;
		bool  animating = false;
	};
	ArrayMap<String, Sprite> sprites;
	
	struct Zone {
		String id;
		Rect   rect;
		String anchor;
		String type;
	};
	ArrayMap<String, Zone> zones;
	
	Color background_color = Color(40, 160, 40);

	RichTextView game_log;

	void Animate();
	Rect GetAbsoluteRect(const Rect& r, const String& anchor, const Size& parent_sz);
	virtual void Paint(Draw& w) override;
	virtual void LeftDown(Point p, dword flags) override;
};

class CardGameProperties : public PropertiesWindow {
public:
	void Generate(FormObject* pI, int index);
};

class CardGameLayoutEditor : public FormEdit<ParentCtrl>, public IDocumentHost {
public:
	CardGameLayoutEditor();
	virtual ~CardGameLayoutEditor();

	// IDocumentHost
	virtual Ctrl&  GetCtrl() override { return *this; }
	virtual bool   Load(const String& path) override;
	virtual bool   Save() override;
	virtual bool   SaveAs(const String& path) override;
	virtual String GetPath() const override { return path; }
	virtual bool   IsModified() const override { return !const_cast<CardGameLayoutEditor*>(this)->IsProjectSaved(); }
	virtual void   SetFocus() override { FormEdit<ParentCtrl>::SetFocus(); }
	
	virtual void   ActivateUI() override;
	virtual void   DeactivateUI() override;
	virtual void   MainMenu(Bar& bar) override;
	virtual void   Toolbar(Bar& bar) override;

	void OpenCardProperties(const Vector<int>& indexes);

private:
	String path;
	CardGameProperties card_properties;
};

#endif
