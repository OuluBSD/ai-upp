// This file have been generated automatically.
// DO NOT MODIFY THIS FILE!

#ifndef _IHolograph_IHolograph_h_
#define _IHolograph_IHolograph_h_

#include <Eon/Eon.h>
#include <api/Media/Media.h>
#include <plugin/hcidump/hcidump.h>
#include <SoftHMD/SoftHMD.h>

NAMESPACE_UPP

#define HOLO_CLS_LIST(x) \
	HOLO_CLS(SinkDevice, x) \

#define HOLO_VNDR_LIST \
	HOLO_VNDR(HoloOpenHMD) \
	HOLO_VNDR(HoloLocalHMD) \
	HOLO_VNDR(HoloRemoteVRServer) \
	HOLO_VNDR(HoloDevUsb) \
	HOLO_VNDR(HoloDevBluetooth) \
	HOLO_VNDR(HoloOpenVR) \

#define HOLO_CLS(x, v) struct v##x;
#define HOLO_VNDR(x) HOLO_CLS_LIST(x)
HOLO_VNDR_LIST
#undef HOLO_VNDR
#undef HOLO_CLS

#if (defined flagLINUX && defined flagOPENHMD) || (defined flagFREEBSD && defined flagOPENHMD)
struct HoloOpenHMD {
	#if defined flagVR
	struct NativeSinkDevice;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
#if defined flagLOCALHMD
struct HoloLocalHMD {
	#if defined flagVR
	struct NativeSinkDevice;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
#if (defined flagLINUX) || (defined flagFREEBSD)
struct HoloRemoteVRServer {
	#if defined flagVR
	struct NativeSinkDevice;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
#if (defined flagLINUX) || (defined flagFREEBSD)
struct HoloDevUsb {
	#if defined flagVR
	struct NativeSinkDevice;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
#if (defined flagLINUX && defined flagHACK) || (defined flagFREEBSD && defined flagHACK)
struct HoloDevBluetooth {
	#if defined flagVR
	struct NativeSinkDevice;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
#if (defined flagWIN32 && defined flagOPENVR)
struct HoloOpenVR {
	#if defined flagVR
	struct NativeSinkDevice;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif

#if defined flagVR
struct HoloSinkDevice : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~HoloSinkDevice() {}
};
#endif


#if defined flagVR
template <class Holo> struct HolographSinkDeviceT : HoloSinkDevice {
	using CLASSNAME = HolographSinkDeviceT<Holo>;
	using HoloSinkDevice::HoloSinkDevice;
	void Visit(Vis& v) override {
		if (dev) Holo::SinkDevice_Visit(*dev, *this, v);
		VIS_THIS(HoloSinkDevice);
	}
	typename Holo::NativeSinkDevice* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Holo::SinkDevice_Create(dev))
			return false;
		if (!Holo::SinkDevice_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Holo::SinkDevice_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Holo::SinkDevice_Start(*dev, *this);
	}
	void Stop() override {
		Holo::SinkDevice_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Holo::SinkDevice_Uninitialize(*dev, *this);
		Holo::SinkDevice_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Holo::SinkDevice_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool Recv(int sink_ch, const Packet& in) override {
		return Holo::SinkDevice_Recv(*dev, *this, sink_ch, in);
	}
	void Finalize(RealtimeSourceConfig& cfg) override {
		return Holo::SinkDevice_Finalize(*dev, *this, cfg);
	}
	bool IsReady(PacketIO& io) override {
		return Holo::SinkDevice_IsReady(*dev, *this, io);
	}
};
#endif

#if (defined flagLINUX && defined flagOPENHMD) || (defined flagFREEBSD && defined flagOPENHMD)
#if defined flagVR
using OpenHMDSinkDevice = HolographSinkDeviceT<HoloOpenHMD>;
#endif
#endif
#if defined flagLOCALHMD
#if defined flagVR
using LocalHMDSinkDevice = HolographSinkDeviceT<HoloLocalHMD>;
#endif
#endif
#if (defined flagLINUX) || (defined flagFREEBSD)
#if defined flagVR
using RemoteVRServerSinkDevice = HolographSinkDeviceT<HoloRemoteVRServer>;
#endif
#endif
#if (defined flagLINUX) || (defined flagFREEBSD)
#if defined flagVR
using DevUsbSinkDevice = HolographSinkDeviceT<HoloDevUsb>;
#endif
#endif
#if (defined flagLINUX && defined flagHACK) || (defined flagFREEBSD && defined flagHACK)
#if defined flagVR
using DevBluetoothSinkDevice = HolographSinkDeviceT<HoloDevBluetooth>;
#endif
#endif
#if (defined flagWIN32 && defined flagOPENVR)
#if defined flagVR
using OpenVRSinkDevice = HolographSinkDeviceT<HoloOpenVR>;
#endif
#endif

END_UPP_NAMESPACE

#endif
