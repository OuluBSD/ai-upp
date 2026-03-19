#include "ScriptCommon.h"

NAMESPACE_UPP

// ---- headless cardgame_view stub bindings ----

static PyValue hv_log(const Vector<PyValue>& args, void*)
{
	if(args.GetCount() >= 1)
		Cout() << args[0].ToString() << "\n";
	return PyValue();
}

static PyValue hv_clear_sprites(const Vector<PyValue>& args, void*)
{
	return PyValue();
}

static PyValue hv_begin_sprite_frame(const Vector<PyValue>& args, void*)
{
	return PyValue();
}

static PyValue hv_remove_sprite(const Vector<PyValue>& args, void*)
{
	return PyValue();
}

static PyValue hv_set_label(const Vector<PyValue>& args, void*)
{
	return PyValue();
}

static PyValue hv_set_expected_sprite_count(const Vector<PyValue>& args, void*)
{
	return PyValue();
}

static PyValue hv_set_button(const Vector<PyValue>& args, void*)
{
	return PyValue();
}

static PyValue hv_set_highlight(const Vector<PyValue>& args, void*)
{
	return PyValue();
}

static PyValue hv_set_status(const Vector<PyValue>& args, void*)
{
	if(args.GetCount() >= 1)
		Cout() << args[0].ToString() << "\n";
	return PyValue();
}

static PyValue hv_set_card(const Vector<PyValue>& args, void*)
{
	// set_card(card_id, asset_path, x, y)
	return PyValue();
}

static PyValue hv_move_card(const Vector<PyValue>& args, void*)
{
	// move_card(card_id, zone_id, offset, animated)
	return PyValue();
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

static PyValue hv_set_timeout(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() < 2 || !ud)
		return PyValue();
	PyVM& vm = *(PyVM*)ud;
	String callback_name = args[1].ToString();
	PyValue fn = vm.GetGlobals().GetItem(PyValue(callback_name));
	if(fn.IsFunction())
		vm.Call(fn, {});
	return PyValue();
}

// ---- helpers ----

