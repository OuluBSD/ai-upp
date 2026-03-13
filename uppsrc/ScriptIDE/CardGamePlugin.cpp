#include "ScriptIDE.h"
#include "CardGamePlugin.h"

namespace {

struct CardGameZoneDef : Moveable<CardGameZoneDef> {
	String id;
	String anchor;
	String user_class;
	String parent;
	String label;
};

String NormalizeUserClass(const String& user_class, const String& control_type = String())
{
	String type = ToUpper(user_class);
	if(type.IsEmpty()) {
		if(control_type == "Button")
			type = "BUTTON";
		else if(control_type == "Label")
			type = "LABEL";
	}
	return type;
}

String UserClassToControlType(const String& user_class)
{
	return NormalizeUserClass(user_class) == "BUTTON" ? "Button" : "Label";
}

Color ParseBackgroundColor(const String& value, const Color& fallback)
{
	Vector<String> parts = Split(value, ',');
	if(parts.GetCount() != 3)
		return fallback;
	return Color(ScanInt(parts[0]), ScanInt(parts[1]), ScanInt(parts[2]));
}

bool LoadCardGameFormView(FormView& view, const String& path, Color& background_color)
{
	background_color = Color(40, 160, 40);
	if(!view.LoadAll(path, false))
		return false;

	if(!view.IsLayout() && view.GetLayoutCount() > 0)
		view.SelectLayout(0);
	if(view.IsLayout()) {
		String bg = view.GetCurrentLayout()->Get("CardGame.Background");
		if(!bg.IsEmpty())
			background_color = ParseBackgroundColor(bg, background_color);
	}
	return true;
}

Vector<CardGameZoneDef> ExtractCardGameZones(const FormView& view)
{
	Vector<CardGameZoneDef> defs;
	const FormLayout* layout = view.GetCurrentLayout();
	if(!layout)
		return defs;

	const Array<FormObject>& objects = layout->GetObjects();
	for(int i = 0; i < objects.GetCount(); i++) {
		const FormObject& obj = objects[i];
		CardGameZoneDef& def = defs.Add();
		def.id = obj.Get("Variable");
		def.anchor = obj.Get("Anchor");
		if(def.anchor.IsEmpty())
			def.anchor = "TOP_LEFT";
		def.user_class = NormalizeUserClass(obj.Get("UserClass"), obj.Get("Type"));
		def.parent = obj.Get("Parent");
		def.label = obj.Get("Label");
	}
	return defs;
}

Rect GetZoneRectFromForm(const Form& form, const String& zone_id)
{
	if(Ctrl* ctrl = const_cast<Form&>(form).GetCtrl(zone_id)) {
		Rect r = ctrl->GetRect();
		for(Ctrl* parent = ctrl->GetParent(); parent && parent != &form; parent = parent->GetParent())
			r.Offset(parent->GetRect().TopLeft());
		return r;
	}
	return Rect();
}

Image RotateCardImage(const Image& img, int rotation_deg)
{
	int rot = rotation_deg % 360;
	if(rot < 0)
		rot += 360;
	if(rot == 90)
		return RotateClockwise(img);
	if(rot == 180)
		return Rotate180(img);
	if(rot == 270)
		return RotateAntiClockwise(img);
	return img;
}

Size RotatedImageSize(const Image& img, int rotation_deg)
{
	Image rotated = RotateCardImage(img, rotation_deg);
	return rotated.GetSize();
}

}

// --- CardGameDocumentHost ---

bool CardGameDocumentHost::log_to_stdout = false;

CardGameOverlay::CardGameOverlay()
{
	Transparent();
}

void CardGameOverlay::Paint(Draw& w)
{
	if(owner)
		owner->PaintOverlay(w);
}

void CardGameOverlay::LeftDown(Point p, dword flags)
{
	if(owner)
		owner->OverlayLeftDown(p, flags);
}

CardSpriteHitCtrl::CardSpriteHitCtrl()
{
	Transparent();
	NoWantFocus();
}

void CardSpriteHitCtrl::LeftDown(Point p, dword flags)
{
	if(owner)
		owner->InvokePythonCard(card_id);
}

CardSpriteCtrl::CardSpriteCtrl()
{
	NoWantFocus();
}

void CardSpriteCtrl::LeftDown(Point p, dword flags)
{
	if(owner)
		owner->InvokePythonCard(card_id);
}

CardGameDocumentHost::CardGameDocumentHost()
{
	game_log_lines << "Welcome to the Game!" << "Ready to play.";
	game_log.SetQTF("Welcome to the Game!&Ready to play.");
	Add(table_form.SizePos());
	overlay.owner = this;
	overlay.NoWantFocus();
	overlay.IgnoreMouse();
	Add(overlay.SizePos());
	Upp::SetTimeCallback(-16, [=] { Animate(); }, this);
}

CardGameDocumentHost::~CardGameDocumentHost()
{
	Upp::KillTimeCallback(&callback_timer_key);
	StopVmThread();
	if(runtime_plugin)
		runtime_plugin->Shutdown();
	runtime_plugin.Clear();
	runtime_context.Clear();
	plugin = nullptr;
	ClearCardCtrls();
	Upp::KillTimeCallback(this);
	Upp::KillTimeCallback(&resize_refresh_pending);
	Upp::KillTimeCallback(&scene_sync_pending);
}

bool CardGameDocumentHost::Load(const String& path_)
{
	path = path_;

	// Load the form layout if specified in the gamestate
	String json = LoadFile(path);
	Value gs = ParseJSON(json);
	if(!gs.IsVoid()) {
		String layout = gs["layout"];
		if(!layout.IsEmpty())
			SetLayout(AppendFileName(GetFileDirectory(path), layout));
	}

	Refresh();
	SyncFormExplorer();

	return true;
}

void CardGameDocumentHost::Layout()
{
	Ctrl::Layout();

	Size sz = GetSize();
	if(sz == last_layout_size)
		return;

	last_layout_size = sz;
	if(sz.cx <= 0 || sz.cy <= 0)
		return;

	SyncFormControls();
	SyncFormExplorer();

	if(resize_refresh_pending)
		return;

	resize_refresh_pending = true;
	Upp::PostCallback(callback(this, &CardGameDocumentHost::RetryLayoutRefresh), &resize_refresh_pending);
}

