#include "ScriptCommon.h"

NAMESPACE_UPP

// ---- headless hearts_view stub bindings ----

static PyValue hv_log(const Vector<PyValue>& args, void*)
{
	if(args.GetCount() >= 1)
		Cout() << args[0].ToString() << "\n";
	return PyValue::None();
}

static PyValue hv_clear_sprites(const Vector<PyValue>& args, void*)
{
	return PyValue::None();
}

static PyValue hv_set_label(const Vector<PyValue>& args, void*)
{
	return PyValue::None();
}

static PyValue hv_set_button(const Vector<PyValue>& args, void*)
{
	return PyValue::None();
}

static PyValue hv_set_status(const Vector<PyValue>& args, void*)
{
	if(args.GetCount() >= 1)
		Cout() << args[0].ToString() << "\n";
	return PyValue::None();
}

static PyValue hv_set_card(const Vector<PyValue>& args, void*)
{
	// set_card(card_id, asset_path, x, y)
	return PyValue::None();
}

static PyValue hv_move_card(const Vector<PyValue>& args, void*)
{
	// move_card(card_id, zone_id, offset, animated)
	return PyValue::None();
}

static PyValue hv_get_zone_rect(const Vector<PyValue>& args, void*)
{
	// Return a dummy rect dict so layout code doesn't crash
	PyValue d = PyValue::Dict();
	d.SetItem(PyValue("x"), PyValue(0.0));
	d.SetItem(PyValue("y"), PyValue(0.0));
	d.SetItem(PyValue("w"), PyValue(552.0));
	d.SetItem(PyValue("h"), PyValue(96.0));
	return d;
}

// ---- helpers ----

// Recursively scan dir for .py files and pre-load them as modules.
// base_dir is the search root; rel_prefix is the dotted prefix (e.g. "hearts").
static void LoadPyModulesFromDir(PyVM& vm, const String& base_dir, const String& pkg_dir, const String& mod_prefix)
{
	// Only load .py files when inside a package (non-empty prefix), not top-level game dir
	if(!mod_prefix.IsEmpty()) {
		FindFile ff(AppendFileName(pkg_dir, "*.py"));
		while(ff) {
			String name = ff.GetName();
			if(name != "__init__.py") {
				String stem = GetFileTitle(name);
				String mod_name = mod_prefix + "." + stem;
				String src = LoadFile(AppendFileName(pkg_dir, name));
				if(!src.IsVoid())
					vm.LoadModule(mod_name, src, AppendFileName(pkg_dir, name));
			}
			ff.Next();
		}
	}
	// Also load Python subdirectories (with or without __init__.py, skip __pycache__)
	FindFile ffd(AppendFileName(pkg_dir, "*"));
	while(ffd) {
		if(ffd.IsDirectory() && ffd.GetName() != "." && ffd.GetName() != ".." && ffd.GetName() != "__pycache__") {
			String subdir = AppendFileName(pkg_dir, ffd.GetName());
			String sub_prefix = mod_prefix.IsEmpty() ? ffd.GetName() : (mod_prefix + "." + ffd.GetName());
			String init = AppendFileName(subdir, "__init__.py");
			if(FileExists(init)) {
				String src = LoadFile(init);
				if(!src.IsVoid())
					vm.LoadModule(sub_prefix, src, init);
			}
			LoadPyModulesFromDir(vm, base_dir, subdir, sub_prefix);
		}
		ffd.Next();
	}
}

// ---- CardGamePlugin ----

CardGamePlugin::CardGamePlugin()
{
}

CardGamePlugin::~CardGamePlugin()
{
}

void CardGamePlugin::Init(IPluginContext& ctx)
{
	context = &ctx;
	context->RegisterCustomExecuteProvider(*this);
	context->RegisterPythonBindingProvider(*this);
}

void CardGamePlugin::Shutdown()
{
}

bool CardGamePlugin::CanExecute(const String& path)
{
	return ToLower(GetFileExt(path)) == ".gamestate";
}

void CardGamePlugin::Execute(const String& path)
{
	PyVM* vm = context ? context->GetVM() : nullptr;
	if(!vm) { LOG("CardGamePlugin: no VM!"); return; }

	// Parse .gamestate JSON
	String json = LoadFile(path);
	if(json.IsVoid()) {
		LOG("CardGamePlugin: cannot read " << path);
		return;
	}
	Value gs = ParseJSON(json);
	if(gs.IsVoid()) {
		LOG("CardGamePlugin: invalid JSON in " << path);
		return;
	}

	String game_dir = GetFileDirectory(path);
	String entry_script   = gs["entry_script"];
	String entry_function = gs["entry_function"];
	if(entry_script.IsEmpty()) {
		LOG("CardGamePlugin: no entry_script in " << path);
		return;
	}

	// Change working directory to game folder
	SetCurrentDirectory(game_dir);

	// Add game_dir to sys.path so relative Python imports resolve
	PyValue sys = vm->GetGlobals().GetItem(PyValue("sys"));
	if(sys.GetType() == PY_DICT) {
		PyValue path_list = sys.GetItem(PyValue("path"));
		if(path_list.GetType() == PY_LIST) {
			// Prepend game_dir
			PyValue new_list = PyValue::List();
			new_list.Add(PyValue(game_dir));
			for(int i = 0; i < path_list.GetCount(); i++)
				new_list.Add(path_list.GetItem(i));
			sys.SetItem(PyValue("path"), new_list);
		}
	}

	// Register hearts_view stub module (headless)
	SyncBindings(*vm);

	// Pre-load all Python packages/modules from game_dir into sys.modules
	LoadPyModulesFromDir(*vm, game_dir, game_dir, "");

	// Load and run entry script
	String entry_path = AppendFileName(game_dir, entry_script);
	String entry_src = LoadFile(entry_path);
	if(entry_src.IsVoid()) {
		LOG("CardGamePlugin: cannot read entry script " << entry_path);
		return;
	}

	try {
		Tokenizer tk;
		tk.SkipComments();
		tk.SkipPythonComments();
		if(!tk.Process(entry_src, entry_path)) return;
		tk.NewlineToEndStatement();
		tk.CombineTokens();

		PyCompiler compiler(tk.GetTokens(), entry_path);
		Vector<PyIR> ir;
		compiler.Compile(ir);

		vm->SetIR(ir);
		vm->Run();
	} catch(Exc& e) {
		LOG("CardGamePlugin: error in entry script: " << e);
		return;
	}

	// Call entry function if specified
	if(!entry_function.IsEmpty()) {
		PyValue fn = vm->GetGlobals().GetItem(PyValue(entry_function));
		LOG("CardGamePlugin: looking for entry_function='" << entry_function << "', found=" << (!fn.IsNone() ? "yes" : "no"));
		if(!fn.IsNone()) {
			try {
				vm->Call(fn, {});
				LOG("CardGamePlugin: entry function completed");
			} catch(Exc& e) {
				LOG("CardGamePlugin: error calling " << entry_function << "(): " << e);
				throw;
			}
		} else {
			LOG("CardGamePlugin: entry function '" << entry_function << "' not found");
		}
	}
}

