#include "Adventure.h"

namespace Adventure {



const SObj* Program::FindRoom(const String& name) const {
	const auto& arr = rooms.GetArray();
	for (const auto& val : arr) {
		String room_str = val;
		//LOG(room_str);
		if (room_str == name)
			return FindDeep(name);
	}
	return 0;
}

SObj Program::GetInRoom(SObj o) {
	//LOG(o.ToString());
	if (o.IsMap()) {
		const auto& map = o.GetMap();
		int i = map.Find("in_room");
		if (i >= 0) {
			SObj in_room = o.MapGet("in_room");
			if (in_room.IsMap())
				return in_room;
			
			DUMPM(o.GetMap());
			TODO
			/*LOG(in_room.ToString());
			String name = in_room;
			if (name.GetCount()) {
				const SObj* ptr = FindDeep(name);
				if (ptr) {
					ASSERT(ptr->IsMap());
					return *ptr;
				}
			}*/
		}
	}
	return SObj();
}

// open one (or more) doors
void Program::OpenDoor(SObj door_obj1, SObj door_obj2) {
	StateType state1 = GetState(door_obj1);
	
	if (state1 == STATE_OPEN) {
		SayLine("it's already open");
	}
	else {
		SetState(door_obj1, STATE_OPEN);
		if (door_obj2)
			SetState(door_obj2, STATE_OPEN);
	}
}

// close one (or more) doors
void Program::CloseDoor(SObj door_obj1, SObj door_obj2) {
	StateType state1 = GetState(door_obj1);
	if (state1 == STATE_CLOSED) {
		SayLine("it's already closed");
	}
	else {
		SetState(door_obj1, STATE_CLOSED);
		if (door_obj2)
			SetState(door_obj2, STATE_CLOSED);
	}
}

void Program::ComeOutDoor(SObj from_door, SObj to_door, bool fade_effect) {
	// check param
	/*if (to_door == NULL) {
		ShowError("target door does not exist");
		return;
	}*/
	EscValue selected_actor = GetSelectedActor();
	
	StateType from_state = GetState(from_door);
	if (from_state != STATE_OPEN) {
		SayLine("the door is closed");
		return;
	}
	
	// go to new room!
	SObj new_room = GetInRoom(to_door);
	
	if (new_room != room_curr) {
		ChangeRoom(new_room, fade_effect); // switch to new room and ...
	    
		// ...auto-position actor at to_door in new room...
		Point pos = GetUsePoint(to_door);
		PutAt(selected_actor, pos.x, pos.y, new_room);
	}
	
	FaceDir to_dir = GetFaceDir(to_door);
	FaceDir opp_dir;
	
	if (to_dir) {
		//  ... in opposite use direction!
		opp_dir = (FaceDir)(((int)to_dir + 1) % 4 + 1);
		
		// opp_dir = to_dir + 2
		// if (opp_dir > 4 {
		// opp_dir -= 4
		// }
	}
	else {
		opp_dir = FACE_FRONT;
	}
	
	selected_actor.MapSet("face_dir", GetFaceString(opp_dir));
	
	// is target dir left? flip?
	selected_actor.MapSet("flip", GetFaceDir(selected_actor) == FACE_LEFT);
	
}

void Program::ChangeRoom(SObj new_room, SObj fade_) {
	// Convert EscValue room to PyValue
	PyValue py_room;
	if(new_room.IsMap()) {
		const VectorMap<EscValue, EscValue>& esc_map = new_room.GetMap();
		py_room = PyValue::Dict();
		VectorMap<PyValue, PyValue>& d = py_room.GetDictRW();
		for(int i = 0; i < esc_map.GetCount(); i++) {
			if(esc_map.GetKey(i).IsStringLike()) {
				d.Add(PyValue(esc_map.GetKey(i).ToString()), EscToPyValue(esc_map[i]));
			}
		}
	}
	
	int fade = fade_.GetInt();

	// stop any existing fade
	if (fade_script) {
		StopScript(*fade_script);
		fade_script = 0;
	}

	// fade out existing room (if any)
	if (fade && room_curr.GetType() == PY_DICT) {
		Fades(fade, 1);
	}

	// execute the exit() callback of old room (Python)
	if (room_curr.GetType() == PY_DICT) {
		PyValue exit_fn = GetDictItem(room_curr, "exit");
		if(exit_fn.IsFunction()) {
			Vector<PyValue> args;
			args.Add(room_curr);
			py_vm.Call(exit_fn, args);
		}
	}

	// stop all active (local) scripts
	ctx.RemoveProgramGroup(SCRIPT_LOCAL);

	// clear current command
	ClearCurrCmd();

	// change to new room
	room_curr = py_room;

	// reset camera pos in new room
	if (cam_following_actor.IsVoid() || GetInRoom(cam_following_actor).IsVoid())
		cam = Point(0,0);

	// stop everyone talking & remove displayed text
	StopTalking();

	// fade up again?
	if (fade) {
		StartScript(THISBACK2(Fades, fade, -1), true);
	}
	else {
		fade_iris = 0;
	}

	// execute the enter() callback of new room (Python)
	if (room_curr.GetType() == PY_DICT) {
		PyValue enter_fn = GetDictItem(room_curr, "enter");
		if(enter_fn.IsFunction()) {
			Vector<PyValue> args;
			args.Add(room_curr);
			py_vm.Call(enter_fn, args);
		}
	}
}

void Program::PutAt(SObj obj, int x, int y, SObj room) {
	//GetReference(obj, true);
	//GetReference(room, true);
	//LOG(obj.ToString());
	//LOG(room.ToString());
	ASSERT(obj.IsMap());
	ASSERT(room.IsMap());
	
	if (room.IsMap()) {
		if (!HasFlag(Classes(obj), "class_actor")) {
			SObj in_room = obj.MapGet("in_room");
			//if (in_room.IsArray())
			//	GetReference(in_room, true);
			ASSERT(in_room.IsMap() || in_room.IsVoid());
			if (in_room.IsMap()) {
				SObj objects = in_room.MapGet("objects");
				Vector<EscValue>& arr = (Vector<EscValue>&)objects.GetArray();
				VectorRemoveKey(arr, obj);
			}
			obj.MapSet("owner", EscValue());
			SObj objects = room.MapGet("objects");
			ASSERT(objects.IsArray());
			objects.ArrayAdd(obj);
		}
	}
	obj.MapSet("in_room", room);
	
	//LOG(obj.ToString());
	ASSERT(obj.IsMap());
	obj.MapSet("x", x);
	obj.MapSet("y", y);
}



}
