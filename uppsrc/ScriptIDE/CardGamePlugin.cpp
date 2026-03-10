#include "ScriptIDE.h"
#include "CardGamePlugin.h"

NAMESPACE_UPP

// --- CardGameDocumentHost ---

CardGameDocumentHost::CardGameDocumentHost()
{
	game_log.SetQTF("Welcome to the Game!&Ready to play.");
}

CardGameDocumentHost::~CardGameDocumentHost()
{
}

bool CardGameDocumentHost::Load(const String& path_)
{
	path = path_;
	Refresh();
	return true;
}

void CardGameDocumentHost::ActivateUI()
{
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		ide->context_pane_right.Title("Game Log");
		ide->context_pane_right.Add(game_log.SizePos());
		ide->context_pane_right.Show();
	}
}

void CardGameDocumentHost::DeactivateUI()
{
	if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
		game_log.Remove();
		ide->context_pane_right.Hide();
	}
}

void CardGameDocumentHost::MainMenu(Bar& bar)
{
	bar.Sub("Game", [=](Bar& b) {
		b.Add("Restart Match", [=] { Todo("Restart"); });
		b.Add("Concede", [=] { Todo("Concede"); });
	});
}

void CardGameDocumentHost::Toolbar(Bar& bar)
{
	bar.Add("Play", CtrlImg::right_arrow(), [=] { Todo("Play Turn"); });
}

void CardGameDocumentHost::SetLayout(const String& path)
{
	form_path = path;
	sprites.Clear();
	
	Value v = ParseJSON(LoadFile(path));
	if(v.IsVoid()) return;
	
	if(v.IsMap()) {
		Value bg = v["background_color"];
		if(bg.IsMap()) {
			background_color = Color(bg["r"], bg["g"], bg["b"]);
		}
		
		Value vzones = v["zones"];
		if(vzones.IsArray()) {
			for(int i = 0; i < vzones.GetCount(); i++) {
				Value vz = vzones[i];
				// TODO: Store zones for alignment logic
			}
		}
	}
	
	Refresh();
}

void CardGameDocumentHost::ClearSprites()
{
	sprites.Clear();
	Refresh();
}

void CardGameDocumentHost::SetSprite(const String& id, const String& asset_path, int x, int y)
{
	Sprite& s = sprites.GetAdd(id);
	s.img = StreamRaster::LoadFileAny(asset_path);
	s.rect = RectC(x, y, s.img.GetWidth(), s.img.GetHeight());
	Refresh();
}

void CardGameDocumentHost::MoveSprite(const String& id, int x, int y)
{
	int q = sprites.Find(id);
	if(q >= 0) {
		sprites[q].rect = RectC(x, y, sprites[q].rect.GetWidth(), sprites[q].rect.GetHeight());
		Refresh();
	}
}

void CardGameDocumentHost::Paint(Draw& w)
{
	Size sz = GetSize();
	w.DrawRect(sz, background_color);
	
	for(int i = 0; i < sprites.GetCount(); i++) {
		const Sprite& s = sprites[i];
		if(!s.img.IsEmpty())
			w.DrawImage(s.rect.left, s.rect.top, s.img);
	}
}

void CardGameDocumentHost::LeftDown(Point p, dword flags)
{
	for(int i = sprites.GetCount() - 1; i >= 0; i--) {
		if(sprites[i].rect.Contains(p)) {
			String id = sprites.GetKey(i);
			if(PythonIDE* ide = dynamic_cast<PythonIDE*>(Ctrl::GetTopWindow())) {
				// Call Python on_click(id)
				PyValue on_click = ide->vm.GetGlobals().Get("on_click", PyValue());
				if(!on_click.IsVoid())
					ide->vm.Call(on_click, {PyValue(id)});
			}
			break;
		}
	}
}

// --- CardGamePlugin ---

CardGamePlugin::CardGamePlugin()
{
}

CardGamePlugin::~CardGamePlugin()
{
}

void CardGamePlugin::Init(IPluginContext& ctx)
{
	context = &ctx;
	context->GetIDE().plugin_manager->RegisterFileTypeHandler(*this);
	context->GetIDE().plugin_manager->RegisterCustomExecuteProvider(*this);
	context->GetIDE().plugin_manager->RegisterPythonBindingProvider(*this);
	
	context->GetIDE().Log("CardGamePlugin initialized.");
}

void CardGamePlugin::Shutdown()
{
}

IDocumentHost* CardGamePlugin::CreateDocumentHost()
{
	return new CardGameDocumentHost();
}

bool CardGamePlugin::CanExecute(const String& path)
{
	return ToLower(GetFileExt(path)) == ".gamestate";
}

void CardGamePlugin::Execute(const String& path)
{
	if(!context) return;
	
	PythonIDE& ide = context->GetIDE();
	ide.Log("Launching Game State: " + path);
	
	// 1. Parse .gamestate (JSON)
	Value g = ParseJSON(LoadFile(path));
	if(g.IsVoid()) {
		ide.Error("Failed to parse .gamestate");
		return;
	}
	
	String script = g["entry_script"];
	String func = g["entry_function"];
	String layout = g["layout"];
	
	// 2. Open/Ensure document host is active
	ide.LoadFile(path);
	CardGameDocumentHost* view = nullptr;
	if(ide.active_file >= 0) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~ide.open_files[ide.active_file].editor))
			view = dynamic_cast<CardGameDocumentHost*>(h);
	}
	
	if(!view) {
		ide.Error("Active tab is not a CardGameDocumentHost");
		return;
	}
	
	// 3. Apply layout
	if(!layout.IsEmpty())
		view->SetLayout(AppendFileName(GetFileDirectory(path), layout));
	
	// 4. Start Python logic
	String script_path = AppendFileName(GetFileDirectory(path), script);
	String code = LoadFile(script_path);
	
	ide.plugin_manager->SyncBindings(ide.vm);
	ide.run_manager.Run(code, script_path);
}

static PyValue SetCard(const Vector<PyValue>& args, void* user_data)
{
	CardGameDocumentHost* view = (CardGameDocumentHost*)user_data;
	if(args.GetCount() >= 4 && view) {
		view->SetSprite(args[0].ToString(), args[1].ToString(), args[2].AsInt(), args[3].AsInt());
	}
	return PyValue();
}

static PyValue ClearSprites(const Vector<PyValue>& args, void* user_data)
{
	CardGameDocumentHost* view = (CardGameDocumentHost*)user_data;
	if(view) {
		view->ClearSprites();
	}
	return PyValue();
}

void CardGamePlugin::SyncBindings(PyVM& vm)
{
	PythonIDE& ide = context->GetIDE();
	CardGameDocumentHost* view = nullptr;
	if(ide.active_file >= 0 && ide.open_files[ide.active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~ide.open_files[ide.active_file].editor))
			view = dynamic_cast<CardGameDocumentHost*>(h);
	}
	
	PyValue hearts_view = PyValue::Dict();
	hearts_view.SetItem("set_card", PyValue::Function("set_card", &SetCard, view));
	hearts_view.SetItem("clear_sprites", PyValue::Function("clear_sprites", &ClearSprites, view));
	
	vm.GetGlobals().GetAdd("hearts_view") = hearts_view;
}

REGISTER_PLUGIN(CardGamePlugin)

END_UPP_NAMESPACE
