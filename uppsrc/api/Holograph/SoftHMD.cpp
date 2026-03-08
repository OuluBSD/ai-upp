#include "Holograph.h"

#if defined flagSOFTHMD
#include <SoftHMD/SoftHMD.h>


NAMESPACE_UPP



struct HoloSoftHMD::NativeSinkDevice {
	Upp::HMD::System sys;
	TimeStop ts;
};


bool HoloSoftHMD::SinkDevice_Create(NativeSinkDevice*& dev) {
	dev = new NativeSinkDevice;
	return true;
}

void HoloSoftHMD::SinkDevice_Destroy(NativeSinkDevice*& dev) {
	delete dev;
}

void HoloSoftHMD::SinkDevice_Visit(NativeSinkDevice&, AtomBase&, Visitor& vis) {

}

bool HoloSoftHMD::SinkDevice_Initialize(NativeSinkDevice& dev, AtomBase& a, const WorldState& ws) {
	bool emulate = ws.IsTrue(".emulatedevice", false);
	dev.sys.require_hmd = !ws.IsTrue(".device.optional.hmd", false);
	dev.sys.require_left = !ws.IsTrue(".device.optional.left", true);
	dev.sys.require_right = !ws.IsTrue(".device.optional.right", true);
	dev.sys.user_hmd_idx = ws.GetInt(".device.hmd.idx", dev.sys.require_hmd ? 0 : -1);
	dev.sys.user_ctrl_idx[0] = ws.GetInt(".device.left.idx", dev.sys.require_left ? 0 : -1);
	dev.sys.user_ctrl_idx[1] = ws.GetInt(".device.right.idx", dev.sys.require_right ? 0 : -1);
	dev.sys.verbose = ws.IsTrue(".verbose", false);

	if (emulate) {
		dev.sys.require_hmd = false;
		dev.sys.user_hmd_idx = -1;
	}

	if (!dev.sys.Initialise() && !emulate)
		return false;

	dev.ts.Reset();
	return true;
}

bool HoloSoftHMD::SinkDevice_PostInitialize(NativeSinkDevice& dev, AtomBase& a) {

	return true;
}

bool HoloSoftHMD::SinkDevice_Start(NativeSinkDevice& dev, AtomBase& a) {

	return true;
}

void HoloSoftHMD::SinkDevice_Stop(NativeSinkDevice& dev, AtomBase& a) {

}

void HoloSoftHMD::SinkDevice_Uninitialize(NativeSinkDevice& dev, AtomBase& a) {
	dev.sys.Uninitialise();
}

bool HoloSoftHMD::SinkDevice_Send(NativeSinkDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	ASSERT(dev.sys.ev_sendable);
	if (!dev.sys.ev_sendable)
		return false;

	ValueFormat fmt = out.GetFormat();
	//RTLOG("HoloSoftHMD::SinkDevice_Send: " << fmt.ToString());

	if (fmt.IsEvent()) {
		UPP::CtrlEvent& dst = out.SetData<UPP::CtrlEvent>();
		dst = dev.sys.ev;
		out.seq = dev.sys.seq++;
		if (a.packet_router && !a.router_source_ports.IsEmpty() && fmt.IsValid()) {
			int credits = a.RequestCredits(src_ch, 1);
			if (credits <= 0) {
				//RTLOG("HoloSoftHMD::SinkDevice_Send: credit request denied for src_ch=" << src_ch);
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

bool HoloSoftHMD::SinkDevice_Recv(NativeSinkDevice& dev, AtomBase& a, int sink_ch, const Packet& in) {
	return true;
}

void HoloSoftHMD::SinkDevice_Finalize(NativeSinkDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg) {

}

bool HoloSoftHMD::SinkDevice_IsReady(NativeSinkDevice& dev, AtomBase& a, PacketIO& io) {
	dev.sys.UpdateData();
	return true;
}





END_UPP_NAMESPACE
#endif