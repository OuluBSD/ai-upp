#include "Holograph.h"

#if defined flagOPENHMD && ((defined flagLINUX) || (defined flagFREEBSD))
#include <openhmd.h>


#ifdef Status
#undef Status
#endif

NAMESPACE_UPP


struct HoloOpenHMD::NativeSinkDevice {
	ohmd_context* ctx;
	ohmd_device_settings* settings;
	const char* fragment;
	const char* vertex;
	ohmd_device* hmd;
	ohmd_device* ctrl[2];
	Size screen_sz;
	CtrlEvent ev;
	ControllerMatrix ev3d;
	bool ev_sendable;
	int seq;
	TimeStop ts;
	int control_count[2];
	int controls_fn[2][64];
	int controls_types[2][64];
};



void PrintOHMD(HoloOpenHMD::NativeSinkDevice& dev, String name, int len, ohmd_float_value val)
{
	float f[16];
	ASSERT(len <= 16);
	ohmd_device_getf(dev.hmd, val, f);
	
	String s;
	s << name << ":";
	if (name.GetCount() < 25) s.Cat(' ', 25 - name.GetCount());
	for(int i = 0; i < len; i++) {
		if (i) s << ", ";
		s << DblStr(f[i]);
	}
	LOG("HoloOpenHMD: info: " << s);
}

void PrintOHMD(HoloOpenHMD::NativeSinkDevice& dev, String name, int len, ohmd_int_value val)
{
	int v[16];
	ASSERT(len <= 16);
	ohmd_device_geti(dev.hmd, val, v);
	
	String s;
	s << name << ":";
	if (name.GetCount() < 25) s.Cat(' ', 25 - name.GetCount());
	for(int i = 0; i < len; i++) {
		if (i) s << ", ";
		s << IntStr(v[i]);
	}
	LOG("HoloOpenHMD: info: " << s);
}

bool HoloOpenHMD::SinkDevice_Create(NativeSinkDevice*& dev) {
	dev = new NativeSinkDevice;
	return true;
}

void HoloOpenHMD::SinkDevice_Destroy(NativeSinkDevice*& dev) {
	delete dev;
}