void CardGameDocumentHost::RetryLayoutRefresh()
{
	if(!resize_refresh_pending)
		return;
	if(refresh_running) {
		Upp::PostCallback(callback(this, &CardGameDocumentHost::RetryLayoutRefresh), &resize_refresh_pending);
		return;
	}
	resize_refresh_pending = false;
	RefreshGameView();
}

void CardGameDocumentHost::ActivateUI()
{
	SyncFormExplorer();
}

void CardGameDocumentHost::DeactivateUI()
{
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		if(ide->form_explorer)
			ide->form_explorer->Clear();
	}
}

void CardGameDocumentHost::MainMenu(Bar& bar)
{
	bar.Sub("Game", [=](Bar& b) {
		b.Add(game_running ? "Stop" : "Run", [=] {
			if(game_running) Stop();
			else Run();
		});
		b.Add("Debug Overlay", [=] {
			debug_overlay = !debug_overlay;
			overlay.Refresh();
		}).Check(debug_overlay);
		b.Add("Restart Match", [=] { Todo("Restart"); });
		b.Add("Concede", [=] { Todo("Concede"); });
	});
}

void CardGameDocumentHost::Toolbar(Bar& bar)
{
	bar.Add(game_running ? "Stop" : "Run", game_running ? Icons::Stop() : CtrlImg::right_arrow(), [=] {
		if(game_running) Stop();
		else Run();
	});
	bar.Add("Debug", [=] {
		debug_overlay = !debug_overlay;
		overlay.Refresh();
	}).Check(debug_overlay);
}

void CardGameDocumentHost::InitRuntime()
{
	Upp::KillTimeCallback(&callback_timer_key);
	StopVmThread();
	if(runtime_plugin)
		runtime_plugin->Shutdown();
	runtime_plugin.Clear();
	runtime_context.Clear();
	vm.Clear();

	runtime_context.Create(vm);
	runtime_plugin.Create();
	plugin = ~runtime_plugin;
	plugin->SetView(this);
	plugin->Init(*runtime_context);
}

void CardGameDocumentHost::StartGame(const String& mode)
{
	if(stop_in_progress) {
		pending_start_mode = mode;
		return;
	}
	if(game_running) {
		pending_start_mode = mode;
		Stop();
		return;
	}
	execution_mode = mode;
	last_error.Clear();
	game_log_lines.Clear();
	game_log_lines << "Starting Hearts game (" + mode + ")...";
	game_log.SetQTF(DeQtfLf(game_log_lines[0]));
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		ide->Log("--- Running " + GetFileName(path) + " (" + mode + ") ---");
		ide->Log("Python engine: ByteVM");
		ide->Log("Working directory: " + GetFileDirectory(path));
	}
	InitRuntime();
	vm.EnableBreakpoints(mode != "run");
	StartVmThread();
	game_running = true;
	game_paused = false;
	stop_requested = false;
	stop_in_progress = false;
	paused_stack.Clear();
	paused_globals.Clear();
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow()))
		ide->RefreshRunStateUI();
	QueueVmTask([=] {
		if(!plugin)
			return;
		try {
			plugin->Execute(path);
		}
		catch(Exc& e) {
			ReportVmError("Execute", e);
		}
	});
	PostCallback([=] { RefreshGameView(); });
}

void CardGameDocumentHost::ResumeGame()
{
	{
		Mutex::Lock __(vm_mutex);
		game_paused = false;
		vm_cv.Broadcast();
	}
	stop_requested = false;
	if(!pending_callback_name.IsEmpty() && pending_timeout_ms >= 0)
		ApplySetTimeout(pending_timeout_ms, pending_callback_name);
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow()))
		ide->RefreshRunStateUI();
}

void CardGameDocumentHost::StartVmThread()
{
	StopVmThread();
	vm_shutdown = false;
	vm_thread_running = vm_thread.Run([=] { VmThreadMain(); }, true);
	ASSERT(vm_thread_running);
}

void CardGameDocumentHost::StopVmThread()
{
	bool wait = false;
	{
		Mutex::Lock __(vm_mutex);
		if(vm_thread_running) {
			vm_shutdown = true;
			vm_cv.Broadcast();
			wait = true;
		}
	}
	if(wait) {
		vm_thread.Wait();
		vm_thread_running = false;
	}
	{
		Mutex::Lock __(vm_mutex);
		vm_tasks.Clear();
		vm_shutdown = false;
	}
}

void CardGameDocumentHost::Run()
{
	if(game_running && game_paused) {
		ResumeGame();
		return;
	}
	StartGame("run");
}

void CardGameDocumentHost::Debug()
{
	if(game_running && game_paused) {
		ResumeGame();
		return;
	}
	StartGame("debug");
}

void CardGameDocumentHost::Profile()
{
	if(game_running && game_paused) {
		ResumeGame();
		return;
	}
	StartGame("profile");
}

void CardGameDocumentHost::Pause()
{
	if(!game_running || game_paused)
		return;
	Upp::KillTimeCallback(&callback_timer_key);
	QueueVmTask([=] {
		{
			Mutex::Lock __(vm_mutex);
			game_paused = true;
		}
		if(execution_mode != "run")
			CapturePausedDebugState();
		QueueUiCommand([=] {
			if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
				if(execution_mode != "run")
					PopulateDebugState(*ide);
				ide->RefreshRunStateUI();
			}
			if(stop_requested)
				PostCallback([=] { FinishStop(); });
		});
	});
}

void CardGameDocumentHost::Stop()
{
	if(stop_in_progress)
		return;
	if(game_running && !game_paused) {
		stop_requested = true;
		stop_in_progress = true;
		Pause();
		return;
	}
	stop_in_progress = true;
	FinishStop();
}

void CardGameDocumentHost::FinishStop()
{
	bool had_session = game_running || game_paused || stop_requested || vm_thread_running || (bool)runtime_plugin;
	Upp::KillTimeCallback(&callback_timer_key);
	StopVmThread();
	if(runtime_plugin)
		runtime_plugin->Shutdown();
	runtime_plugin.Clear();
	runtime_context.Clear();
	plugin = nullptr;
	game_running = false;
	game_paused = false;
	stop_requested = false;
	stop_in_progress = false;
	execution_mode = "run";
	pending_callback_name.Clear();
	pending_timeout_ms = -1;
	paused_stack.Clear();
	paused_globals.Clear();
	ResetGameView();
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		if(had_session) {
			ide->Log("--- Script finished ---");
			ide->Log("Exit: stopped");
		}
		ide->RefreshRunStateUI();
	}
	if(!pending_start_mode.IsEmpty()) {
		String mode = pending_start_mode;
		pending_start_mode.Clear();
		PostCallback([=] { StartGame(mode); });
	}
}

