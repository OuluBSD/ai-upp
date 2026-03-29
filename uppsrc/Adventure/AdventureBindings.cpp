#include "Adventure.h"
#include "AdventureBindings.h"

// Don't use NAMESPACE_UPP here because Adventure is a separate namespace
// using namespace Upp;

// Helper function to get Program instance from user_data
static Adventure::Program* GetProgram(void* user_data)
{
	return (Adventure::Program*)user_data;
}

// Helper function to extract EscValue from PyValue (for object references)
static EscValue PyToEscValue(const Upp::PyValue& pv)
{
	if(pv.IsUserData()) {
		// If it's a wrapped EscValue, extract it
		Upp::PyUserData& ud = pv.GetUserData();
		// Check if it's our EscValue wrapper
		if(ud.GetTypeName() == "EscValue") {
			// This would need a custom wrapper class for EscValue
			// For now, return empty EscValue
		}
	}
	if(pv.GetType() == Upp::PY_STR) {
		// String keys like ":room_name" can be converted to EscValue
		return EscValue(pv.GetStr());
	}
	if(pv.IsInt()) {
		return EscValue(pv.AsInt());
	}
	if(pv.IsFloat()) {
		return EscValue(pv.AsDouble());
	}
	return EscValue();
}

// ============================================================================
// Core Functions - Room & Camera
// ============================================================================

Upp::PyValue AdventureBindings::change_room(const Upp::Vector<Upp::PyValue>& args, void* user_data)
{
	Adventure::Program* prog = GetProgram(user_data);
	
	if(args.GetCount() < 1) {
		return Upp::PyValue::None();
	}
	
	// Extract room and fade parameters
	EscValue room = PyToEscValue(args[0]);
	EscValue fade = args.GetCount() > 1 ? PyToEscValue(args[1]) : EscValue();
	
	// Call the C++ implementation
	prog->ChangeRoom(room, fade);
	
	return Upp::PyValue::None();
}

Upp::PyValue AdventureBindings::camera_follow(const Upp::Vector<Upp::PyValue>& args, void* user_data)
{
	Adventure::Program* prog = GetProgram(user_data);
	
	if(args.GetCount() < 1) {
		return Upp::PyValue::None();
	}
	
	EscValue actor = PyToEscValue(args[0]);
	prog->CameraFollow(actor);
	
	return Upp::PyValue::None();
}

Upp::PyValue AdventureBindings::camera_at(const Upp::Vector<Upp::PyValue>& args, void* user_data)
{
	Adventure::Program* prog = GetProgram(user_data);
	
	if(args.GetCount() < 1) {
		return Upp::PyValue::None();
	}
	
	// Can be a position (int) or an actor (EscValue)
	if(args[0].IsInt()) {
		Upp::Point pos;
		pos.x = args[0].AsInt();
		pos.y = 0;
		prog->CameraAt(pos);
	} else {
		EscValue target = PyToEscValue(args[0]);
		prog->CameraAt(Upp::Point(0, 0)); // Would need proper implementation
	}
	
	return Upp::PyValue::None();
}

Upp::PyValue AdventureBindings::camera_pan_to(const Upp::Vector<Upp::PyValue>& args, void* user_data)
{
	Adventure::Program* prog = GetProgram(user_data);
	
	if(args.GetCount() < 1) {
		return Upp::PyValue::None();
	}
	
	EscValue target = PyToEscValue(args[0]);
	prog->CameraPanTo(target);
	
	return Upp::PyValue::None();
}

Upp::PyValue AdventureBindings::put_at(const Upp::Vector<Upp::PyValue>& args, void* user_data)
{
	Adventure::Program* prog = GetProgram(user_data);
	
	if(args.GetCount() < 3) {
		return Upp::PyValue::None();
	}
	
	EscValue obj = PyToEscValue(args[0]);
	int x = args[1].AsInt();
	int y = args[2].AsInt();
	EscValue room = args.GetCount() > 3 ? PyToEscValue(args[3]) : EscValue();
	
	// This would need a proper PutAt implementation in Program
	// For now, we set the object's x, y properties directly
	// In ESC, this is typically: obj.x = x; obj.y = y; obj.in_room = room;
	
	return Upp::PyValue::None();
}

// ============================================================================
// UI Functions - Dialog & Text
// ============================================================================

Upp::PyValue AdventureBindings::say_line(const Upp::Vector<Upp::PyValue>& args, void* user_data)
{
	Adventure::Program* prog = GetProgram(user_data);
	
	if(args.GetCount() < 1) {
		return Upp::PyValue::None();
	}
	
	// Check if first arg is an actor (multi-arg form) or text (single-arg form)
	WString text;
	EscValue actor;
	
	if(args[0].GetType() == Upp::PY_STR) {
		// say_line(text)
		text = args[0].GetStr();
	} else if(args.GetCount() >= 2 && args[1].GetType() == Upp::PY_STR) {
		// say_line(actor, text, wait, duration)
		actor = PyToEscValue(args[0]);
		text = args[1].GetStr();
	}
	
	// Convert :newlines to actual newlines if present
	Upp::String str_text = text.ToString();
	str_text.Replace(":\\n", "\n");
	
	// Call the C++ implementation
	// prog->SayLine(text); // Would need this method
	
	return Upp::PyValue::None();
}

