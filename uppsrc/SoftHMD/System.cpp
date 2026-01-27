#include "SoftHMD.h"
#include <Core/Core.h>

NAMESPACE_HMD_BEGIN

System::System() {
	//verbose = true;
	hmd = 0;
	ctrl[0] = ctrl[1] = 0;
	ctx = 0;
	settings = 0;
}

bool System::Initialise() {
	LOG("System::Initialise: starting");
	seq = 0;
	ev.type = EVENT_HOLO_STATE;
	ev.trans = &trans;
	ev.ctrl = &ev3d;
	has_initial_orient = 0;
	initial_orient.SetNull();
	
	// Get & check openhmd version
	int major, minor, patch;
	HMD::GetVersion(&major, &minor, &patch);
	LOG("SoftHMD version: " << major << "." << minor << "." << patch);
	int code = major * 100 + minor;
	if (code < 3) {
		LOG("HoloSoftHMD::SinkDevice_Initialize: error: too old openhmd version (" << major << "." << minor << ")");
		return false;
	}
	
	// Create openhmd context
	int num_devices = 0;
	if(!emulate) {
		ctx = HMD::CreateContext();
		
		// Probe for devices
		num_devices = HMD::ProbeContext(ctx);
		if (num_devices < 0){
			LOG("HoloSoftHMD::SinkDevice_Initialize: error: failed to probe devices: " << HMD::GetContextError(ctx));
			return false;
		}
		if (num_devices == 0){
			LOG("HoloSoftHMD::SinkDevice_Initialize: error: no connected devices");
			if (require_hmd)
				return false;
		}
	}
	else {
		LOG("System::Initialise: Emulation mode, skipping hardware probe.");
	}
	
	// Find device indices;
	int hmd_idx = -1, hmd_count = 0;
	int ctrl_idx[2] = {-1,-1}, ctrl_count[2] = {0,0};
	for (int i = 0; i < num_devices; i++) {
		int device_class = 0, device_flags = 0;
		HMD::GetListInt(ctx, i, HMD::HMD_DEVICE_CLASS, &device_class);
		HMD::GetListInt(ctx, i, HMD::HMD_DEVICE_FLAGS, &device_flags);
		String path = HMD::GetListString(ctx, i, HMD::HMD_PATH);
		
		// Skip null devices unless emulating
		if (!emulate && (device_flags & HMD::HMD_DEVICE_FLAGS_NULL_DEVICE))
			continue;
		
		if (device_class == HMD::HMD_DEVICE_CLASS_HMD && (emulate || path != "(none)")) {
			if (hmd_count == user_hmd_idx || hmd_idx < 0)
				hmd_idx = i;
			hmd_count++;
		}
		else if (device_class == HMD::HMD_DEVICE_CLASS_CONTROLLER) {
			int j = (device_flags & HMD::HMD_DEVICE_FLAGS_LEFT_CONTROLLER ? 0 : 1);
			if (ctrl_count[j] == user_ctrl_idx[j] || ctrl_idx[j] < 0)
				ctrl_idx[j] = i;
			ctrl_count[j]++;
		}
	}

	if (hmd_idx < 0 && require_hmd) {
		LOG("HoloSoftHMD::SinkDevice_Initialize: error: could not find any hmd device");
		return false;
	}

	if (ctx) {
		settings = HMD::CreateDeviceSettings(ctx);
		int auto_update = 0;
		HMD::SetDeviceSettingsInt(settings, HMD::HMD_IDS_AUTOMATIC_UPDATE, &auto_update);
	
		if (hmd_idx >= 0) {
			hmd = HMD::OpenListDeviceSettings(ctx, hmd_idx, settings);
			if (!hmd) {
				LOG("HoloSoftHMD::SinkDevice_Initialize: error: could not open hmd device");
				if (require_hmd) return false;
			}
		}
	
		for(int i = 0; i < 2; i++) {
			if (ctrl_idx[i] >= 0) {
				ctrl[i] = HMD::OpenListDeviceSettings(ctx, ctrl_idx[i], settings);
				if (!ctrl[i]) {
					LOG("HoloSoftHMD::SinkDevice_Initialize: warning: could not open controller " << i);
				}
			}
		}
	}

	if (hmd) {
		float ipd;
		HMD::GetDeviceFloat(hmd, HMD::HMD_EYE_IPD, &ipd);
		
		float viewport_scale[2];
		float distortion_coeffs[4];
		float aberr_scale[3];
		float sep;
		float left_lens_center[2];
		float right_lens_center[2];
		
		// Viewport is half the screen
		HMD::GetDeviceFloat(hmd, HMD::HMD_SCREEN_HORIZONTAL_SIZE, &(viewport_scale[0]));
		viewport_scale[0] /= 2.0f;
		HMD::GetDeviceFloat(hmd, HMD::HMD_SCREEN_VERTICAL_SIZE, &(viewport_scale[1]));
		
		// Distortion coefficients
		HMD::GetDeviceFloat(hmd, HMD::HMD_UNIVERSAL_DISTORTION_K, &(distortion_coeffs[0]));
		HMD::GetDeviceFloat(hmd, HMD::HMD_UNIVERSAL_ABERRATION_K, &(aberr_scale[0]));
		
		// Calculate lens centers (assuming the eye separation is the distance between the lens centers)
		HMD::GetDeviceFloat(hmd, HMD::HMD_LENS_HORIZONTAL_SEPARATION, &sep);
		HMD::GetDeviceFloat(hmd, HMD::HMD_LENS_VERTICAL_POSITION, &(left_lens_center[1]));
		HMD::GetDeviceFloat(hmd, HMD::HMD_LENS_VERTICAL_POSITION, &(right_lens_center[1]));
		left_lens_center[0] = viewport_scale[0] - sep/2.0f;
		right_lens_center[0] = sep/2.0f;
		
		// Assume calibration was for lens view to which ever edge of screen is further away from lens center
		//float warp_scale = (left_lens_center[0] > right_lens_center[0]) ? left_lens_center[0] : right_lens_center[0];
		float warp_adj = 1.0f;
		
		int w, h;
		HMD::GetDeviceInt(hmd, HMD::HMD_SCREEN_HORIZONTAL_RESOLUTION, &w);
		HMD::GetDeviceInt(hmd, HMD::HMD_SCREEN_VERTICAL_RESOLUTION, &h);
		screen_sz = Size(w, h);
	}
	else {
		screen_sz = Size(1280, 720);
	}
	
	// Get controller info
	for(int i = 0; i < 2; i++) {
		if (!this->ctrl[i])
			continue;
		
		HMD::GetDeviceInt(this->ctrl[i], HMD::HMD_CONTROL_COUNT, &control_count[i]);
		HMD::GetDeviceInt(this->ctrl[i], HMD::HMD_CONTROLS_HINTS, controls_fn[i]);
		HMD::GetDeviceInt(this->ctrl[i], HMD::HMD_CONTROLS_TYPES, controls_types[i]);
		
		const char* controls_fn_str[] = {
			"generic",
			"trigger",
			"trigger_click",
			"squeeze",
			"menu",
			"home",
			"analog-x",
			"analog-y",
			"anlog_press",
			"button-a",
			"button-b",
			"button-x",
			"button-y",
			"volume-up",
			"volume-down",
			"mic-mute"
		};
		
		const char* controls_type_str[] = {"digital", "analog"};
		
		int c = control_count[i];
		if (c > 0) {
			LOG("HoloSoftHMD: control " << i << ":");
			for(int j = 0; j < c; j++){
				LOG(controls_fn_str[controls_fn[i][j]] << " (" <<
					controls_type_str[controls_types[i][j]] << ")" <<
					(j == c - 1 ? "" : ", "));
			}
		}
	}
	
	ev_sendable = true;
	return true;
}

