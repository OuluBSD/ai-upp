#ifndef _Adventure_AdventureBindings_h_
#define _Adventure_AdventureBindings_h_

#include "Adventure.h"
#include <ByteVM/ByteVM.h>

// Don't use NAMESPACE_UPP here - we're declaring classes in the Adventure namespace

class AdventureBindings {
public:
	// Register all bindings with the PyVM
	static void RegisterAll(Upp::PyVM& vm, Adventure::Program& prog);
	
	// Core functions - Room & Camera
	static Upp::PyValue change_room(const Upp::Vector<Upp::PyValue>& args, void* user_data);
	static Upp::PyValue camera_follow(const Upp::Vector<Upp::PyValue>& args, void* user_data);
	static Upp::PyValue camera_at(const Upp::Vector<Upp::PyValue>& args, void* user_data);
	static Upp::PyValue camera_pan_to(const Upp::Vector<Upp::PyValue>& args, void* user_data);
	static Upp::PyValue put_at(const Upp::Vector<Upp::PyValue>& args, void* user_data);
	
	// UI functions - Dialog & Text
	static Upp::PyValue say_line(const Upp::Vector<Upp::PyValue>& args, void* user_data);
	static Upp::PyValue print_line(const Upp::Vector<Upp::PyValue>& args, void* user_data);
	
	// Game logic functions
	static Upp::PyValue break_time(const Upp::Vector<Upp::PyValue>& args, void* user_data);
	static Upp::PyValue cutscene(const Upp::Vector<Upp::PyValue>& args, void* user_data);
	static Upp::PyValue pickup_obj(const Upp::Vector<Upp::PyValue>& args, void* user_data);
	
	// Drawing functions
	static Upp::PyValue set_trans_col(const Upp::Vector<Upp::PyValue>& args, void* user_data);
	static Upp::PyValue map_draw(const Upp::Vector<Upp::PyValue>& args, void* user_data);
};

#endif
