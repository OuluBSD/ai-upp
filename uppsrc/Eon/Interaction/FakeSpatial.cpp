#include "Interaction.h"
#include <Core/TextParsing/TextParsing.h>

NAMESPACE_UPP


void FakeControllerSource::GetVelocity(float* v3) const {
	COPY3(v3, mgr->hand_velocity);
}

void FakeControllerSource::GetAngularVelocity(float* v3) const {
	COPY3(v3, mgr->hand_angular_velocity);
}







FakeSpatialInteractionManager::FakeSpatialInteractionManager() {
	ctrl.mgr = this;
	ctrl_state.source = &ctrl;

	for(int i = 0; i < 3; i++) av[i].Resize(30);
	
}

bool FakeSpatialInteractionManager::Initialize(InteractionSystem& sys) {
	this->sys = &sys;
	
	hand_velocity = vec3(0, 0.1f, 0.1f);
	hand_angular_velocity = vec3(0.1f, 0.1f, 0);
	
	prev.SetAll(false);
	
	return true;
}

void FakeSpatialInteractionManager::Update(double dt) {
	time += dt;
	last_dt = dt;

	String env_name = sys->env_name;

	if (!env_name.IsEmpty()) {
		String normalized = NormalizePathSeparators(env_name);

		// Debug: show what we're searching for and from where
		VfsValue& search_root = GetVfsValue();
		LOG("FakeSpatialInteractionManager::Update: searching for '" << env_name << "' (normalized: '" << normalized << "')");
		LOG("  search_root id='" << search_root.id << "' sub.GetCount()=" << search_root.sub.GetCount());

		// Debug: show what's in the tree
		if (search_root.sub.GetCount() > 0) {
			LOG("  Children:");
			for (int i = 0; i < min(10, search_root.sub.GetCount()); i++) {
				VfsValue& child = search_root.sub[i];
				LOG("    [" << i << "] id='" << child.id << "' sub.GetCount()=" << child.sub.GetCount());

				// Show grandchildren for entries with id='' or 'loop'
				if ((child.id.IsEmpty() || child.id == "loop") && child.sub.GetCount() > 0) {
					for (int j = 0; j < min(5, child.sub.GetCount()); j++) {
						VfsValue& grandchild = child.sub[j];
						LOG("      [" << i << "][" << j << "] id='" << grandchild.id << "' sub.GetCount()=" << grandchild.sub.GetCount());
					}
				}
			}
		}

		env = search_root.FindOwnerWithPathAndCast<EnvState>(normalized);
		if (!env && normalized != env_name)
			env = search_root.FindOwnerWithPathAndCast<EnvState>(env_name);

		// HACK: Manually navigate to event loop since path resolution doesn't work
		// EnvState is at engine_root.sub[3="loop"].sub[1="event"].sub[0="register"]
		if (!env && search_root.sub.GetCount() > 3) {
			VfsValue& loop_node = search_root.sub[3];
			if (loop_node.id == "loop" && loop_node.sub.GetCount() > 1) {
				VfsValue& event_node = loop_node.sub[1];
				if (event_node.id == "event" && event_node.sub.GetCount() > 0) {
					// Try to find register in event's children
					for (int i = 0; i < event_node.sub.GetCount(); i++) {
						VfsValue& child = event_node.sub[i];
						if (child.id == "register" || child.id.Find("register") >= 0) {
							env = child.FindExt<EnvState>();
							if (env) {
								LOG("FakeSpatialInteractionManager::Update: found EnvState at loop[3].event[1].register[" << i << "]");
								break;
							}
						}
					}
				}
			}
		}

		if (!env) {
			// Only log error once per lookup attempt
			static int error_count = 0;
			if (error_count++ < 5) {  // Limit spam
				LOG("FakeSpatialInteractionManager::Update: error: environment state with name '" << env_name << "' not found");
				LOG("  Tried paths: '" << normalized << "', '" << env_name << "', 'loop" << env_name << "'");
			}
		}
		env_name.Clear();
		
		DetectController();
		Look(Point(0,0)); // camera might be messed up, so update it immediately
	}
	
	if (env) {
		static int update_count = 0;
		if (update_count++ < 3) {
			LOG("FakeSpatialInteractionManager::Update: calling UpdateState()");
		}
		UpdateState();
	}

}

void FakeSpatialInteractionManager::DetectController() {
	GeomEvent ev;
	ev.ctrl = &ctrl_state.props;
	ev.state = &ctrl_state;
	ev.type = EVENT_HOLO_CONTROLLER_DETECTED;
	
	WhenSourceDetected(*this, ev);
}

void FakeSpatialInteractionManager::UpdateState() {
	
	UpdateStateKeyboard();
	
}

