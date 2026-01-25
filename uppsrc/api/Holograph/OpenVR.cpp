#include "Holograph.h"

#if defined flagOPENVR

#include <SoftHMD/SoftHMD.h>

#ifdef None
#undef None
#endif


NAMESPACE_UPP



struct HoloOpenVR::NativeSinkDevice {
	void* hmd = 0;
	byte tracked_dev[ 64 * 64 ]; // placeholder for vr::TrackedDevicePose_t[64]
	String driver;
	String display;
	mat4 dev_pose[ 64 ];
	Upp::HMD::System sys;
	TimeStop ts;

	NativeSinkDevice() {
		hmd = 0;
		memset(tracked_dev, 0, sizeof(tracked_dev));
		for(int i = 0; i < 64; i++)
			dev_pose[i] = Upp::Identity<mat4>();
	}
};


bool HoloOpenVR::SinkDevice_Create(NativeSinkDevice*& dev) {
	dev = new NativeSinkDevice;
	return true;
}

void HoloOpenVR::SinkDevice_Destroy(NativeSinkDevice*& dev) {
	delete dev;
}

void HoloOpenVR::SinkDevice_Visit(NativeSinkDevice&, AtomBase&, Visitor& vis) {
	
}

bool HoloOpenVR::SinkDevice_Initialize(NativeSinkDevice& dev, AtomBase& a, const WorldState& ws) {
	LOG("HoloOpenVR::SinkDevice_Initialize: starting");
	ValueMap values = ws.GetValues();
	for(int i = 0; i < values.GetCount(); i++)
		LOG("  ws[" << i << "]: " << values.GetKey(i).ToString() << " = " << values.GetValue(i).ToString());
	
	bool emulate = ws.IsTrue("emulatedevice", false);
	LOG("HoloOpenVR::SinkDevice_Initialize: emulate=" << emulate);
	
	dev.sys.emulate = emulate;
	dev.sys.require_hmd = !ws.IsTrue("device.optional.hmd", false);
	if (emulate) dev.sys.require_hmd = false;
	dev.sys.require_left = !ws.IsTrue("device.optional.left", true);
	dev.sys.require_right = !ws.IsTrue("device.optional.right", true);
	dev.sys.user_hmd_idx = ws.GetInt("device.hmd.idx", dev.sys.require_hmd ? 0 : -1);
	if (emulate) dev.sys.user_hmd_idx = -1;
	dev.sys.user_ctrl_idx[0] = ws.GetInt("device.left.idx", dev.sys.require_left ? 0 : -1);
	dev.sys.user_ctrl_idx[1] = ws.GetInt("device.right.idx", dev.sys.require_right ? 0 : -1);
	dev.sys.verbose = true; // ws.IsTrue("verbose", false); // Causes crash?
	
	dev.sys.ev.ctrl = &dev.sys.ev3d;
	
	bool init_ok = true;
	if (emulate) {
		dev.sys.seq = 0;
		dev.sys.ev.type = EVENT_HOLO_STATE;
		dev.sys.ev.trans = &dev.sys.trans;
		//dev.sys.ev.ctrl = &dev.sys.ev3d; // Already set above
		dev.sys.ev_sendable = true;
		
		// Set orientation: Look back (180 yaw) and down 30 degrees
		dev.sys.trans.orientation = Upp::MatQuat(Upp::XRotation(30.0f * (float)M_PI / 180.0f));
	}
	else {
		init_ok = dev.sys.Initialise();
	}

	LOG("HoloOpenVR::SinkDevice_Initialize: Initialise returned " << init_ok);
	if (!init_ok && !emulate)
		return false;
		
	dev.ts.Reset();
	LOG("HoloOpenVR::SinkDevice_Initialize: success");
	return true;
}

bool HoloOpenVR::SinkDevice_PostInitialize(NativeSinkDevice& dev, AtomBase& a) {
	
	return true;
}

bool HoloOpenVR::SinkDevice_Start(NativeSinkDevice& dev, AtomBase& a) {
	
	return true;
}

void HoloOpenVR::SinkDevice_Stop(NativeSinkDevice& dev, AtomBase& a) {
	
}

void HoloOpenVR::SinkDevice_Uninitialize(NativeSinkDevice& dev, AtomBase& a) {
	dev.sys.Uninitialise();
}

bool HoloOpenVR::SinkDevice_Send(NativeSinkDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	ASSERT(dev.sys.ev_sendable);
	if (!dev.sys.ev_sendable)
		return false;
	
	ValueFormat fmt = out.GetFormat();
	RTLOG("HoloOpenVR::SinkDevice_Send: " << fmt.ToString());
	
	if (fmt.IsEvent()) {
		GeomEvent& dst = out.SetData<GeomEvent>();
		dst = dev.sys.ev;
		out.seq = dev.sys.seq++;
		if (a.packet_router && !a.router_source_ports.IsEmpty() && fmt.IsValid()) {
			int credits = a.RequestCredits(src_ch, 1);
			if (credits <= 0) {
				RTLOG("HoloOpenVR::SinkDevice_Send: credit request denied for src_ch=" << src_ch);
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

bool HoloOpenVR::SinkDevice_Recv(NativeSinkDevice& dev, AtomBase& a, int sink_ch, const Packet& in) {
	return true;
}

void HoloOpenVR::SinkDevice_Finalize(NativeSinkDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg) {
	
}

bool HoloOpenVR::SinkDevice_IsReady(NativeSinkDevice& dev, AtomBase& a, PacketIO& io) {
	dev.sys.UpdateData();
	return true;
}





END_UPP_NAMESPACE
#endif