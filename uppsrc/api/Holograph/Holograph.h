// This file have been generated automatically.
// DO NOT MODIFY THIS FILE!

#ifndef _IHolograph_IHolograph_h_
#define _IHolograph_IHolograph_h_

#include <Eon/Eon.h>
#include <api/Media/Media.h>
#if defined flagLINUX
#include <plugin/hcidump/hcidump.h>
#endif
#if defined flagSOFTHMD
#include <SoftHMD/SoftHMD.h>
#endif

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
	#if defined flagVR || defined flagOPENHMD || defined flagOPENVR
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
	#if defined flagVR || defined flagLOCALHMD || defined flagOPENVR
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
	#if defined flagVR || defined flagOPENVR
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
	#if defined flagVR || defined flagOPENVR
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
	#if defined flagVR || defined flagOPENVR
	struct NativeSinkDevice;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
#if (defined flagWIN32 && defined flagOPENVR) || (defined flagLINUX && defined flagOPENVR)
struct HoloOpenVR {
	#if defined flagVR || defined flagOPENVR
	struct NativeSinkDevice;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif

struct HoloSinkDevice : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~HoloSinkDevice() {}
};


template <class Holo> struct HolographSinkDeviceT : HoloSinkDevice {
	using CLASSNAME = HolographSinkDeviceT<Holo>;
	using HoloSinkDevice::HoloSinkDevice;
	void Visit(Vis& v) override {
		if (dev) Holo::SinkDevice_Visit(*dev, *this, v);
		VIS_THIS(HoloSinkDevice);
	}
	typename Holo::NativeSinkDevice* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Holo::SinkDevice_Create(dev)) {
			LOG("HolographSinkDeviceT::Initialize: SinkDevice_Create failed");
			return false;
		}
		if (!Holo::SinkDevice_Initialize(*dev, *this, ws)) {
			LOG("HolographSinkDeviceT::Initialize: SinkDevice_Initialize failed");
			return false;
		}
		LOG("HolographSinkDeviceT::Initialize: success");
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

#if (defined flagLINUX && defined flagOPENHMD) || (defined flagFREEBSD && defined flagOPENHMD)
using OpenHMDSinkDevice = HolographSinkDeviceT<HoloOpenVR>;
#endif
#if defined flagLOCALHMD
using LocalHMDSinkDevice = HolographSinkDeviceT<HoloOpenVR>;
#endif
#if (defined flagLINUX) || (defined flagFREEBSD)
using RemoteVRServerSinkDevice = HolographSinkDeviceT<HoloOpenVR>;
#endif
#if (defined flagLINUX) || (defined flagFREEBSD)
using DevUsbSinkDevice = HolographSinkDeviceT<HoloOpenVR>;
#endif
#if (defined flagLINUX && defined flagHACK) || (defined flagFREEBSD && defined flagHACK)
using DevBluetoothSinkDevice = HolographSinkDeviceT<HoloOpenVR>;
#endif
#if (defined flagWIN32 && defined flagOPENVR) || (defined flagLINUX && defined flagOPENVR)
using OpenVRSinkDevice = HolographSinkDeviceT<HoloOpenVR>;
#endif

END_UPP_NAMESPACE

#endif
