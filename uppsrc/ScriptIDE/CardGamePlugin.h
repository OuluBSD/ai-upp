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

class CardGameDocumentHost;

class CardGameOverlay : public Ctrl {
public:
	CardGameDocumentHost* owner = nullptr;

	CardGameOverlay();

	virtual void Paint(Draw& w) override;
	virtual void LeftDown(Point p, dword flags) override;
};

class CardSpriteHitCtrl : public Ctrl {
public:
	CardGameDocumentHost* owner = nullptr;
	String card_id;

	CardSpriteHitCtrl();

	virtual void LeftDown(Point p, dword flags) override;
};

class CardSpriteCtrl : public ImageCtrl {
public:
	CardGameDocumentHost* owner = nullptr;
	String card_id;

	CardSpriteCtrl();

	virtual void LeftDown(Point p, dword flags) override;
};

class CardGameDocumentHost : public IDocumentHost, public IHeartsView, public Ctrl {
public:
	CardGameDocumentHost();
	virtual ~CardGameDocumentHost();
	static bool log_to_stdout;

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
	virtual bool   CanRun() const override { return true; }
	virtual bool   IsRunning() const override { return game_running || stop_in_progress || vm_thread_running; }
	virtual bool   CanPause() const override { return true; }
	virtual bool   IsPaused() const override { return game_paused; }
	virtual void   Run() override;
	virtual void   Debug() override;
	virtual void   Profile() override;
	virtual void   Pause() override;
	virtual void   Stop() override;
	virtual void   PopulateDebugState(PythonIDE& ide) override;

	// IHeartsView
	virtual void  SetCard(const String& card_id, const String& asset_path, int x, int y, int rotation_deg = 0) override;
	virtual void  MoveCardToZone(const String& card_id, const String& zone_id, int offset, bool animated) override;
	virtual Value GetZoneRect(const String& zone_id) override;
	virtual void  ClearSprites() override;
	virtual void  SetLabel(const String& zone_id, const String& text) override;
	virtual void  SetButton(const String& zone_id, const String& text, bool enabled) override;
	virtual void  SetHighlight(const String& zone_id, bool enabled) override;
	virtual void  SetStatus(const String& text) override;
	virtual void  Log(const String& msg) override;
	virtual void  SetTimeout(int delay_ms, const String& callback_name) override;

	void SetLayout(const String& form_path);
	void SetPlugin(CardGamePlugin* p) { registration_plugin = p; }
	String DumpScene();
	void DebugInvokeButton(const String& button_id) { InvokePythonButton(button_id); }
	void DebugInvokeCard(const String& card_id) { InvokePythonCard(card_id); }
	void DebugInvokeFirstHandCards(int count);

private:
	String path;
	String form_path;
	CardGamePlugin* registration_plugin = nullptr;
	CardGamePlugin* plugin = nullptr;
	One<CardGamePlugin> runtime_plugin;
	One<HeadlessPluginContext> runtime_context;
	PyVM vm;
	Thread vm_thread;
	Mutex vm_mutex;
	ConditionVariable vm_cv;
	Vector<Function<void ()>> vm_tasks;
	bool vm_shutdown = false;
	bool vm_thread_running = false;
	Mutex ui_mutex;
	Vector<Function<void ()>> ui_commands;
	bool ui_flush_pending = false;
	Mutex rect_cache_mutex;
	ArrayMap<String, Rect> rect_cache;
	Size last_layout_size;
	bool refresh_running = false;
	bool resize_refresh_pending = false;
	bool scene_sync_pending = false;
	bool debug_overlay = false;
	bool game_running = false;
	bool game_paused = false;
	bool stop_requested = false;
	bool stop_in_progress = false;
	String last_error;
	String execution_mode = "run";
	String pending_start_mode;
	String pending_callback_name;
	int pending_timeout_ms = -1;
	int callback_timer_key = 0;
	Form table_form;
	CardGameOverlay overlay;
	
	struct Sprite {
		Image  img;
		String asset_path;
		Rect   rect;
		Rect   target_rect;
		int    rotation_deg = 0;
		bool   animating = false;
	};
	ArrayMap<String, Sprite> sprites;
	ArrayMap<String, CardSpriteCtrl*> card_ctrls;
	ArrayMap<String, Image> image_cache;
	
	struct FormItem {
		String id;
		String anchor;
		String user_class;
	};
	ArrayMap<String, FormItem> form_items;
	ArrayMap<String, String> labels;
	struct ActionButton {
		String text;
		bool   enabled = false;
	};
	ArrayMap<String, ActionButton> buttons;
	Index<String> highlights;
	String status_text;
	Index<String> active_cards;
	
	Color background_color = Color(40, 160, 40);

	RichTextView game_log;
	Vector<String> game_log_lines;
	Vector<PyVM::StackFrame> paused_stack;
	VectorMap<PyValue, PyValue> paused_globals;

	void Animate();
	void InitRuntime();
	void StartGame(const String& mode);
	void ResumeGame();
	void FinishStop();
	void StartVmThread();
	void StopVmThread();
	void VmThreadMain();
	void QueueVmTask(Function<void ()> fn);
	void QueueVmRefresh();
	void QueueVmNamedCallback(const String& callback_name);
	void QueueUiCommand(Function<void ()> fn);
	void ScheduleUiFlush();
	void DrainUiQueue();
	void ApplyClearSprites();
	void ApplySetLabel(const String& zone_id, const String& text);
	void ApplySetButton(const String& zone_id, const String& text, bool enabled);
	void ApplySetHighlight(const String& zone_id, bool enabled);
	void ApplySetStatus(const String& text);
	void ApplyLog(const String& msg);
	void ApplySetCard(const String& card_id, const String& asset_path, int x, int y, int rotation_deg);
	void ApplyMoveCardToZone(const String& card_id, const String& zone_id, int offset, bool animated);
	void ApplySetTimeout(int delay_ms, const String& callback_name);
	void CapturePausedDebugState();
	void ReportVmError(const String& where, const String& msg);
	void ResetGameView();
	void RefreshGameView();
	void InvokePythonButton(const String& button_id);
	void InvokePythonCard(const String& card_id);
	void SyncCardCtrl(const String& card_id);
	void ClearCardCtrls();
	void ScheduleSceneSync();
	void SyncFormExplorer();
	void SyncFormControls();
	void PaintOverlay(Draw& w);
	void OverlayLeftDown(Point p, dword flags);
	virtual void Layout() override;
	virtual void Paint(Draw& w) override;
	friend class CardGameOverlay;
	friend class CardSpriteHitCtrl;
	friend class CardSpriteCtrl;
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
	ParentCtrl main;
};

#endif