bool HoloOpenHMD::SinkDevice_Initialize(NativeSinkDevice& dev, AtomBase& a, const WorldState& ws) {
	bool emulate = ws.IsTrue("emulatedevice", false);
	bool require_hmd = !ws.IsTrue("device.optional.hmd", false);
	bool require_left = !ws.IsTrue("device.optional.left", true);
	bool require_right = !ws.IsTrue("device.optional.right", true);
	int user_hmd_idx = ws.GetInt("device.hmd.idx", require_hmd ? 0 : -1);
	int user_ctrl_idx[2];
	user_ctrl_idx[0] = ws.GetInt("device.left.idx", require_left ? 0 : -1);
	user_ctrl_idx[1] = ws.GetInt("device.right.idx", require_right ? 0 : -1);
	bool verbose = !ws.IsTrue("quiet", false) || ws.IsTrue("verbose", false);
	
	if (emulate) {
		require_hmd = false;
		user_hmd_idx = -1;
	}
	
	dev.seq = 0;
	dev.ev.spatial = &dev.ev3d;
	
	
	// Get & check openhmd version
	int major, minor, patch;
	ohmd_get_version(&major, &minor, &patch);
	LOG("OpenHMD version: " << major << "." << minor << "." << patch);
	int code = major * 100 + minor;
	if (code < 3) {
		LOG("HoloOpenHMD::SinkDevice_Initialize: error: too old openhmd version (" << major << "." << minor << ")");
		return false;
	}
	
	// Create openhmd context
	dev.ctx = ohmd_ctx_create();
	
	// Probe for devices
	int num_devices = ohmd_ctx_probe(dev.ctx);
	if (num_devices < 0){
		LOG("HoloOpenHMD::SinkDevice_Initialize: error: failed to probe devices: " << ohmd_ctx_get_error(dev.ctx));
		return false;
	}
	if (num_devices == 0 && !emulate){
		LOG("HoloOpenHMD::SinkDevice_Initialize: error: no connected devices");
		return false;
	}
	
	// Find device indices;
	int hmd_idx = -1, hmd_count = 0;
	int ctrl_idx[2] = {-1,-1}, ctrl_count[2] = {0,0};
	for (int i = 0; i < num_devices; i++) {
		int device_class = 0, device_flags = 0;
		ohmd_list_geti(dev.ctx, i, OHMD_DEVICE_CLASS, &device_class);
		ohmd_list_geti(dev.ctx, i, OHMD_DEVICE_FLAGS, &device_flags);
		String path = ohmd_list_gets(dev.ctx, i, OHMD_PATH);
		
		if (device_class == OHMD_DEVICE_CLASS_HMD && path != "(none)") {
			if (hmd_count == user_hmd_idx || hmd_idx < 0)
				hmd_idx = i;
			hmd_count++;
		}
		else if (device_class == OHMD_DEVICE_CLASS_CONTROLLER) {
			int j = (device_flags & OHMD_DEVICE_FLAGS_LEFT_CONTROLLER ? 0 : 1);
			if (ctrl_count[j] >= user_ctrl_idx[j] || ctrl_idx[j] < 0)
				ctrl_idx[j] = i;
			ctrl_count[j]++;
		}
		
	}
	
	// Dump device info
	if (verbose) {
		LOG("HoloOpenHMD::SinkDevice_Initialize: dumping openhmd device info");
		for(int i = 0; i < num_devices; i++){
			int device_class = 0, device_flags = 0;
			const char* device_class_s[] = {"HMD", "Controller", "Generic Tracker", "Unknown"};
	
			ohmd_list_geti(dev.ctx, i, OHMD_DEVICE_CLASS, &device_class);
			ohmd_list_geti(dev.ctx, i, OHMD_DEVICE_FLAGS, &device_flags);
	
			LOG("device " << i);
			LOG("  vendor:  " << ohmd_list_gets(dev.ctx, i, OHMD_VENDOR));
			LOG("  product: " << ohmd_list_gets(dev.ctx, i, OHMD_PRODUCT));
			LOG("  path:    " << ohmd_list_gets(dev.ctx, i, OHMD_PATH));
			LOG("  class:   " << device_class_s[device_class > OHMD_DEVICE_CLASS_GENERIC_TRACKER ? 4 : device_class]);
			LOG("  flags:   " << HexStr(device_flags));
			LOG("    null device:         " << (device_flags & OHMD_DEVICE_FLAGS_NULL_DEVICE ? "yes" : "no"));
			LOG("    rotational tracking: " << (device_flags & OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING ? "yes" : "no"));
			LOG("    positional tracking: " << (device_flags & OHMD_DEVICE_FLAGS_POSITIONAL_TRACKING ? "yes" : "no"));
			LOG("    left controller:     " << (device_flags & OHMD_DEVICE_FLAGS_LEFT_CONTROLLER ? "yes" : "no"));
			LOG("    right controller:    " << (device_flags & OHMD_DEVICE_FLAGS_RIGHT_CONTROLLER ? "yes" : "no"));
		}
	}

	dev.settings = ohmd_device_settings_create(dev.ctx);

	// If OHMD_IDS_AUTOMATIC_UPDATE is set to 0, ohmd_ctx_update() must be called at least 10 times per second.
	// It is enabled by default.

	int auto_update = 0;
	ohmd_device_settings_seti(dev.settings, OHMD_IDS_AUTOMATIC_UPDATE, &auto_update);

	if (hmd_idx >= 0)
		dev.hmd = ohmd_list_open_device_s(dev.ctx, hmd_idx, dev.settings);
	
	for(int i = 0; i < 2; i++) {
		if (ctrl_idx[i] >= 0)
			dev.ctrl[i] = ohmd_list_open_device_s(dev.ctx, ctrl_idx[i], dev.settings);
	}
	
	if(	(hmd_idx >= 0 && !dev.hmd) ||
		(ctrl_idx[0] >= 0 && !dev.ctrl[0]) ||
		(ctrl_idx[1] >= 0 && !dev.ctrl[1])){
		LOG("HoloOpenHMD::SinkDevice_Initialize: error: failed to open device: " << ohmd_ctx_get_error(dev.ctx));
		if (!emulate)
			return false;
	}
	
	if (hmd_idx < 0) {
		LOG("HoloOpenHMD::SinkDevice_Initialize: error: could not find any hmd device");
		if (require_hmd)
			return false;
		
		dev.screen_sz = Size(1280, 720);
		dev.vertex = "";
		dev.fragment = "";
		dev.ev_sendable = true;
		dev.ts.Reset();
		return true;
	}
	
	// Dump device info
	
	// Get controller info
	for(int i = 0; i < 2; i++) {
		if (dev.ctrl[i]) {
			ohmd_device_geti(dev.ctrl[i], OHMD_CONTROL_COUNT, &dev.control_count[i]);
			ohmd_device_geti(dev.hmd, OHMD_CONTROLS_HINTS, dev.controls_fn[i]);
			ohmd_device_geti(dev.hmd, OHMD_CONTROLS_TYPES, dev.controls_types[i]);
			
			
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
			
			int c = dev.control_count[i];
			if (c > 0) {
				LOG("HoloOpenHMD: control " << i << ":");
				for(int j = 0; j < c; j++){
					LOG(controls_fn_str[dev.controls_fn[i][j]] << " (" <<
						controls_type_str[dev.controls_types[i][j]] << ")" <<
						(j == c - 1 ? "" : ", "));
				}
			}
		}
	}
	
	ohmd_device_settings_destroy(dev.settings);

	dev.ts.Reset();
	return true;
}

