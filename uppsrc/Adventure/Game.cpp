#include "Adventure.h"
#include "AdventureBindings.h"

namespace Adventure {




/*void Program::RealizeGame() {
	auto& global = ctx.global;
	
	if (game.IsVoid())
		game = global.Get("game", EscValue());
}*/

bool Program::ReadGame() {
	LOG("ReadGame: Starting to read game state from Python module");
	
	// Get the loaded Python module from sys.modules
	PyValue sys = Program::GetProp(vm.GetGlobals(), "sys");
	PyValue sys_modules = Program::GetProp(sys, "modules");
	PyValue demo_module;
	if (sys_modules.GetType() == PY_DICT) {
		demo_module = Program::GetProp(sys_modules, "demo");
	}
	
	if (demo_module.GetType() != PY_DICT) {
		LOG("ReadGame: demo module not found in sys.modules, type=" << sys_modules.GetType());
		return false;
	}
	
	PyValue py_globals = demo_module;  // Module IS the globals dict

	// Get rooms and verbs from Python module globals
	PyValue rooms = Program::GetProp(py_globals, "rooms");
	LOG("ReadGame: Got rooms, type=" << rooms.GetType() << " count=" << rooms.GetArray().GetCount());

	if (rooms.GetType() != PY_LIST || rooms.GetArray().IsEmpty()) {
		LOG("Program::ReadGame: error: could not find rooms");
		return false;
	}

	PyValue verbs = Program::GetProp(py_globals, "verbs");
	LOG("ReadGame: Got verbs, type=" << verbs.GetType() << " count=" << verbs.GetArray().GetCount());
	if (verbs.GetType() != PY_LIST) {
		LOG("No verbs in game");
		return false;
	}

	const Vector<PyValue>& verb_arr = verbs.GetArray();
	for(int i = 0; i < verb_arr.GetCount(); i++) {
		const PyValue& verb = verb_arr[i];
		if (verb.GetType() != PY_DICT) {
			LOG("Invalid verb at index " << i);
			return false;
		}
		const VectorMap<PyValue, PyValue>& verb_dict = verb.GetDict();
		if(verb_dict.Find(PyValue(WString("name"))) < 0 || verb_dict.Find(PyValue(WString("text"))) < 0) {
			LOG("Invalid verb - missing name or text");
			return false;
		}
		String name = verb_dict.Get(PyValue(WString("name"))).GetStr().ToString();
		if (name == "use")		V_USE = verb;
		if (name == "give")		V_GIVE = verb;
		if (name == "push")		V_PUSH = verb;
		if (name == "pull")		V_PULL = verb;
		if (name == "walkto")	V_WALKTO = verb;
		if (name == "pickup")	V_PICKUP = verb;
		if (name == "lookat")	V_LOOKAT = verb;
		if (name == "open")		V_OPEN = verb;
		if (name == "close")	V_CLOSE = verb;
		if (name == "talkto")	V_TALKTO = verb;

		if (verb_idx.Find(name) >= 0) {
			LOG("Verb already defined: " << name);
			return false;
		}
		verb_idx.Add(name);
	}
	V_COUNT = verb_arr.GetCount();
	if (V_COUNT == 0)  {
		LOG("No verbs");
		return false;
	}

	PyValue def_verb_val = Program::GetProp(py_globals, "verb_default");
	int verb_default_idx = Program::PyInt(def_verb_val);
	if (verb_default_idx < 0 || verb_default_idx >= verb_idx.GetCount()) {
		LOG("Invalid default inventory index");
		return false;
	}
	verb_curr = verbs.GetArray()[verb_default_idx];
	
	
	//LOG(game.ToString());
	return true;
}

// initialise all the rooms (e.g. add in parent links)
bool Program::InitGame() {
	LOG("InitGame: Starting initialization");
	
	// Get the loaded Python module from sys.modules
	PyValue sys = Program::GetProp(vm.GetGlobals(), "sys");
	PyValue sys_modules = Program::GetProp(sys, "modules");
	PyValue demo_module;
	if (sys_modules.GetType() == PY_DICT) {
		demo_module = Program::GetProp(sys_modules, "demo");
	}
	
	if (demo_module.GetType() != PY_DICT) {
		LOG("InitGame: demo module not found in sys.modules, type=" << sys_modules.GetType());
		return false;
	}
	
	PyValue py_globals = demo_module;  // Module IS the globals dict

	try {
		// Call Python main() and startup_script() if they exist
		PyValue main_func = Program::GetProp(py_globals, "main");
		LOG("InitGame: main_func type=" << main_func.GetType() << " IsFunction=" << main_func.IsFunction());
		if (main_func.GetType() != PY_NONE && main_func.IsFunction()) {
			LOG("InitGame: Calling Python main()");
			try {
				py_vm.Call(main_func, Vector<PyValue>());
				LOG("InitGame: Python main() completed");
			} catch(Exc& e) {
				LOG("InitGame: Python main() exception: " << e);
			}
		}
		
		PyValue startup_func = Program::GetProp(py_globals, "startup_script");
		LOG("InitGame: startup_func type=" << startup_func.GetType() << " IsFunction=" << startup_func.IsFunction());
		if (startup_func.GetType() != PY_NONE && startup_func.IsFunction()) {
			LOG("InitGame: Calling Python startup_script()");
			try {
				py_vm.Call(startup_func, Vector<PyValue>());
				LOG("InitGame: Python startup_script() completed");
			} catch(Exc& e) {
				LOG("InitGame: Python startup_script() exception: " << e);
			}
		}
		//RealizeGame();
	}
	catch(CParser::Error e) {
		LOG("Program::InitGame: error: " << e << "\n");
		return false;
	}
	catch(Exc& e) {
		LOG("Program::InitGame: unhandled exception: " << e << "\n");
		return false;
	}
	
	
	/*for (room in all(rooms)) {
		ExplodeData(room);
		
		room.map_w = #room.map > 2 ? room.map[3] - room.map[1] + 1 : 16;
		room.map_h = #room.map > 2 ? room.map[4] - room.map[2] + 1 : 8;
		// auto-depth (or defaults)
		room.autodepth_pos = room.autodepth_pos || {9, 50};
		room.autodepth_scale = room.autodepth_scale || {0.25, 1};
		
		// init objects (in room)
		for (obj in all(room.objects)) {
			ExplodeData(obj);
			obj.in_room, obj.h = room, obj.h || 0;
			if (obj.init)
				obj.init(obj);
		}
	}
	// init actors with defaults
	for (ka, actor in pairs(actors)) {
		ExplodeData(actor);
		actor.moving = 2 ;	// 0=stopped, 1=walking, 2=arrived
		actor.tmr = 1;      // internal timer for (managing animation
		actor.talk_tmr = 1;
		actor.anim_pos = 1; // used to track anim pos
		actor.inventory = {
			// obj_switch_player,
			// obj_switch_tent
		};
		actor.inv_pos = 0;  // pointer to the row to start displaying from
	}*/
	
	
	return true;
}


}