void FakeSpatialInteractionManager::UpdateStateKeyboard() {
	if (!env) {
		LOG("FakeSpatialInteractionManager::UpdateStateKeyboard: error: env is null");
		return;
	}

	static int kbd_count = 0;
	if (kbd_count++ < 3) {
		LOG("FakeSpatialInteractionManager::UpdateStateKeyboard: called");
	}

	FboKbd::KeyVec& data = env->Set<FboKbd::KeyVec>(KEYBOARD_PRESSED);

	if (env->GetBool(MOUSE_LEFTDOWN)) {
		Point& drag = env->Set<Point>(MOUSE_TOYCOMPAT_DRAG, Point(0,0));
		
		Point diff = drag - prev_mouse;
		
		if (prev_mouse.x != 0 && prev_mouse.y != 0 && (diff.x || diff.y))
			Look(diff);
		
		prev_mouse = drag;
	}
	else {
		prev_mouse = Point(0,0);
	}
	
	
	bool fwd   = data['w'];
	bool left  = data['a'];
	bool bwd  = data['s'];
	bool right = data['d'];
	
	float step = (float)last_dt * 1.5f;
	
	if (fwd) {
		Move(VEC_FWD, step);
	}
	if (left) {
		Move(VEC_LEFT, step);
	}
	if (bwd) {
		Move(VEC_BWD, step);
	}
	if (right) {
		Move(VEC_RIGHT, step);
	}
	
	static const int assign[ControllerMatrix::VALUE_COUNT] = {
		'1',
		0,
		0,
		'2',
		'3',
		
		0,
		'2',
		0,
		0,
		0,
		
		0,
		0,
		0,
		0,
		0,
		
		0,
		0,
		0,
		0,
		0,
		
		0,
		0,
		0,
		0,
		0,
		
	};
	
	for(int i = 0; i < ControllerMatrix::VALUE_COUNT; i++) {
		int key = assign[i];
		if (!key)
			continue;
		
		bool pushed = data[key] && !prev[key];
		bool released = !data[key] && prev[key];
		
		if (pushed)
			Pressed((ControllerMatrix::Value)i);
		
		if (released)
			Released((ControllerMatrix::Value)i);
		
	}
	
	prev = data;
}

void FakeSpatialInteractionManager::Pressed(ControllerMatrix::Value b) {
	ctrl_state.props.ctrl[1].value[(int)b] = 1.0f;
	ctrl_state.props.ctrl[1].is_value[(int)b] = true;
	
	GeomEvent ev;
	ev.ctrl = &ctrl_state.props;
	ev.state = &ctrl_state;
	ev.type = EVENT_HOLO_PRESSED;
	ev.value = (int)b;
	
	WhenSourcePressed(*this, ev);
}

void FakeSpatialInteractionManager::Released(ControllerMatrix::Value b) {
	ctrl_state.props.ctrl[1].value[(int)b] = 0.0f;
	ctrl_state.props.ctrl[1].is_value[(int)b] = true;
	
	GeomEvent ev;
	ev.ctrl = &ctrl_state.props;
	ev.state = &ctrl_state;
	ev.type = EVENT_HOLO_RELEASED;
	ev.value = (int)b;
	
	WhenSourceReleased(*this, ev);
}








void FakeSpatialInteractionManager::Look(Point mouse_diff) {
	GeomEvent ev;
	ev.ctrl = &ctrl_state.props;
	ev.state = &ctrl_state;
	ev.type = EVENT_HOLO_LOOK;
	ev.pt = mouse_diff; // extra
	ev.trans = &trans;
	
	float prev_yaw = yaw;
	float prev_pitch = pitch;
	
	float rot_speed = 0.05f / (2*M_PIf);
	float yaw_change = -mouse_diff.x * rot_speed;
	float pitch_change = mouse_diff.y * rot_speed;
	yaw += yaw_change;
	pitch += pitch_change;
	
	vec3 prev_head_direction;
	for(int i = 0; i < 3; i++)
		prev_head_direction[i] = (float)av[i].GetMean();
	
	head_direction = AxesDir(yaw, pitch);
	
	for(int i = 0; i < 3; i++)
		av[i].Add(head_direction[i]);
	
	trans.mode = TransformMatrix::MODE_LOOKAT;
	trans.direction = head_direction;
	trans.position = vec3(0,0,0);
	trans.up = VEC_UP;
	trans.FillFromLookAt();
	
	vec3 head_direction_diff = head_direction - prev_head_direction;
	float multiplier = 5;
	hand_velocity = head_direction_diff * multiplier;
	hand_angular_velocity = vec3(yaw, pitch, 0);
	
	if (sys->debug_log) {
		LOG("FakeSpatialInteractionManager::Look: dx: " << mouse_diff.x << ", dy: " << mouse_diff.y <<
			", rot: " << head_direction[0] << ", " << head_direction[1] << ", " << head_direction[2] <<
			", angle: " << yaw << ", angle1: " << pitch);
	}
	
	WhenSourceUpdated(*this, ev);
}

void FakeSpatialInteractionManager::Move(vec3 rel_dir, float step) {
	GeomEvent ev;
	ev.ctrl = &ctrl_state.props;
	ev.state = &ctrl_state;
	ev.type = EVENT_HOLO_MOVE_FAR_RELATIVE;
	ev.trans = &trans;
	vec3 dir = head_direction;
	
	// remove y component
	dir[1] = 0;
	
	float straight = rel_dir[2] * VEC_FWD[2];
	float sideways = rel_dir[0];
	
	if (straight) {
		trans.position = dir * step * straight;
	}
	if (sideways) {
		vec3 s = dir * step * sideways;
		trans.position = vec3(-s[2], 0, +s[0]);
	}
	
	WhenSourceUpdated(*this, ev);
}

VfsValue& FakeSpatialInteractionManager::GetVfsValue() {
	ASSERT(sys);
	if (!sys) throw Exc("No sys ptr");

	// Search from Machine root, not InteractionSystem's VfsValue
	// EnvState is created in the chain hierarchy, not under systems
	Engine* engine = sys->val.FindOwner<Engine>();
	ASSERT(engine);
	if (!engine) throw Exc("No Engine found");

	VfsValue& engine_val = engine->val;

	// Get Machine root from Engine
	for (int i = 0; i < engine_val.sub.GetCount(); i++) {
		VfsValue& v = engine_val.sub[i];
		if (v.id.Find("Machine") >= 0 || v.id.Find("machine") >= 0) {
			LOG("FakeSpatialInteractionManager::GetVfsValue: using Machine root id='" << v.id << "'");
			return v;
		}
	}

	// Fallback: search from Engine root
	LOG("FakeSpatialInteractionManager::GetVfsValue: Machine not found, using Engine root");
	return engine_val;
}


END_UPP_NAMESPACE