void CardGameDocumentHost::VmThreadMain()
{
	for(;;) {
		Function<void ()> task;
		{
			Mutex::Lock __(vm_mutex);
			while(!vm_shutdown && (vm_tasks.IsEmpty() || game_paused))
				vm_cv.Wait(vm_mutex);
			if(vm_shutdown) {
				vm_tasks.Clear();
				break;
			}
			task = pick(vm_tasks[0]);
			vm_tasks.Remove(0);
		}
		try {
			task();
		}
		catch(Exc& e) {
			ReportVmError("VM task", e);
		}
	}
}

void CardGameDocumentHost::QueueVmTask(Function<void ()> fn)
{
	Mutex::Lock __(vm_mutex);
	if(vm_shutdown || !vm_thread_running)
		return;
	vm_tasks.Add(pick(fn));
	vm_cv.Signal();
}

void CardGameDocumentHost::QueueVmRefresh()
{
	QueueVmTask([=] {
		if(!plugin)
			return;
		PyValue refresh_ui = plugin->GetGameFunction("refresh_ui");
		if(!refresh_ui.IsFunction())
			return;
		try {
			vm.Call(refresh_ui, {});
		}
		catch(Exc& e) {
			ReportVmError("refresh_ui", e);
		}
	});
}

void CardGameDocumentHost::QueueVmNamedCallback(const String& callback_name)
{
	QueueVmTask([=] {
		if(!plugin)
			return;
		PyValue cb = plugin->GetGameFunction(callback_name);
		if(!cb.IsFunction())
			return;
		try {
			vm.Call(cb, {});
		}
		catch(Exc& e) {
			ReportVmError(callback_name, e);
		}
	});
}

void CardGameDocumentHost::QueueUiCommand(Function<void ()> fn)
{
	bool post = false;
	{
		Mutex::Lock __(ui_mutex);
		ui_commands.Add(pick(fn));
		if(!ui_flush_pending) {
			ui_flush_pending = true;
			post = true;
		}
	}
	if(post)
		PostCallback([=] { DrainUiQueue(); });
}

void CardGameDocumentHost::ScheduleUiFlush()
{
	bool post = false;
	{
		Mutex::Lock __(ui_mutex);
		if(!ui_flush_pending) {
			ui_flush_pending = true;
			post = true;
		}
	}
	if(post)
		PostCallback([=] { DrainUiQueue(); });
}

void CardGameDocumentHost::DrainUiQueue()
{
	ASSERT(IsMainThread());

	Vector<Function<void ()>> cmds;
	{
		Mutex::Lock __(ui_mutex);
		ui_flush_pending = false;
		Swap(cmds, ui_commands);
	}

	for(int i = 0; i < cmds.GetCount(); i++)
		cmds[i]();

	SyncFormControls();
	SyncFormExplorer();
	table_form.Refresh();
	overlay.Refresh();
	Refresh();
}

void CardGameDocumentHost::ApplyClearSprites()
{
	active_cards.Clear();
	for(int i = 0; i < card_ctrls.GetCount(); i++) {
		if(card_ctrls[i])
			card_ctrls[i]->Hide();
	}
	sprites.Clear();
}

void CardGameDocumentHost::ApplySetLabel(const String& zone_id, const String& text)
{
	labels.GetAdd(zone_id) = text;
}

void CardGameDocumentHost::ApplySetButton(const String& zone_id, const String& text, bool enabled)
{
	ActionButton& b = buttons.GetAdd(zone_id);
	b.text = text;
	b.enabled = enabled;
}

void CardGameDocumentHost::ApplySetHighlight(const String& zone_id, bool enabled)
{
	int q = highlights.Find(zone_id);
	if(enabled) {
		if(q < 0)
			highlights.Add(zone_id);
	}
	else if(q >= 0)
		highlights.Remove(q);
}

void CardGameDocumentHost::ApplySetStatus(const String& text)
{
	status_text = text;
}

void CardGameDocumentHost::ApplyLog(const String& msg)
{
	if(log_to_stdout)
		Cout() << msg << "\n";
	game_log_lines.Add(msg);
	const int max_log_lines = 200;
	while(game_log_lines.GetCount() > max_log_lines)
		game_log_lines.Remove(0);
	String qtf;
	for(int i = 0; i < game_log_lines.GetCount(); i++) {
		if(i)
			qtf << "&";
		qtf << DeQtfLf(game_log_lines[i]);
	}
	game_log.SetQTF(qtf);
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow()))
		ide->Log(msg);
}

void CardGameDocumentHost::ApplySetCard(const String& card_id, const String& asset_path, int x, int y, int rotation_deg)
{
	Sprite& s = sprites.GetAdd(card_id);

	String full_path = asset_path;
	if(!IsFullPath(asset_path))
		full_path = AppendFileName(GetFileDirectory(path), asset_path);

	if(s.img.IsEmpty() || s.asset_path != full_path) {
		int q = image_cache.Find(full_path);
		if(q >= 0)
			s.img = image_cache[q];
		else {
			s.img = StreamRaster::LoadFileAny(full_path);
			image_cache.Add(full_path, s.img);
		}
		s.asset_path = full_path;
	}
	s.rotation_deg = rotation_deg;
	Size sprite_sz = RotatedImageSize(s.img, s.rotation_deg);
	s.rect = RectC(x, y, sprite_sz.cx, sprite_sz.cy);
	s.target_rect = s.rect;
	s.animating = false;
	if(active_cards.Find(card_id) < 0)
		active_cards.Add(card_id);
	SyncCardCtrl(card_id);
}