bool HoloOpenHMD::SinkDevice_PostInitialize(NativeSinkDevice& dev, AtomBase& a) {
	
	return true;
}

bool HoloOpenHMD::SinkDevice_Start(NativeSinkDevice& dev, AtomBase& a) {
	
	return true;
}

void HoloOpenHMD::SinkDevice_Stop(NativeSinkDevice& dev, AtomBase& a) {
	
}

void HoloOpenHMD::SinkDevice_Uninitialize(NativeSinkDevice& dev, AtomBase& a) {
	ohmd_ctx_destroy(dev.ctx);
}

bool HoloOpenHMD::SinkDevice_IsReady(NativeSinkDevice& dev, AtomBase&, PacketIO& io) {
	if (!dev.hmd) {
		dev.ev_sendable = true;
		dev.ev.type = EVENT_HOLO_STATE;
		return true;
	}
	memset(dev.ev3d.ctrl, 0, sizeof(dev.ev3d.ctrl));
	
	const float wait_time = 1.0 / 30;
	if (dev.ts.Seconds() < wait_time)
		return false;
	dev.ts.Reset();
	
	ohmd_ctx_update(dev.ctx);
	
	quat q;
	ohmd_device_getf(dev.hmd, OHMD_ROTATION_QUAT, q.data.data);
	LOG(q.ToString());
	
	vec3 p;
	ohmd_device_getf(dev.hmd, OHMD_POSITION_VECTOR, p.data);
	LOG(p.ToString());
	
	TODO
	
	// set hmd rotation, for left eye.
	float l_proj[16];
	float l_view[16];
	ohmd_device_getf(dev.hmd, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX, dev.ev3d.l_proj);
	ohmd_device_getf(dev.hmd, OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX, dev.ev3d.l_view);
	
	// set hmd rotation, for right eye.
	float r_proj[16];
	float r_view[16];
	ohmd_device_getf(dev.hmd, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX, dev.ev3d.r_proj);
	ohmd_device_getf(dev.hmd, OHMD_RIGHT_EYE_GL_MODELVIEW_MATRIX, dev.ev3d.r_view);
	
	
	bool verbose = true;
	
	for(int i = 0; i < 2; i++) {
		auto d = dev.ctrl[i];
		ControllerMatrix::Ctrl& ctrl = dev.ev3d.ctrl[i];
		int c = dev.control_count[i];
		
		ctrl.is_enabled = dev.ctrl[i] != 0;
		
		if (c) {
			ohmd_device_getf(d, OHMD_ROTATION_QUAT, ctrl.rot);
			ohmd_device_getf(d, OHMD_POSITION_VECTOR, ctrl.pos);
			
			float control_state[256];
			ohmd_device_getf(d, OHMD_CONTROLS_STATE, control_state);
			
			for(int j = 0; j < c; j++) {
				ControllerMatrix::Value type = ControllerMatrix::INVALID;
				switch (dev.controls_fn[i][j]) {
					#undef CTRL
					#define CTRL(x) case OHMD_##x: type = ControllerMatrix::Value::x; break;
					CTRL(GENERIC)
					CTRL(TRIGGER)
					CTRL(TRIGGER_CLICK)
					CTRL(SQUEEZE)
					CTRL(MENU)
					CTRL(HOME)
					CTRL(ANALOG_X)
					CTRL(ANALOG_Y)
					CTRL(ANALOG_PRESS)
					CTRL(BUTTON_A)
					CTRL(BUTTON_B)
					CTRL(BUTTON_X)
					CTRL(BUTTON_Y)
					CTRL(VOLUME_PLUS)
					CTRL(VOLUME_MINUS)
					CTRL(MIC_MUTE)
					#undef CTRL
					default: break;
				}
				if (type != ControllerMatrix::INVALID) {
					ctrl.is_value[type] = true;
					ctrl.value[type] = control_state[i];
				}
			}
			
			if (verbose) {
				String s;
				for(int i = 0; i < c; i++) {
					if (i)
						s << ", ";
					s << i << "=" << control_state[i];
				}
				LOG("HoloOpenHMD::SinkDevice_IsReady: " << s);
			}
		}
	}
	
	dev.ev_sendable = true;
	dev.ev.type = EVENT_HOLO_STATE;
	
	return true;
}