// Recursively scan dir for .py files and pre-load them as modules.
// base_dir is the search root; rel_prefix is the dotted prefix (e.g. "hearts").
static bool LoadPyModulesFromDir(PyVM& vm, const String& base_dir, const String& pkg_dir, const String& mod_prefix,
                                 String* failed_module = nullptr, String* failed_path = nullptr)
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
				if(!src.IsVoid() && !vm.LoadModule(mod_name, src, AppendFileName(pkg_dir, name))) {
					if(failed_module) *failed_module = mod_name;
					if(failed_path) *failed_path = AppendFileName(pkg_dir, name);
					return false;
				}
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
				if(!src.IsVoid() && !vm.LoadModule(sub_prefix, src, init)) {
					if(failed_module) *failed_module = sub_prefix;
					if(failed_path) *failed_path = init;
					return false;
				}
			}
			if(!LoadPyModulesFromDir(vm, base_dir, subdir, sub_prefix, failed_module, failed_path))
				return false;
		}
		ffd.Next();
	}
	return true;
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
	entry_module_name.Clear();
	bool want_breakpoints = vm->AreBreakpointsEnabled();
	struct BreakpointRestore {
		PyVM* vm;
		bool enabled;
		~BreakpointRestore() { if(vm) vm->EnableBreakpoints(enabled); }
	} breakpoint_restore { vm, want_breakpoints };
	vm->EnableBreakpoints(false);
	SyncBindings(*vm);
	{
		PyValue cg = vm->GetGlobals().GetItem(PyValue("cardgame_view"));
		if(cg.GetType() == PY_DICT) {
			PyValue log_func = cg.GetItem(PyValue("log"));
		}
	}

	// Parse .gamestate JSON
	String json = LoadFile(path);
	if(json.IsVoid()) {
		LOG("CardGamePlugin: cannot read " << path);
		if(view) view->Log("CardGamePlugin: cannot read " + path);
		return;
	}
	Value gs = ParseJSON(json);
	if(gs.IsVoid()) {
		LOG("CardGamePlugin: invalid JSON in " << path);
		if(view) view->Log("CardGamePlugin: invalid JSON in " + path);
		return;
	}

	String game_dir = GetFileDirectory(path);
	String entry_script   = gs["entry_script"];
	String entry_function = gs["entry_function"];
	if(entry_script.IsEmpty()) {
		LOG("CardGamePlugin: no entry_script in " << path);
		if(view) view->Log("CardGamePlugin: no entry_script in " + path);
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

	// Add extra pythonpath entries from metadata (same as OCRPlugin does).
	// Without this, 'import main' and other cross-directory imports fail when
	// the entry script lives in a different folder than the dependency modules.
	Value pythonpath = gs["metadata"]["pythonpath"];
	if(pythonpath.Is<ValueArray>()) {
		const ValueArray& pa = pythonpath;
		PyValue sys2 = vm->GetGlobals().GetItem(PyValue("sys"));
		if(sys2.GetType() == PY_DICT) {
			PyValue path_list2 = sys2.GetItem(PyValue("path"));
			if(path_list2.GetType() == PY_LIST) {
				for(int i = 0; i < pa.GetCount(); i++) {
					String extra = pa[i].ToString();
					if(extra.IsEmpty()) continue;
					if(!IsFullPath(extra))
						extra = AppendFileName(game_dir, extra);
					// Avoid duplicates
					bool found = false;
					for(int j = 0; j < path_list2.GetCount(); j++)
						if(path_list2.GetItem(j).ToString() == extra) { found = true; break; }
					if(!found) path_list2.Add(PyValue(extra));
				}
				sys2.SetItem(PyValue("path"), path_list2);
			}
		}
	}

	// Register cardgame_view stub module (headless)
	SyncBindings(*vm);

	// Pre-load all Python packages/modules from game_dir and pythonpath dirs into sys.modules.
	// game_dir is loaded first (highest priority); pythonpath dirs are loaded after.
	String failed_module, failed_path;
	if(!LoadPyModulesFromDir(*vm, game_dir, game_dir, "", &failed_module, &failed_path)) {
		String msg = "CardGamePlugin: failed to preload module " + failed_module;
		if(!failed_path.IsEmpty())
			msg << " from " << failed_path;
		LOG(msg);
		if(view) view->Log(msg);
		return;
	}
	if(pythonpath.Is<ValueArray>()) {
		const ValueArray& pa = pythonpath;
		for(int i = 0; i < pa.GetCount(); i++) {
			String extra = pa[i].ToString();
			if(extra.IsEmpty()) continue;
			if(!IsFullPath(extra))
				extra = AppendFileName(game_dir, extra);
			if(!DirectoryExists(extra)) continue;
			String fm, fp;
			// Non-fatal: pythonpath dirs may be optional
			LoadPyModulesFromDir(*vm, extra, extra, "", &fm, &fp);
		}
	}

	// Load and run entry script
	String entry_path = AppendFileName(game_dir, entry_script);
	String entry_src = LoadFile(entry_path);
	if(entry_src.IsVoid()) {
		LOG("CardGamePlugin: cannot read entry script " << entry_path);
		if(view) view->Log("CardGamePlugin: cannot read entry script " + entry_path);
		return;
	}

	entry_module_name = GetFileTitle(entry_script);
	if(entry_module_name.IsEmpty())
		entry_module_name = "__game_main__";

	try {
		Tokenizer tk;
		tk.SkipComments();
		tk.SkipPythonComments();
		if(!tk.Process(entry_src, entry_path)) {
			String msg = "CardGamePlugin: tokenizer failed for " + entry_path;
			LOG(msg);
			if(view) view->Log(msg);
			return;
		}
		tk.NewlineToEndStatement();
		tk.CombineTokens();

		PyCompiler compiler(tk.GetTokens(), entry_path);
		Vector<PyIR> ir;
		compiler.Compile(ir);
	}
	catch(Exc& e) {
		String msg = "CardGamePlugin: compile error in " + entry_path + ": " + String(e);
		LOG(msg);
		if(view) view->Log(msg);
		return;
	}

        if(!vm->LoadModule(entry_module_name, entry_src, entry_path)) {
		LOG("CardGamePlugin: failed to load entry module " << entry_module_name);
		if(view) view->Log("CardGamePlugin: failed to load entry module " + entry_module_name);
		return;
	}

	PyValue entry_sys = vm->GetGlobals().GetItem(PyValue("sys"));
	PyValue mod_dict;
	if(entry_sys.GetType() == PY_DICT) {
		PyValue modules = entry_sys.GetItem(PyValue("modules"));
		if(modules.GetType() == PY_DICT)
			mod_dict = modules.GetItem(PyValue(entry_module_name));
	}
	if(mod_dict.GetType() == PY_DICT) {
		const VectorMap<PyValue, PyValue>& items = mod_dict.GetDict();
		VectorMap<PyValue, PyValue>& globals = vm->GetGlobals().GetDictRW();
		for(int i = 0; i < items.GetCount(); i++)
			globals.GetAdd(items.GetKey(i)) = items[i];
	}

	// Call entry function if specified
	if(!entry_function.IsEmpty()) {
		vm->EnableBreakpoints(want_breakpoints);
		PyValue fn = GetGameFunction(entry_function);
		LOG("CardGamePlugin: looking for entry_function='" << entry_function << "', found=" << (!fn.IsNone() ? "yes" : "no"));
		if(!fn.IsNone()) {
			try {
				vm->Call(fn, {});
				LOG("CardGamePlugin: entry function completed");
			} catch(Exc& e) {
				LOG("CardGamePlugin: error calling " << entry_function << "(): " << e);
				if(view) view->Log("CardGamePlugin: error calling " + entry_function + "(): " + String(e));
				throw;
			}
		} else {
			LOG("CardGamePlugin: entry function '" << entry_function << "' not found");
			if(view) view->Log("CardGamePlugin: entry function '" + entry_function + "' not found");
		}
	}
}