void CardGameDocumentHost::ApplyMoveCardToZone(const String& card_id, const String& zone_id, int offset, bool animated)
{
	int qs = sprites.Find(card_id);
	if(qs < 0)
		return;

	Rect abs_z = GetZoneRectFromForm(table_form, zone_id);
	if(abs_z.IsEmpty())
		return;
	int tx = abs_z.left + (abs_z.GetWidth() / 2) - (sprites[qs].rect.GetWidth() / 2) + offset;
	int ty = abs_z.top  + (abs_z.GetHeight() / 2) - (sprites[qs].rect.GetHeight() / 2);

	Sprite& s = sprites[qs];
	s.target_rect = RectC(tx, ty, s.rect.GetWidth(), s.rect.GetHeight());
	if(animated)
		s.animating = true;
	else {
		s.rect = s.target_rect;
		s.animating = false;
	}
	if(active_cards.Find(card_id) < 0)
		active_cards.Add(card_id);
	SyncCardCtrl(card_id);
}

void CardGameDocumentHost::ApplySetTimeout(int delay_ms, const String& callback_name)
{
	pending_timeout_ms = delay_ms;
	pending_callback_name = callback_name;
	Upp::KillTimeCallback(&callback_timer_key);
	Upp::SetTimeCallback(max(0, delay_ms), [=] {
		QueueVmNamedCallback(callback_name);
	}, &callback_timer_key);
}

void CardGameDocumentHost::CapturePausedDebugState()
{
	paused_stack = vm.GetCallStack();
	paused_globals.Clear();
	const VectorMap<PyValue, PyValue>& globals = vm.GetGlobals().GetDict();
	for(int i = 0; i < globals.GetCount(); i++)
		paused_globals.Add(globals.GetKey(i), globals[i]);
}

void CardGameDocumentHost::ReportVmError(const String& where, const String& msg)
{
	String text = where + " error: " + msg;
	LOG("CardGameDocumentHost: " << text);
	QueueUiCommand([=] {
		last_error = msg;
		ApplyLog(text);
		game_running = false;
		game_paused = false;
		if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow()))
			ide->RefreshRunStateUI();
	});
}

void CardGameDocumentHost::PopulateDebugState(PythonIDE& ide)
{
	ide.ShowDebugState(paused_stack, paused_globals, false);
}

void CardGameDocumentHost::ResetGameView()
{
	ApplyClearSprites();
	labels.Clear();
	buttons.Clear();
	highlights.Clear();
	status_text.Clear();
	last_error.Clear();
	game_log_lines.Clear();
	game_log.SetQTF("");
	if(!form_path.IsEmpty())
		SetLayout(form_path);
	SyncFormControls();
	SyncFormExplorer();
	table_form.Refresh();
	overlay.Refresh();
	Refresh();
}

void CardGameDocumentHost::SetLayout(const String& path)
{
	form_path = path;
	sprites.Clear();
	form_items.Clear();
	labels.Clear();
	buttons.Clear();
	highlights.Clear();

	FormView view;
	if(!LoadCardGameFormView(view, path, background_color))
		return;

	String xml;
	view.SaveAllString(xml, false);
	if(!table_form.LoadString(xml, false))
		return;

	String layout_name = "Default";
	if(view.IsLayout())
		layout_name = view.GetCurrentLayout()->Get("Form.Name", "Default");
	if(!table_form.Layout(layout_name))
		return;

	Vector<CardGameZoneDef> defs = ExtractCardGameZones(view);
	for(const CardGameZoneDef& def : defs) {
		FormItem& item = form_items.GetAdd(def.id);
		item.id = def.id;
		item.anchor = def.anchor;
		item.user_class = def.user_class;
	}

	table_form.SizePos();
	SyncFormControls();
	Refresh();
	SyncFormExplorer();
}

// IHeartsView implementation

void CardGameDocumentHost::ClearSprites()
{
	if(IsMainThread()) {
		ApplyClearSprites();
		DrainUiQueue();
	}
	else
		QueueUiCommand([=] { ApplyClearSprites(); });
}

void CardGameDocumentHost::SetLabel(const String& zone_id, const String& text)
{
	if(IsMainThread()) {
		ApplySetLabel(zone_id, text);
		DrainUiQueue();
	}
	else
		QueueUiCommand([=] { ApplySetLabel(zone_id, text); });
}

void CardGameDocumentHost::SetButton(const String& zone_id, const String& text, bool enabled)
{
	if(IsMainThread()) {
		ApplySetButton(zone_id, text, enabled);
		DrainUiQueue();
	}
	else
		QueueUiCommand([=] { ApplySetButton(zone_id, text, enabled); });
}

void CardGameDocumentHost::SetHighlight(const String& zone_id, bool enabled)
{
	if(IsMainThread()) {
		ApplySetHighlight(zone_id, enabled);
		DrainUiQueue();
	}
	else
		QueueUiCommand([=] { ApplySetHighlight(zone_id, enabled); });
}

void CardGameDocumentHost::SetStatus(const String& text)
{
	if(IsMainThread()) {
		ApplySetStatus(text);
		DrainUiQueue();
	}
	else
		QueueUiCommand([=] { ApplySetStatus(text); });
}

void CardGameDocumentHost::SetCard(const String& card_id, const String& asset_path, int x, int y, int rotation_deg)
{
	if(IsMainThread()) {
		ApplySetCard(card_id, asset_path, x, y, rotation_deg);
		DrainUiQueue();
	}
	else
		QueueUiCommand([=] { ApplySetCard(card_id, asset_path, x, y, rotation_deg); });
}

void CardGameDocumentHost::MoveCardToZone(const String& card_id, const String& zone_id, int offset, bool animated)
{
	if(IsMainThread()) {
		ApplyMoveCardToZone(card_id, zone_id, offset, animated);
		DrainUiQueue();
	}
	else
		QueueUiCommand([=] { ApplyMoveCardToZone(card_id, zone_id, offset, animated); });
}

Value CardGameDocumentHost::GetZoneRect(const String& zone_id)
{
	ValueMap m;
	Rect r;
	if(IsMainThread()) {
		r = GetZoneRectFromForm(table_form, zone_id);
	}
	else {
		Mutex::Lock __(rect_cache_mutex);
		int q = rect_cache.Find(zone_id);
		if(q >= 0)
			r = rect_cache[q];
	}
	if(!r.IsEmpty()) {
		m.Add("x", r.left);
		m.Add("y", r.top);
		m.Add("w", r.GetWidth());
		m.Add("h", r.GetHeight());
	} else {
		m.Add("x", 0); m.Add("y", 0); m.Add("w", 552); m.Add("h", 96);
	}
	return m;
}