// ---- GUI-backed hearts_view bindings (user_data = IHeartsView*) ----

static PyValue hv_gui_log(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() >= 1)
		((IHeartsView*)ud)->Log(args[0].ToString());
	return PyValue::None();
}

static PyValue hv_gui_clear_sprites(const Vector<PyValue>&, void* ud)
{
	((IHeartsView*)ud)->ClearSprites();
	return PyValue::None();
}

static PyValue hv_gui_set_label(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() >= 2)
		((IHeartsView*)ud)->SetLabel(args[0].ToString(), args[1].ToString());
	return PyValue::None();
}

static PyValue hv_gui_set_button(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() >= 3)
		((IHeartsView*)ud)->SetButton(args[0].ToString(), args[1].ToString(), args[2].IsTrue());
	return PyValue::None();
}

static PyValue hv_gui_set_status(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() >= 1)
		((IHeartsView*)ud)->SetStatus(args[0].ToString());
	return PyValue::None();
}

static PyValue hv_gui_set_card(const Vector<PyValue>& args, void* ud)
{
	// set_card(card_id, asset_path, x, y)
	if(args.GetCount() >= 4)
		((IHeartsView*)ud)->SetCard(args[0].ToString(), args[1].ToString(),
		                            (int)args[2].AsInt64(), (int)args[3].AsInt64());
	return PyValue::None();
}

static PyValue hv_gui_move_card(const Vector<PyValue>& args, void* ud)
{
	// move_card(card_id, zone_id, offset, animated)
	if(args.GetCount() >= 4)
		((IHeartsView*)ud)->MoveCardToZone(args[0].ToString(), args[1].ToString(),
		                                   (int)args[2].AsInt64(), args[3].IsTrue());
	return PyValue::None();
}

static PyValue hv_gui_get_zone_rect(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() < 1) return PyValue::None();
	Value r = ((IHeartsView*)ud)->GetZoneRect(args[0].ToString());
	PyValue d = PyValue::Dict();
	d.SetItem(PyValue("x"), PyValue((int64)(int)r["x"]));
	d.SetItem(PyValue("y"), PyValue((int64)(int)r["y"]));
	d.SetItem(PyValue("w"), PyValue((int64)(int)r["w"]));
	d.SetItem(PyValue("h"), PyValue((int64)(int)r["h"]));
	return d;
}

void CardGamePlugin::SyncBindings(PyVM& vm)
{
	if(view) {
		// GUI-backed hearts_view module — real calls into IHeartsView
		PY_MODULE(hearts_view, vm)
		PY_MODULE_FUNC(log,           hv_gui_log,           view)
		PY_MODULE_FUNC(clear_sprites, hv_gui_clear_sprites, view)
		PY_MODULE_FUNC(set_label,     hv_gui_set_label,     view)
		PY_MODULE_FUNC(set_button,    hv_gui_set_button,    view)
		PY_MODULE_FUNC(set_status,    hv_gui_set_status,    view)
		PY_MODULE_FUNC(set_card,      hv_gui_set_card,      view)
		PY_MODULE_FUNC(move_card,     hv_gui_move_card,     view)
		PY_MODULE_FUNC(get_zone_rect, hv_gui_get_zone_rect, view)
	} else {
		// Headless stubs — used by ScriptCLI and tests
		PY_MODULE(hearts_view, vm)
		PY_MODULE_FUNC(log,           hv_log,           nullptr)
		PY_MODULE_FUNC(clear_sprites, hv_clear_sprites, nullptr)
		PY_MODULE_FUNC(set_label,     hv_set_label,     nullptr)
		PY_MODULE_FUNC(set_button,    hv_set_button,    nullptr)
		PY_MODULE_FUNC(set_status,    hv_set_status,    nullptr)
		PY_MODULE_FUNC(set_card,      hv_set_card,      nullptr)
		PY_MODULE_FUNC(move_card,     hv_move_card,     nullptr)
		PY_MODULE_FUNC(get_zone_rect, hv_get_zone_rect, nullptr)
	}
}

REGISTER_PLUGIN(CardGamePlugin)

END_UPP_NAMESPACE
