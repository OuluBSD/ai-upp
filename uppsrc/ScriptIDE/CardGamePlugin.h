#ifndef _ScriptIDE_CardGamePlugin_h_
#define _ScriptIDE_CardGamePlugin_h_

// No includes here - they are in ScriptIDE.h or Main package header

class CardGamePluginGUI : public CardGamePlugin {
public:
	CardGamePluginGUI();
	virtual ~CardGamePluginGUI();

	// IPlugin overrides
	virtual String GetID() const override { return "CardGamePluginGUI"; }
	virtual String GetName() const override { return "Card Game Engine (GUI)"; }
	virtual void   Init(IPluginContext& context) override;
	virtual void   Shutdown() override;

	virtual void   ExecuteSeparateWindow(const String& path) override;
	virtual void   DebugSeparateWindow(const String& path) override;

	// File Type Handlers
	struct GameStateHandler : public IFileTypeHandler {
		CardGamePluginGUI* plugin;
		virtual String         GetExtension() const override { return ".gamestate"; }
		virtual String         GetFileDescription() const override { return "Game State JSON"; }
		virtual bool           SupportsHostRole(HostRole role) const override { return role == HOSTROLE_EDITOR; }
		virtual IDocumentHost* CreateEditorHost() override;
	} gamestate_handler;

	struct FormHandler : public IFileTypeHandler {
		CardGamePluginGUI* plugin;
		virtual String         GetExtension() const override { return ".form"; }
		virtual String         GetFileDescription() const override { return "Card Game Layout"; }
		virtual bool           SupportsHostRole(HostRole role) const override { return role == HOSTROLE_EDITOR; }
		virtual IDocumentHost* CreateEditorHost() override;
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

class CardSpriteCtrl : public Ctrl {
public:
	CardGameDocumentHost* owner = nullptr;
	String card_id;

	CardSpriteCtrl();

	virtual void LeftDown(Point p, dword flags) override;
};

class CardGameDocumentHost : public IDocumentHost, public IVideoRenderSource, public IHeartsView, public Ctrl {
public:
	CardGameDocumentHost();
	virtual ~CardGameDocumentHost();
	static bool log_to_stdout;
	static bool exit_on_assert;

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
	virtual RunMode GetRunMode() const override {
		if(execution_mode == "debug")   return RUNMODE_DEBUG;
		if(execution_mode == "profile") return RUNMODE_PROFILE;
		return game_running || stop_in_progress || vm_thread_running ? RUNMODE_RUN : RUNMODE_NONE;
	}
	virtual bool   CanRecordVideo() const override { return game_running && !game_paused; }
	virtual Size   GetRecordFrameSize() const override { return table_form.GetSize().cx > 0 ? table_form.GetSize() : GetSize(); }
	virtual Image  CaptureRecordFrame(Size target_size = Size()) const override;
	virtual void   Run() override;
	virtual void   Debug() override;
	virtual void   Profile() override;
	virtual void   Pause() override;
	virtual void   Stop() override;
	virtual void   PopulateDebugState(PythonIDE& ide) override;
	virtual String DumpPythonStack() const override;