bool HoloOpenHMD::SinkDevice_Send(NativeSinkDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	ASSERT(dev.ev_sendable);
	if (!dev.ev_sendable)
		return false;
	
	ValueFormat fmt = out.GetFormat();
	RTLOG("HoloOpenHMD::SinkDevice_Send: " << fmt.ToString());
	
	if (fmt.IsEvent()) {
		UPP::CtrlEvent& dst = out.SetData<UPP::CtrlEvent>();
		dst = dev.ev;
		out.seq = dev.seq++;
		if (a.packet_router && !a.router_source_ports.IsEmpty() && fmt.IsValid()) {
			int credits = a.RequestCredits(src_ch, 1);
			if (credits <= 0) {
				RTLOG("HoloOpenHMD::SinkDevice_Send: credit request denied for src_ch=" << src_ch);
				return false;
			}
			Packet route_pkt = CreatePacket(out.GetOffset());
			route_pkt->Pick(out);
			route_pkt->SetFormat(fmt);
			bool routed = a.EmitViaRouter(src_ch, route_pkt);
			a.AckCredits(src_ch, credits);
			out.Pick(*route_pkt);
			if (!routed)
				return false;
		}
	}
	
	return true;
}

bool HoloOpenHMD::SinkDevice_Recv(NativeSinkDevice& dev, AtomBase& a, int sink_ch, const Packet& in) {
	return true;
}

void HoloOpenHMD::SinkDevice_Finalize(NativeSinkDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg) {
	
}





END_UPP_NAMESPACE
#endif