PyValue CardGamePlugin::GetGameFunction(const String& name) const
{
	PyVM* vm = context ? context->GetVM() : nullptr;
	if(!vm)
		return PyValue();

	if(!entry_module_name.IsEmpty()) {
		PyValue sys = vm->GetGlobals().GetItem(PyValue("sys"));
		if(sys.GetType() == PY_DICT) {
			PyValue modules = sys.GetItem(PyValue("modules"));
			if(modules.GetType() == PY_DICT) {
				PyValue mod = modules.GetItem(PyValue(entry_module_name));
				if(mod.GetType() == PY_DICT) {
					PyValue fn = mod.GetItem(PyValue(name));
					if(fn.IsFunction())
						return fn;
				}
			}
		}
	}

	PyValue fn = vm->GetGlobals().GetItem(PyValue(name));
	return fn;
}

// ---- GUI-backed cardgame_view bindings (user_data = IHeartsView*) ----

static PyValue hv_gui_log(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() >= 1)
		((IHeartsView*)ud)->Log(args[0].ToString());
	return PyValue();
}

static PyValue hv_gui_clear_sprites(const Vector<PyValue>&, void* ud)
{
	((IHeartsView*)ud)->ClearSprites();
	return PyValue();
}

static PyValue hv_gui_begin_sprite_frame(const Vector<PyValue>&, void* ud)
{
	((IHeartsView*)ud)->BeginSpriteFrame();
	return PyValue();
}

static PyValue hv_gui_remove_sprite(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() >= 1)
		((IHeartsView*)ud)->RemoveSprite(args[0].ToString());
	return PyValue();
}

static PyValue hv_gui_set_label(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() >= 2)
		((IHeartsView*)ud)->SetLabel(args[0].ToString(), args[1].ToString());
	return PyValue();
}

static PyValue hv_gui_set_expected_sprite_count(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() >= 2)
		((IHeartsView*)ud)->SetExpectedSpriteCount(args[0].ToString(), (int)args[1].AsInt64());
	return PyValue();
}

static PyValue hv_gui_set_button(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() >= 3)
		((IHeartsView*)ud)->SetButton(args[0].ToString(), args[1].ToString(), args[2].IsTrue());
	return PyValue();
}

static PyValue hv_gui_set_highlight(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() >= 2)
		((IHeartsView*)ud)->SetHighlight(args[0].ToString(), args[1].IsTrue());
	return PyValue();
}

static PyValue hv_gui_set_status(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() >= 1)
		((IHeartsView*)ud)->SetStatus(args[0].ToString());
	return PyValue();
}

static PyValue hv_gui_set_card(const Vector<PyValue>& args, void* ud)
{
        // set_card(card_id, asset_path, x, y, rotation_deg=0)
        if(args.GetCount() >= 4) {
                ((IHeartsView*)ud)->SetCard(args[0].ToString(), args[1].ToString(),
                                            (int)args[2].AsInt64(), (int)args[3].AsInt64(),
                                            args.GetCount() >= 5 ? (int)args[4].AsInt64() : 0);
        }
        return PyValue();
}

static PyValue hv_gui_move_card(const Vector<PyValue>& args, void* ud)
{
	// move_card(card_id, zone_id, offset, animated)
	if(args.GetCount() >= 4)
		((IHeartsView*)ud)->MoveCardToZone(args[0].ToString(), args[1].ToString(),
		                                   (int)args[2].AsInt64(), args[3].IsTrue());
	return PyValue();
}