String CardGameDocumentHost::DumpScene()
{
	String out;
	out << "path: " << path << "\n";
	out << "form_path: " << form_path << "\n";
	out << "plugin: " << (plugin ? "yes" : "no") << "\n";
	PyVM* vm = plugin && plugin->GetContext() ? plugin->GetContext()->GetVM() : nullptr;
	out << "vm: " << (vm ? "yes" : "no") << "\n";
	if(plugin) {
		out << "callback refresh_ui: " << (plugin->GetGameFunction("refresh_ui").IsFunction() ? "yes" : "no") << "\n";
		out << "callback on_click: " << (plugin->GetGameFunction("on_click").IsFunction() ? "yes" : "no") << "\n";
		out << "callback on_button: " << (plugin->GetGameFunction("on_button").IsFunction() ? "yes" : "no") << "\n";
		out << "callback start: " << (plugin->GetGameFunction("start").IsFunction() ? "yes" : "no") << "\n";
	}
	out << "host_size: " << GetSize().cx << "x" << GetSize().cy << "\n";
	out << "table_form_size: " << table_form.GetSize().cx << "x" << table_form.GetSize().cy << "\n";
	out << "background: " << background_color.GetR() << "," << background_color.GetG() << "," << background_color.GetB() << "\n";
	out << "last_error: " << last_error << "\n";
	out << "game_log_qtf: " << AsCString(game_log.GetQTF()) << "\n";
	out << "form_items: " << form_items.GetCount() << "\n";
	for(int i = 0; i < form_items.GetCount(); i++) {
		const FormItem& item = form_items[i];
		Rect r = GetZoneRectFromForm(table_form, item.id);
		out << "  form_item " << item.id
		    << " class=" << item.user_class
		    << " anchor=" << item.anchor
		    << " rect=" << r.left << "," << r.top << " " << r.GetWidth() << "x" << r.GetHeight();
		if(Ctrl* ctrl = table_form.GetCtrl(item.id))
			out << " ctrl_rect=" << ctrl->GetRect().left << "," << ctrl->GetRect().top << " " << ctrl->GetRect().GetWidth() << "x" << ctrl->GetRect().GetHeight()
			    << " visible=" << (ctrl->IsShown() ? "1" : "0");
		out << "\n";
	}
	out << "labels: " << labels.GetCount() << "\n";
	for(int i = 0; i < labels.GetCount(); i++)
		out << "  label " << labels.GetKey(i) << " text=" << AsCString(labels[i]) << "\n";
	out << "buttons: " << buttons.GetCount() << "\n";
	for(int i = 0; i < buttons.GetCount(); i++)
		out << "  button " << buttons.GetKey(i) << " enabled=" << (buttons[i].enabled ? "1" : "0")
		    << " text=" << AsCString(buttons[i].text) << "\n";
	out << "highlights: " << highlights.GetCount() << "\n";
	for(int i = 0; i < highlights.GetCount(); i++)
		out << "  highlight " << highlights[i] << "\n";
	out << "sprites: " << sprites.GetCount() << "\n";
	for(int i = 0; i < sprites.GetCount(); i++) {
		const String& id = sprites.GetKey(i);
		const Sprite& s = sprites[i];
		int q = card_ctrls.Find(id);
		out << "  sprite " << id
		    << " asset=" << s.asset_path
		    << " img=" << s.img.GetWidth() << "x" << s.img.GetHeight()
		    << " rot=" << s.rotation_deg
		    << " rect=" << s.rect.left << "," << s.rect.top << " " << s.rect.GetWidth() << "x" << s.rect.GetHeight()
		    << " target=" << s.target_rect.left << "," << s.target_rect.top << " " << s.target_rect.GetWidth() << "x" << s.target_rect.GetHeight()
		    << " anim=" << (s.animating ? "1" : "0")
		    << " ctrl=" << (q >= 0 && card_ctrls[q] ? "1" : "0");
		if(q >= 0 && card_ctrls[q]) {
			Rect cr = card_ctrls[q]->GetRect();
			out << " ctrl_rect=" << cr.left << "," << cr.top << " " << cr.GetWidth() << "x" << cr.GetHeight()
			    << " shown=" << (card_ctrls[q]->IsShown() ? "1" : "0");
		}
		out << "\n";
	}
	return out;
}

void CardGameDocumentHost::Log(const String& msg)
{
	if(IsMainThread()) {
		ApplyLog(msg);
		DrainUiQueue();
	}
	else
		QueueUiCommand([=] { ApplyLog(msg); });
}

void CardGameDocumentHost::SetTimeout(int delay_ms, const String& callback_name)
{
	if(IsMainThread())
		ApplySetTimeout(delay_ms, callback_name);
	else
		QueueUiCommand([=] { ApplySetTimeout(delay_ms, callback_name); });
}

void CardGameDocumentHost::Animate()
{
	bool changed = false;
	for(int i = 0; i < sprites.GetCount(); i++) {
		Sprite& s = sprites[i];
		if(s.animating) {
			Point p = s.rect.TopLeft();
			Point tp = s.target_rect.TopLeft();
			
			int dx = tp.x - p.x;
			int dy = tp.y - p.y;
			
			if(Upp::abs(dx) <= 2 && Upp::abs(dy) <= 2) {
				s.rect = s.target_rect;
				s.animating = false;
			} else {
				s.rect = RectC(p.x + dx / 5, p.y + dy / 5, s.rect.GetWidth(), s.rect.GetHeight());
			}
			SyncCardCtrl(sprites.GetKey(i));
			changed = true;
		}
	}
	if(changed) {
		overlay.Refresh();
		for(int i = 0; i < sprites.GetCount(); i++) {
			int q = card_ctrls.Find(sprites.GetKey(i));
			if(q >= 0 && card_ctrls[q])
				card_ctrls[q]->Refresh();
		}
	}
}