	// IHeartsView
	virtual void  BeginSpriteFrame() override;
	virtual void  SetCard(const String& card_id, const String& asset_path, int x, int y, int rotation_deg = 0) override;
	virtual void  MoveCardToZone(const String& card_id, const String& zone_id, int offset, bool animated) override;
	virtual Value GetZoneRect(const String& zone_id) override;
	virtual void  ClearSprites() override;
	virtual void  RemoveSprite(const String& card_id) override;
	virtual void  SetExpectedSpriteCount(const String& zone_id, int count) override;
	virtual void  SetLabel(const String& zone_id, const String& text) override;
	virtual void  SetLabelColor(const String& zone_id, int r, int g, int b) override;
	virtual void  SetZoneRect(const String& zone_id, int x, int y, int w, int h) override;
	virtual void  SetButton(const String& zone_id, const String& text, bool enabled) override;
	virtual void  SetHighlight(const String& zone_id, bool enabled) override;
	virtual void  SetStatus(const String& text) override;
	virtual void  Log(const String& msg) override;
	virtual void  SetTimeout(int delay_ms, const String& callback_name) override;
	virtual Value GetConfig(const String& key) override;
	virtual const ArrayMap<String, CardGameSprite>& GetSprites() const override { return sprite_export; }
	virtual void  DrawImage(const String& id, const String& asset_path, int x, int y, int w, int h) override;
	virtual void  SetCardDim(const String& card_id, const String& asset_path, int x, int y, int rotation_deg = 0) override;
	virtual void  DrawRect(int x, int y, int w, int h, int r, int g, int b, bool interlaced = false) override;
	virtual Value GetCanvasSize() override;
	virtual void  SetButtonImage(const String& zone_id, const String& normal_asset, const String& hover_asset, const String& pressed_asset) override;
	virtual const Form& GetLayout() const { return table_form; }

	virtual void SetLayout(const String& form_path) override;
	void SetPlugin(CardGamePlugin* p) { registration_plugin = p; if(p) p->SetView(this); }
	void SetFixedArea(Size sz)        { fixed_area = sz; }
	virtual String DumpScene() override;
	void DebugInvokeButton(const String& button_id) { InvokePythonButton(button_id); }
	bool DebugPressFormButton(const String& button_id);
	bool DebugCallFormButtonAction(const String& button_id);
	bool DebugExportStandalone(const String& output_exe_path, String& final_output_path, String& error_text);
	void ExportStandaloneExecutable();
	void DebugInvokeCard(const String& card_id) { InvokePythonCard(card_id); }
	void DebugInvokeDrag(const String& card_id, const String& zone_id) { InvokePythonDrag(card_id, zone_id); }
	void DebugInvokeFirstHandCards(int count);
	virtual PyVM* GetVM() override { return &vm; }

private:
	bool ExportStandalonePackage(const String& output_exe_path, String& final_output_path, String& error_text);

	String path;
	Value  gs;
	String form_path;
	bool browser_host_enabled = false;
	int browser_host_port = -1;
	String browser_host_url;
	String browser_host_session_id;
	byte browser_launch_timer_key = 0;
	CardGamePlugin* registration_plugin = nullptr;
	CardGamePlugin* plugin = nullptr;
	One<CardGamePlugin> runtime_plugin;
	One<HeadlessPluginContext> runtime_context;
	One<LocalProcess> web_host_process;
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
	int ui_batch_depth = 0;
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
	
	struct Sprite : Pte<Sprite> {
		Image  img;
		String asset_path;
		Rect   rect;
		Rect   target_rect;
		int    rotation_deg = 0;
		bool   animating = false;
		bool   dimmed = false;
	};
	struct VisualRect : Moveable<VisualRect> {
		Rect  rect;
		Color color;
		bool  interlaced = false;
	};
	ArrayMap<String, Ptr<Sprite>> sprites;
	ArrayMap<String, CardGameSprite> sprite_export; // mirrors sprites for GetSprites()
	ArrayMap<String, CardSpriteCtrl*> card_ctrls;
	ArrayMap<String, Image> image_cache;
	
	struct FormItem {
		String id;
		String anchor;
		String user_class;
		Rect   design_rect;
	};
	ArrayMap<String, FormItem> form_items;
	ArrayMap<String, int> expected_sprite_counts;
	ArrayMap<String, String> labels;
	ArrayMap<String, Color> label_colors;
	ArrayMap<String, Rect> zone_rect_overrides;
	struct ActionButton {
		String text;
		bool   enabled = false;
		String normal_asset;
		String hover_asset;
		String pressed_asset;
	};
	ArrayMap<String, ActionButton> buttons;
	Array<VisualRect> visual_rects;
	Index<String> highlights;
	String status_text;
	Size design_form_size;
	Rect viewport_rect;
	Index<String> active_cards;
	String last_form_button_event_id;
	int64 last_form_button_event_ms = -1000;
	struct DragState {
		String card_id;
		Point start_point;
		Point grab_offset;
		Rect  original_rect;
		bool  moved = false;
		bool  active = false;
	} drag_state;
	