void System::Start() {
	if (!flag.IsRunning()) {
		flag.Start(1);
		Upp::Thread::Start(callback(this, &System::BackgroundProcess));
	}
}

void System::Stop() {
	flag.Stop();
	flag.Wait();
}

void System::Uninitialise() {
	Stop();
	
	if (ctx)
		HMD::DestroyContext(ctx);
	ctx = 0;
}

void System::BackgroundProcess() {
	while (flag.IsRunning()) {
		UpdateData();
		Sleep(1000/60);
	}
	
	flag.DecreaseRunning();
}

void System::UpdateData() {
	if (!hmd)
		return;
	trans.Clear();
	
	HMD::UpdateContext(ctx);
	
	HMD::GetDeviceFloat(hmd, HMD::HMD_ROTATION_QUAT, trans.orientation.data.data);
	LOGI("UpdateData: orient=%f %f %f %f", trans.orientation.data.data[0], trans.orientation.data.data[1], trans.orientation.data.data[2], trans.orientation.data.data[3]);
	
	if (!has_initial_orient) {
		has_initial_orient = true;
		
		vec3 axes;
		float& yaw = axes[0];
		float& pitch = axes[1];
		float& roll = axes[2];
		QuatAxes(trans.orientation, yaw, pitch, roll);
		pitch = 0;
		roll = 0;
		initial_orient = AxesQuat(yaw, pitch, roll);
	}
	
	trans.orientation = MatQuat(QuatMat(trans.orientation) * QuatMat(initial_orient));
	
	trans.mode = UPP::TransformMatrix::MODE_AXES;
	trans.is_stereo = true;
	trans.position = vec3(0,0,0);
	trans.FillFromOrientation();
	
	if (verbose) {
		LOG("Head: pos " << trans.position.ToString() << ", " << trans.GetAxesString());
	}
	
	HMD::GetDeviceFloat(hmd, HMD::HMD_EYE_IPD, &trans.eye_dist);
	
	for(int i = 0; i < 2; i++) {
		ControllerMatrix::Ctrl& h = ev3d.ctrl[i];
		h.Clear();
		if (this->ctrl[i]) {
			h.is_enabled = true;
			HMD::GetDeviceFloat(this->ctrl[i], HMD::HMD_ROTATION_QUAT, h.trans.orientation.data.data);
			HMD::GetDeviceFloat(this->ctrl[i], HMD::HMD_POSITION_VECTOR, h.trans.position.data);
			
			h.trans.FillFromOrientation();
			
			int c = control_count[i];
			float values[64];
			HMD::GetDeviceFloat(this->ctrl[i], HMD::HMD_CONTROLS_STATE, values);
			for (int j = 0; j < c; j++) {
				int fn = controls_fn[i][j];
				float val = values[j];
				
				switch (fn) {
					case HMD::HMD_TRIGGER: h.value[ControllerMatrix::TRIGGER] = val; h.is_value[ControllerMatrix::TRIGGER] = true; break;
					case HMD::HMD_TRIGGER_CLICK: h.value[ControllerMatrix::TRIGGER_CLICK] = val; h.is_value[ControllerMatrix::TRIGGER_CLICK] = true; break;
					case HMD::HMD_SQUEEZE: h.value[ControllerMatrix::SQUEEZE] = val; h.is_value[ControllerMatrix::SQUEEZE] = true; break;
					case HMD::HMD_MENU: h.value[ControllerMatrix::MENU] = val; h.is_value[ControllerMatrix::MENU] = true; break;
					case HMD::HMD_HOME: h.value[ControllerMatrix::HOME] = val; h.is_value[ControllerMatrix::HOME] = true; break;
					case HMD::HMD_ANALOG_X: h.value[ControllerMatrix::ANALOG_X] = val; h.is_value[ControllerMatrix::ANALOG_X] = true; break;
					case HMD::HMD_ANALOG_Y: h.value[ControllerMatrix::ANALOG_Y] = val; h.is_value[ControllerMatrix::ANALOG_Y] = true; break;
					case HMD::HMD_ANALOG_PRESS: h.value[ControllerMatrix::ANALOG_PRESS] = val; h.is_value[ControllerMatrix::ANALOG_PRESS] = true; break;
					case HMD::HMD_BUTTON_A: h.value[ControllerMatrix::BUTTON_A] = val; h.is_value[ControllerMatrix::BUTTON_A] = true; break;
					case HMD::HMD_BUTTON_B: h.value[ControllerMatrix::BUTTON_B] = val; h.is_value[ControllerMatrix::BUTTON_B] = true; break;
					case HMD::HMD_BUTTON_X: h.value[ControllerMatrix::BUTTON_X] = val; h.is_value[ControllerMatrix::BUTTON_X] = true; break;
					case HMD::HMD_BUTTON_Y: h.value[ControllerMatrix::BUTTON_Y] = val; h.is_value[ControllerMatrix::BUTTON_Y] = true; break;
				}
			}
		}
	}
	
	if (WhenSensorEvent)
		WhenSensorEvent(ev);
}

void System::PrintHMD(String name, int len, HMD::FloatValue val) {
	float f[16];
	HMD::GetDeviceFloat(hmd, val, f);
	String s;
	for (int i = 0; i < len; i++) {
		if (i) s << ", ";
		s << f[i];
	}
	LOG(name << ": " << s);
}

void System::PrintHMD(String name, int len, HMD::IntValue val) {
	int f[16];
	HMD::GetDeviceInt(hmd, val, f);
	String s;
	for (int i = 0; i < len; i++) {
		if (i) s << ", ";
		s << f[i];
	}
	LOG(name << ": " << s);
}

NAMESPACE_HMD_END
