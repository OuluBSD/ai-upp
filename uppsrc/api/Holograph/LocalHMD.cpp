#include "Holograph.h"

#if defined flagLOCALHMD
#include <SoftHMD/SoftHMD.h>


NAMESPACE_UPP



struct HoloLocalHMD::NativeSinkDevice {
	Upp::HMD::System sys;
	TimeStop ts;
};


bool HoloLocalHMD::SinkDevice_Create(NativeSinkDevice*& dev) {
	dev = new NativeSinkDevice;
	return true;
}

void HoloLocalHMD::SinkDevice_Destroy(NativeSinkDevice*& dev) {
	delete dev;
}

void HoloLocalHMD::SinkDevice_Visit(NativeSinkDevice&, AtomBase&, Visitor& vis) {
	
}

bool HoloLocalHMD::SinkDevice_Initialize(NativeSinkDevice& dev, AtomBase& a, const WorldState& ws) {
	dev.sys.require_hmd = !ws.IsTrue(".device.optional.hmd", false);
	dev.sys.require_left = !ws.IsTrue(".device.optional.left", true);
	dev.sys.require_right = !ws.IsTrue(".device.optional.right", true);
	dev.sys.user_hmd_idx = ws.GetInt(".device.hmd.idx", dev.sys.require_hmd ? 0 : -1);
	dev.sys.user_ctrl_idx[0] = ws.GetInt(".device.left.idx", dev.sys.require_left ? 0 : -1);
	dev.sys.user_ctrl_idx[1] = ws.GetInt(".device.right.idx", dev.sys.require_right ? 0 : -1);
	dev.sys.verbose = ws.IsTrue(".verbose", false);
	
	if (!dev.sys.Initialise())
		return false;
		
	dev.ts.Reset();
	return true;
}

bool HoloLocalHMD::SinkDevice_PostInitialize(NativeSinkDevice& dev, AtomBase& a) {
	
	return true;
}

bool HoloLocalHMD::SinkDevice_Start(NativeSinkDevice& dev, AtomBase& a) {
	
	return true;
}

void HoloLocalHMD::SinkDevice_Stop(NativeSinkDevice& dev, AtomBase& a) {
	
}

void HoloLocalHMD::SinkDevice_Uninitialize(NativeSinkDevice& dev, AtomBase& a) {
	dev.sys.Uninitialise();
}

bool HoloLocalHMD::SinkDevice_Send(NativeSinkDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	ASSERT(dev.sys.ev_sendable);
	if (!dev.sys.ev_sendable)
		return false;
	
	ValueFormat fmt = out.GetFormat();
	RTLOG("HoloLocalHMD::SinkDevice_Send: " << fmt.ToString());
	
	if (fmt.IsEvent()) {
		UPP::CtrlEvent& dst = out.SetData<UPP::CtrlEvent>();
		dst = dev.sys.ev;
		out.seq = dev.sys.seq++;
		if (a.packet_router && !a.router_source_ports.IsEmpty() && fmt.IsValid()) {
			int credits = a.RequestCredits(src_ch, 1);
			if (credits <= 0) {
				RTLOG("HoloLocalHMD::SinkDevice_Send: credit request denied for src_ch=" << src_ch);
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

bool HoloLocalHMD::SinkDevice_Recv(NativeSinkDevice& dev, AtomBase& a, int sink_ch, const Packet& in) {
	return true;
}

void HoloLocalHMD::SinkDevice_Finalize(NativeSinkDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg) {
	
}

bool HoloLocalHMD::SinkDevice_IsReady(NativeSinkDevice& dev, AtomBase& a, PacketIO& io) {
	dev.sys.UpdateData();
	return true;
}





END_UPP_NAMESPACE
#endif