	struct SpritePaintOrder : Moveable<SpritePaintOrder> {
		Ptr<Sprite> sprite;
		String      card_id;
		int group = 0;
		int major = 0;
		int minor = 0;
	};

	Color background_color = Color(40, 160, 40);
	Size  fixed_area;

	RichTextView game_log;
	Vector<String> game_log_lines;
	Vector<PyVM::StackFrame> paused_stack;
	Vector<PyVM::StackFrame> last_error_stack;
	VectorMap<PyValue, PyValue> paused_globals;

	void Animate();
	void LoadGameStateSettings();
	bool StartBrowserHost(const String& mode);
	void StopBrowserHost();
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
	void OnFormSignal(const String& script, const String& signal, const String& action);
	void ApplyBeginSpriteFrame();
	void ApplyClearSprites();
	void ApplyRemoveSprite(const String& card_id);
	void ApplySetExpectedSpriteCount(const String& zone_id, int count);
	void ApplySetLabel(const String& zone_id, const String& text);
	void ApplySetLabelColor(const String& zone_id, int r, int g, int b);
	void ApplySetZoneRect(const String& zone_id, int x, int y, int w, int h);
	void ApplySetButton(const String& zone_id, const String& text, bool enabled);
	void ApplySetHighlight(const String& zone_id, bool enabled);
	void ApplySetStatus(const String& text);
	void ApplyLog(const String& msg);
	void ApplySetCard(const String& card_id, const String& asset_path, int x, int y, int rotation_deg);
	void ApplyDrawImage(const String& id, const String& asset_path, int x, int y, int w, int h);
	void ApplySetCardDim(const String& card_id, const String& asset_path, int x, int y, int rotation_deg);
	void ApplyDrawRect(int x, int y, int w, int h, int r, int g, int b, bool interlaced);
	void ApplySetButtonImage(const String& zone_id, const String& normal_asset, const String& hover_asset, const String& pressed_asset);
	void ApplyMoveCardToZone(const String& card_id, const String& zone_id, int offset, bool animated);
	void ApplySetTimeout(int delay_ms, const String& callback_name);
	void EnsureViewportLayout();
	void PruneInactiveSprites();
	bool CheckExpectedSpriteCounts();
	void CapturePausedDebugState();
	void ReportVmError(const String& where, const String& msg);
	void ResetGameView();
	void RefreshGameView();
	void FireFormButtonEvent(const String& button_id, const char* source);
	void InvokePythonButton(const String& button_id);
	void InvokePythonCard(const String& card_id);
	void InvokePythonDrag(const String& card_id, const String& zone_id);
	void SyncCardCtrl(const String& card_id);
	void ClearCardCtrls();
	String FindDropZone(Point p) const;
	bool BeginCardDrag(const String& card_id, Point p);
	void UpdateCardDrag(Point p);
	void EndCardDrag(Point p);
	void ScheduleSceneSync();
	void SyncFormExplorer();
	void SyncFormControls();
	void RetryLayoutRefresh();
	void PaintOverlay(Draw& w);
	void PaintOverlayScaled(Draw& w, double sx, double sy, bool include_debug) const;
	void OverlayLeftDown(Point p, dword flags);
	virtual void MouseMove(Point p, dword flags) override;
	virtual void LeftUp(Point p, dword flags) override;
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
	virtual bool   IsModified() const override;
	virtual void   SetFocus() override { FormEdit<ParentCtrl>::SetFocus(); }
	
	virtual void   ActivateUI() override;
	virtual void   DeactivateUI() override;
	virtual void   MainMenu(Bar& bar) override;
	virtual void   Toolbar(Bar& bar) override;

	void OpenCardProperties(const Vector<int>& indexes);
	void RefreshSavedState();

private:
	String path;
	String last_saved_xml;
	CardGameProperties card_properties;
	ParentCtrl main;
};

#endif