void CardGameDocumentHost::RefreshGameView()
{
	if(!game_running)
		return;
	if(refresh_running)
		return;

	refresh_running = true;
	QueueVmTask([=] {
		if(plugin) {
			PyValue refresh_ui = plugin->GetGameFunction("refresh_ui");
			if(refresh_ui.IsFunction()) {
				try {
					vm.Call(refresh_ui, {});
				}
				catch(Exc& e) {
					ReportVmError("refresh_ui", e);
				}
			}
		}
		QueueUiCommand([=] { refresh_running = false; });
	});
}

void CardGameDocumentHost::InvokePythonButton(const String& button_id)
{
	if(!plugin || !game_running)
		return;
	QueueVmTask([=] {
		PyValue on_button = plugin->GetGameFunction("on_button");
		if(!on_button.IsFunction())
			return;
		try {
			vm.Call(on_button, { PyValue(button_id) });
		}
		catch(Exc& e) {
			ReportVmError("on_button", e);
		}
	});
}

void CardGameDocumentHost::InvokePythonCard(const String& card_id)
{
	if(!plugin || !game_running)
		return;
	QueueVmTask([=] {
		PyValue on_click = plugin->GetGameFunction("on_click");
		if(!on_click.IsFunction())
			return;
		try {
			vm.Call(on_click, { PyValue(card_id) });
		}
		catch(Exc& e) {
			ReportVmError("on_click", e);
		}
	});
}

void CardGameDocumentHost::SyncCardCtrl(const String& card_id)
{
	int qs = sprites.Find(card_id);
	if(qs < 0)
		return;

	int qh = card_ctrls.Find(card_id);
	CardSpriteCtrl* ctrl = nullptr;
	if(qh < 0) {
		ctrl = new CardSpriteCtrl;
		ctrl->owner = this;
		ctrl->card_id = card_id;
		card_ctrls.Add(card_id, ctrl);
		AddChild(ctrl);
	}
	else
		ctrl = card_ctrls[qh];

	if(ctrl) {
		ctrl->SetImage(RotateCardImage(sprites[qs].img, sprites[qs].rotation_deg));
		ctrl->SetRect(sprites[qs].rect);
		ctrl->Show();
		ctrl->Refresh();
	}
}

void CardGameDocumentHost::ClearCardCtrls()
{
	for(int i = 0; i < card_ctrls.GetCount(); i++) {
		CardSpriteCtrl* ctrl = card_ctrls[i];
		if(!ctrl)
			continue;
		ctrl->Remove();
		delete ctrl;
	}
	card_ctrls.Clear();
}

void CardGameDocumentHost::DebugInvokeFirstHandCards(int count)
{
	Rect hand = GetZoneRectFromForm(table_form, "hand_self");
	if(hand.IsEmpty() || count <= 0)
		return;

	struct HandCard : Moveable<HandCard> {
		String id;
		int x = 0;
	};
	Vector<HandCard> cards;
	for(int i = 0; i < sprites.GetCount(); i++) {
		const String& id = sprites.GetKey(i);
		if(id.StartsWith("opp_"))
			continue;
		const Sprite& s = sprites[i];
		Point c = s.rect.CenterPoint();
		if(hand.Contains(c)) {
			HandCard& hc = cards.Add();
			hc.id = id;
			hc.x = s.rect.left;
		}
	}
	for(int i = 0; i < cards.GetCount(); i++) {
		int best = i;
		for(int j = i + 1; j < cards.GetCount(); j++) {
			if(cards[j].x < cards[best].x)
				best = j;
		}
		if(best != i)
			Swap(cards[i], cards[best]);
	}
	if(count > cards.GetCount())
		count = cards.GetCount();
	for(int i = 0; i < count; i++)
		InvokePythonCard(cards[i].id);
}

void CardGameDocumentHost::ScheduleSceneSync()
{
	if(scene_sync_pending)
		return;
	scene_sync_pending = true;
	Upp::PostCallback([=] {
		scene_sync_pending = false;
		SyncFormExplorer();
	}, &scene_sync_pending);
}

void CardGameDocumentHost::SyncFormExplorer()
{
	PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow());
	if(!ide || !ide->form_explorer)
		return;

	Vector<FormExplorerEntry> entries;
	Size sz = GetSize();

	for(int i = 0; i < form_items.GetCount(); i++) {
		const FormItem& item = form_items[i];
		FormExplorerEntry& e = entries.Add();
		e.path = "form_items/" + item.id;
		e.type = "Form item";
		e.rect = GetZoneRectFromForm(table_form, item.id);
		e.details = item.user_class + ", " + item.anchor;
	}

	for(int i = 0; i < sprites.GetCount(); i++) {
		const Sprite& s = sprites[i];
		FormExplorerEntry& e = entries.Add();
		e.path = "sprites/" + sprites.GetKey(i);
		e.type = s.animating ? "Sprite (animating)" : "Sprite";
		e.rect = s.rect;
		e.details = GetFileName(s.asset_path);
	}

	for(int i = 0; i < labels.GetCount(); i++) {
		String zone_id = labels.GetKey(i);
		FormExplorerEntry& e = entries.Add();
		e.path = "labels/" + zone_id;
		e.type = "Label";
		e.rect = GetZoneRectFromForm(table_form, zone_id);
		e.details = labels[i];
	}

	for(int i = 0; i < buttons.GetCount(); i++) {
		String zone_id = buttons.GetKey(i);
		FormExplorerEntry& e = entries.Add();
		e.path = "buttons/" + zone_id;
		e.type = buttons[i].enabled ? "Button" : "Button (disabled)";
		e.rect = GetZoneRectFromForm(table_form, zone_id);
		e.details = buttons[i].text;
	}

	for(int i = 0; i < highlights.GetCount(); i++) {
		String zone_id = highlights[i];
		FormExplorerEntry& e = entries.Add();
		e.path = "highlights/" + zone_id;
		e.type = "Highlight";
		e.rect = GetZoneRectFromForm(table_form, zone_id);
		e.details = "active";
	}

	ide->form_explorer->SetScene(sz, entries);
}

void CardGameDocumentHost::Paint(Draw& w)
{
	w.DrawRect(GetSize(), background_color);
}