static PyValue hv_gui_get_zone_rect(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() < 1) return PyValue();
	Value r = ((IHeartsView*)ud)->GetZoneRect(args[0].ToString());
	PyValue d = PyValue::Dict();
	d.SetItem(PyValue("x"), PyValue((int64)(int)r["x"]));
	d.SetItem(PyValue("y"), PyValue((int64)(int)r["y"]));
	d.SetItem(PyValue("w"), PyValue((int64)(int)r["w"]));
	d.SetItem(PyValue("h"), PyValue((int64)(int)r["h"]));
	return d;
}

static PyValue hv_gui_set_timeout(const Vector<PyValue>& args, void* ud)
{
	if(args.GetCount() >= 2)
		((IHeartsView*)ud)->SetTimeout((int)args[0].AsInt64(), args[1].ToString());
	return PyValue();
}

void CardGamePlugin::SyncBindings(PyVM& vm)
{
	PyValue sys = vm.GetGlobals().GetItem(PyValue("sys"));
	PyValue modules;
	if(sys.GetType() == PY_DICT)
		modules = sys.GetItem(PyValue("modules"));

	if(view) {
		// GUI-backed cardgame_view module — real calls into IHeartsView
		PY_MODULE(cardgame_view, vm)
		PY_MODULE_FUNC(log,           hv_gui_log,           view)
		PY_MODULE_FUNC(clear_sprites, hv_gui_clear_sprites, view)
		PY_MODULE_FUNC(begin_sprite_frame, hv_gui_begin_sprite_frame, view)
		PY_MODULE_FUNC(remove_sprite, hv_gui_remove_sprite, view)
		PY_MODULE_FUNC(set_label,     hv_gui_set_label,     view)
		PY_MODULE_FUNC(set_expected_sprite_count, hv_gui_set_expected_sprite_count, view)
		PY_MODULE_FUNC(set_button,    hv_gui_set_button,    view)
		PY_MODULE_FUNC(set_highlight, hv_gui_set_highlight, view)
		PY_MODULE_FUNC(set_status,    hv_gui_set_status,    view)
		PY_MODULE_FUNC(set_card,      hv_gui_set_card,      view)
		PY_MODULE_FUNC(move_card,     hv_gui_move_card,     view)
		PY_MODULE_FUNC(get_zone_rect, hv_gui_get_zone_rect, view)
		PY_MODULE_FUNC(set_timeout,   hv_gui_set_timeout,   view)
		if(modules.GetType() == PY_DICT) {
			modules.SetItem(PyValue("cardgame_view"), cardgame_view_obj);
			modules.SetItem(PyValue("hearts_view"), cardgame_view_obj);
		}
	} else {
		// Headless stubs — used by ScriptCLI and tests
		PY_MODULE(cardgame_view, vm)
		PY_MODULE_FUNC(log,           hv_log,           nullptr)
		PY_MODULE_FUNC(clear_sprites, hv_clear_sprites, nullptr)
		PY_MODULE_FUNC(begin_sprite_frame, hv_begin_sprite_frame, nullptr)
		PY_MODULE_FUNC(remove_sprite, hv_remove_sprite, nullptr)
		PY_MODULE_FUNC(set_label,     hv_set_label,     nullptr)
		PY_MODULE_FUNC(set_expected_sprite_count, hv_set_expected_sprite_count, nullptr)
		PY_MODULE_FUNC(set_button,    hv_set_button,    nullptr)
		PY_MODULE_FUNC(set_highlight, hv_set_highlight, nullptr)
		PY_MODULE_FUNC(set_status,    hv_set_status,    nullptr)
		PY_MODULE_FUNC(set_card,      hv_set_card,      nullptr)
		PY_MODULE_FUNC(move_card,     hv_move_card,     nullptr)
		PY_MODULE_FUNC(get_zone_rect, hv_get_zone_rect, nullptr)
		PY_MODULE_FUNC(set_timeout,   hv_set_timeout,   &vm)
		if(modules.GetType() == PY_DICT) {
			modules.SetItem(PyValue("cardgame_view"), cardgame_view_obj);
			modules.SetItem(PyValue("hearts_view"), cardgame_view_obj);
		}
	}
}

REGISTER_PLUGIN(CardGamePlugin)

END_UPP_NAMESPACE