Upp::PyValue AdventureBindings::print_line(const Upp::Vector<Upp::PyValue>& args, void* user_data)
{
	Adventure::Program* prog = GetProgram(user_data);
	
	if(args.GetCount() < 1) {
		return Upp::PyValue::None();
	}
	
	WString text = args[0].GetStr();
	int x = args.GetCount() > 1 ? args[1].AsInt() : 0;
	int y = args.GetCount() > 2 ? args[2].AsInt() : 0;
	int color = args.GetCount() > 3 ? args[3].AsInt() : 1;
	bool shadow = args.GetCount() > 4 ? args[4].IsTrue() : false;
	bool centered = args.GetCount() > 5 ? args[5].IsTrue() : false;
	int width = args.GetCount() > 6 ? args[6].AsInt() : 0;
	bool outline = args.GetCount() > 7 ? args[7].IsTrue() : false;
	
	// Call the C++ implementation
	// prog->PrintLine(text, x, y, color, shadow, centered, width, outline);
	
	return Upp::PyValue::None();
}

// ============================================================================
// Game Logic Functions
// ============================================================================

Upp::PyValue AdventureBindings::break_time(const Upp::Vector<Upp::PyValue>& args, void* user_data)
{
	Adventure::Program* prog = GetProgram(user_data);
	
	int frames = 1; // Default to 1 frame if no argument
	if(args.GetCount() > 0 && !args[0].IsNone()) {
		frames = args[0].AsInt();
	}
	
	// In ESC, break_time yields control for N frames
	// This would need integration with the script scheduler
	// For now, just return
	
	return Upp::PyValue::None();
}

Upp::PyValue AdventureBindings::cutscene(const Upp::Vector<Upp::PyValue>& args, void* user_data)
{
	Adventure::Program* prog = GetProgram(user_data);
	
	if(args.GetCount() < 2) {
		return Upp::PyValue::None();
	}
	
	int type = args[0].AsInt();
	Upp::PyValue setup_fn = args[1];
	Upp::PyValue cleanup_fn = args.GetCount() > 2 ? args[2] : Upp::PyValue::None();
	
	// Store the function for later execution
	// This would need to integrate with the cutscene system
	
	if(setup_fn.IsFunction()) {
		// Could call it immediately or schedule it
		// For now, we just acknowledge it
	}
	
	return Upp::PyValue::None();
}

Upp::PyValue AdventureBindings::pickup_obj(const Upp::Vector<Upp::PyValue>& args, void* user_data)
{
	Adventure::Program* prog = GetProgram(user_data);
	
	if(args.GetCount() < 1) {
		return Upp::PyValue::None();
	}
	
	EscValue obj = PyToEscValue(args[0]);
	EscValue actor = args.GetCount() > 1 ? PyToEscValue(args[1]) : EscValue();
	
	// Call the C++ implementation
	// prog->PickupObject(obj, actor);
	
	return Upp::PyValue::None();
}

// ============================================================================
// Drawing Functions
// ============================================================================

Upp::PyValue AdventureBindings::set_trans_col(const Upp::Vector<Upp::PyValue>& args, void* user_data)
{
	Adventure::Program* prog = GetProgram(user_data);
	
	if(args.GetCount() < 2) {
		return Upp::PyValue::None();
	}
	
	int color = args[0].AsInt();
	bool enable = args[1].IsTrue();
	
	// Call the C++ implementation
	// prog->SetTransparencyColor(color, enable);
	
	return Upp::PyValue::None();
}

Upp::PyValue AdventureBindings::map_draw(const Upp::Vector<Upp::PyValue>& args, void* user_data)
{
	Adventure::Program* prog = GetProgram(user_data);
	
	if(args.GetCount() < 6) {
		return Upp::PyValue::None();
	}
	
	int src_x = args[0].AsInt();
	int src_y = args[1].AsInt();
	int dest_x = args[2].AsInt();
	int dest_y = args[3].AsInt();
	int width = args[4].AsInt();
	int height = args[5].AsInt();
	int flags = args.GetCount() > 6 ? args[6].AsInt() : 0;
	
	// Call the C++ implementation
	// prog->MapDraw(src_x, src_y, dest_x, dest_y, width, height, flags);
	
	return Upp::PyValue::None();
}

// ============================================================================
// Registration
// ============================================================================

void AdventureBindings::RegisterAll(Upp::PyVM& vm, Adventure::Program& prog)
{
	Upp::PyValue globals = vm.GetGlobals();
	
	// Core functions - Room & Camera
	globals.SetItem(Upp::PyValue("change_room"), Upp::PyValue::Function("change_room", change_room, &prog));
	globals.SetItem(Upp::PyValue("camera_follow"), Upp::PyValue::Function("camera_follow", camera_follow, &prog));
	globals.SetItem(Upp::PyValue("camera_at"), Upp::PyValue::Function("camera_at", camera_at, &prog));
	globals.SetItem(Upp::PyValue("camera_pan_to"), Upp::PyValue::Function("camera_pan_to", camera_pan_to, &prog));
	globals.SetItem(Upp::PyValue("put_at"), Upp::PyValue::Function("put_at", put_at, &prog));
	
	// UI functions - Dialog & Text
	globals.SetItem(Upp::PyValue("say_line"), Upp::PyValue::Function("say_line", say_line, &prog));
	globals.SetItem(Upp::PyValue("print_line"), Upp::PyValue::Function("print_line", print_line, &prog));
	
	// Game logic functions
	globals.SetItem(Upp::PyValue("break_time"), Upp::PyValue::Function("break_time", break_time, &prog));
	globals.SetItem(Upp::PyValue("cutscene"), Upp::PyValue::Function("cutscene", cutscene, &prog));
	globals.SetItem(Upp::PyValue("pickup_obj"), Upp::PyValue::Function("pickup_obj", pickup_obj, &prog));
	
	// Drawing functions
	globals.SetItem(Upp::PyValue("set_trans_col"), Upp::PyValue::Function("set_trans_col", set_trans_col, &prog));
	globals.SetItem(Upp::PyValue("map_draw"), Upp::PyValue::Function("map_draw", map_draw, &prog));
}