void CardGameDocumentHost::SyncFormControls()
{
	{
		Mutex::Lock __(rect_cache_mutex);
		rect_cache.Clear();
		for(int i = 0; i < form_items.GetCount(); i++)
			rect_cache.Add(form_items[i].id, GetZoneRectFromForm(table_form, form_items[i].id));
	}

	for(int i = 0; i < labels.GetCount(); i++) {
		if(Ctrl* ctrl = table_form.GetCtrl(labels.GetKey(i))) {
			if(Label* label = dynamic_cast<Label*>(ctrl)) {
				label->SetLabel(labels[i]);
				label->SetInk(White());
				int q = form_items.Find(labels.GetKey(i));
				if(q >= 0) {
					label->SetAlign(form_items[q].anchor == "CENTER_LEFT" ? ALIGN_LEFT :
					               form_items[q].anchor == "CENTER_RIGHT" ? ALIGN_RIGHT :
					               ALIGN_CENTER);
				}
			}
		}
	}

	for(int i = 0; i < buttons.GetCount(); i++) {
		if(Ctrl* ctrl = table_form.GetCtrl(buttons.GetKey(i))) {
			if(Button* button = dynamic_cast<Button*>(ctrl)) {
				String button_id = buttons.GetKey(i);
				button->SetLabel(buttons[i].text);
				button->Enable(buttons[i].enabled);
				button->WhenAction = [=] { InvokePythonButton(button_id); };
			}
		}
	}

	if(Ctrl* ctrl = table_form.GetCtrl("status_line")) {
		if(Label* label = dynamic_cast<Label*>(ctrl)) {
			label->SetLabel(status_text);
			label->SetInk(White());
			label->SetAlign(ALIGN_LEFT);
		}
	}
}

void CardGameDocumentHost::PaintOverlay(Draw& w)
{
	if(debug_overlay) {
		for(int i = 0; i < form_items.GetCount(); i++) {
			const FormItem& item = form_items[i];
			Rect r = GetZoneRectFromForm(table_form, item.id);
			if(r.IsEmpty())
				continue;
			Color c = item.user_class == "HAND" ? Color(120, 180, 255)
			        : item.user_class == "TRICK" ? Color(255, 180, 120)
			        : item.user_class == "BUTTON" ? Color(255, 230, 120)
			        : Color(200, 200, 200);
			w.DrawRect(r.left, r.top, r.GetWidth(), 1, c);
			w.DrawRect(r.left, r.bottom - 1, r.GetWidth(), 1, c);
			w.DrawRect(r.left, r.top, 1, r.GetHeight(), c);
			w.DrawRect(r.right - 1, r.top, 1, r.GetHeight(), c);
			w.DrawText(r.left + 2, max(0, r.top - 14), item.id, StdFont(), c);
		}
	}

	for(int i = 0; i < highlights.GetCount(); i++) {
		Rect r = GetZoneRectFromForm(table_form, highlights[i]);
		if(r.IsEmpty())
			continue;
		Color glow = Color(255, 215, 70);
		w.DrawRect(r.left - 4, r.top - 4, r.GetWidth() + 8, 3, glow);
		w.DrawRect(r.left - 4, r.bottom + 1, r.GetWidth() + 8, 3, glow);
		w.DrawRect(r.left - 4, r.top - 4, 3, r.GetHeight() + 8, glow);
		w.DrawRect(r.right + 1, r.top - 4, 3, r.GetHeight() + 8, glow);
	}

	for(int i = 0; i < sprites.GetCount(); i++) {
	}
}

void CardGameDocumentHost::OverlayLeftDown(Point p, dword flags)
{
}

// --- CardGameProperties ---

void CardGameProperties::Generate(FormObject* pI, int index)
{
	if (!pI) return;

	PropertiesWindow::Generate(pI, index);

	String type = pI->Get("Type");
	String user_class = pI->Get("UserClass");
	if (type.IsEmpty())
		return;

	Property("UserClass", t_("User class:"), "EditField",
		Array<String>() << user_class);
	Property("Anchor", t_("Anchor:"), "DropList",
		Array<String>() << pI->Get("Anchor", "TOP_LEFT")
		                << "TOP_LEFT" << "TOP_CENTER" << "TOP_RIGHT"
		                << "CENTER_LEFT" << "CENTER" << "CENTER_RIGHT"
		                << "BOTTOM_LEFT" << "BOTTOM_CENTER" << "BOTTOM_RIGHT"
		                << "TOP_HSIZE" << "CENTER_HSIZE" << "BOTTOM_HSIZE"
		                << "LEFT_VSIZE" << "CENTER_VSIZE" << "RIGHT_VSIZE"
		                << "SIZE");

	if(ToUpper(user_class) == "SPRITE")
		Property("Image", t_("Asset:"), "EditField", Array<String>() << pI->Get("Image"));

	_Options.HideRow(0);
}

// --- CardGameLayoutEditor ---

CardGameLayoutEditor::CardGameLayoutEditor()
{
	embedded = true;

	Add(main.SizePos());
	main.Add(_CtrlContainer.SizePos());
	main.Add(_Container.SizePos());

	Construct(false);
	_View.SetBool("View.Coloring", true);
	SetViewMode(VIEW_MODE_WIREFRAME);
	
	_TypeList.Clear();
	_TypeList.Add("Label");
	_TypeList.Add("Button");
	
	_View.WhenUpdate = [this] {
		this->UpdateTools();
		this->OpenCardProperties(_View.GetSelected());
	};
	_View.WhenObjectProperties = [this](const Vector<int>& idx) { this->OpenCardProperties(idx); };
	_View.WhenChildSelected = [this](const Vector<int>& idx) { this->OpenCardProperties(idx); };
	_ItemList.WhenChangeRow = [this] {
		if (!_View.IsLayout())
			return;

		if (_ItemList.IsSelected()) {
			Vector<int> sel;
			_View.ClearSelection();
			for (int i = 0; i < _ItemList.GetRowCount(); ++i) {
				if (_ItemList.IsSelected(i)) {
					_View.AddToSelection(i);
					if (!sel.GetCount())
						sel << i;
					else if (sel.GetCount() > 0)
						sel.Clear();
				}
			}
			OpenCardProperties(sel);
		}
	};
	_ItemList.WhenLeftClick = _ItemList.WhenChangeRow;
}

CardGameLayoutEditor::~CardGameLayoutEditor()
{
}

void CardGameLayoutEditor::OpenCardProperties(const Vector<int>& indexes)
{
	if (!_View.IsLayout())
		return;

	String temp = _TempObjectName;
	_TempObjectName.Clear();
	_ItemList.EndEdit(false, false, false);
	int row = _ItemList.GetCurrentRow();
	if (row >= 0 && !temp.IsEmpty())
	{
		_View.GetCurrentLayout()->GetObjects()[row].Set("Variable", temp);
		_ItemList.Set(row, 1, temp);
	}
	_LayoutList.EndEdit();

	if (indexes.GetCount() == 1)
	{
		FormObject* pI = _View.GetObject(indexes[0]);
		if (!pI) return;

		card_properties._Options.EndEdit();
		card_properties.Generate(pI, indexes[0]);
	}

	if (indexes.GetCount() == 0)
	{
		card_properties._Options.EndEdit();
		card_properties._Headers.Clear();
		card_properties._Options.Clear();
	}

	UpdateItemList();
}

bool CardGameLayoutEditor::Load(const String& path_)
{
	path = path_;
	Clear();

	Color bg;
	if(!LoadCardGameFormView(_View, path, bg))
		return false;
	if(!_View.IsLayout() && _View.GetLayoutCount() > 0)
		_View.SelectLayout(0);
	UpdateLayoutList();
	UpdateChildZ();
	_Container.Set(_View, _View.GetPageRect().GetSize());
	SetViewMode(VIEW_MODE_WIREFRAME);
	
	UpdateTools();
	ProjectSaved(true);
	return true; 
}

bool CardGameLayoutEditor::Save()
{
	if(_View.IsLayout()) {
		FormLayout* layout = _View.GetCurrentLayout();
		if(layout && layout->Get("CardGame.Background").IsEmpty())
			layout->Set("CardGame.Background", "40,160,40");

		Array<FormObject>* objs = _View.GetObjects();
		if(objs) {
			for(int i = 0; i < objs->GetCount(); i++) {
				FormObject& obj = (*objs)[i];
				String user_class = NormalizeUserClass(obj.Get("UserClass"), obj.Get("Type"));
				if(!user_class.IsEmpty())
					obj.Set("UserClass", user_class);
				if(obj.Get("Type") == "Button" && obj.Get("Label").IsEmpty())
					obj.Set("Label", obj.Get("Variable"));
			}
		}
	}

	bool ok = _View.SaveAll(path, false);

	if(ok) {
		ProjectSaved(true);
		return true;
	}
	return false;
}

bool CardGameLayoutEditor::SaveAs(const String& path_)
{
	path = path_;
	return Save();
}

void CardGameLayoutEditor::ActivateUI()
{
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		Ptr<PythonIDE> p = ide;
		Ptr<CardGameLayoutEditor> self = this;
		PostCallback([p, self] {
			if(!p || !self)
				return;
			if(p->active_file < 0 || p->active_file >= p->open_files.GetCount())
				return;
			if(p->open_files[p->active_file].editor != self)
				return;

			auto add_left_pane = [&](const String& id, const String& title, Ctrl& ctrl, const Size& hint, int pos) {
				int q = p->plugin_panes.Find(id);
				if(q >= 0) {
					p->plugin_panes[q].Show();
					return;
				}

				DockableCtrl& pane = p->plugin_panes.Add(id);
				pane.Title(title);
				pane.Add(ctrl.SizePos());
				pane.SizeHint(hint);
				p->Register(pane);
				p->DockLeft(pane, pos);
				pane.Show();
			};

			auto add_right_tab = [&](const String& id, const String& title, Ctrl& ctrl) {
				int q = p->plugin_panes.Find(id);
				if(q >= 0) {
					p->plugin_panes[q].Show();
					return;
				}

				DockableCtrl& pane = p->plugin_panes.Add(id);
				pane.Title(title);
				pane.Add(ctrl.SizePos());
				p->Register(pane);
				p->Tabify(*p->var_explorer, pane);
				pane.Show();
			};

			add_left_pane("FormEditorLayouts", "Form Layouts", self->_LayoutList, Size(250, 240), 1);
			add_right_tab("FormEditorItems", "Form Items", self->_ItemList);
			add_right_tab("FormEditorProperties", "Item Properties", self->card_properties);
		});
	}
}

void CardGameLayoutEditor::DeactivateUI()
{
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		auto remove_pane = [&](const String& id) {
			int q = ide->plugin_panes.Find(id);
			if(q < 0)
				return;
			ide->plugin_panes[q].Close();
			ide->plugin_panes[q].Remove();
			ide->plugin_panes.Remove(q);
		};

		_LayoutList.Ctrl::Remove();
		_ItemList.Ctrl::Remove();
		card_properties.Remove();

		remove_pane("FormEditorLayouts");
		remove_pane("FormEditorItems");
		remove_pane("FormEditorProperties");
	}
}

void CardGameLayoutEditor::MainMenu(Bar& bar)
{
	CreateMenuBar(bar);
}

void CardGameLayoutEditor::Toolbar(Bar& bar)
{
	CreateToolBar(bar);
}

// --- CardGamePluginGUI ---

CardGamePluginGUI::CardGamePluginGUI()
{
	gamestate_handler.plugin = this;
	form_handler.plugin = this;
}

CardGamePluginGUI::~CardGamePluginGUI()
{
}

void CardGamePluginGUI::Init(IPluginContext& context_)
{
	CardGamePlugin::Init(context_);
	
	if(IPluginContextGUI* gui = dynamic_cast<IPluginContextGUI*>(&context_)) {
		if(IPluginRegistryGUI* reg = dynamic_cast<IPluginRegistryGUI*>(&context_)) {
			reg->RegisterFileTypeHandler(gamestate_handler);
			reg->RegisterFileTypeHandler(form_handler);
		}
		gui->GetIDE().Log("CardGamePlugin (GUI) initialized.");
	}
}

void CardGamePluginGUI::Shutdown()
{
	CardGamePlugin::Shutdown();
}

IDocumentHost* CardGamePluginGUI::GameStateHandler::CreateDocumentHost()
{
	CardGameDocumentHost* host = new CardGameDocumentHost();
	host->SetPlugin(plugin);
	return host;
}

IDocumentHost* CardGamePluginGUI::FormHandler::CreateDocumentHost()
{
	return new CardGameLayoutEditor();
}

// Registration
REGISTER_PLUGIN(CardGamePluginGUI)
