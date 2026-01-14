// This file have been generated automatically.
// DO NOT MODIFY THIS FILE!

#ifndef _IHal_IHal_h_
#define _IHal_IHal_h_

#include <Eon/Eon.h>
#include <api/Graphics/Graphics.h>

NAMESPACE_UPP

#define HAL_CLS_LIST(x) \
	HAL_CLS(AudioSinkDevice, x) \
	HAL_CLS(CenterScreenSinkDevice, x) \
	HAL_CLS(CenterFboSinkDevice, x) \
	HAL_CLS(OglScreenSinkDevice, x) \
	HAL_CLS(D12ScreenSinkDevice, x) \
	HAL_CLS(ContextBase, x) \
	HAL_CLS(EventsBase, x) \
	HAL_CLS(GuiSinkBase, x) \
	HAL_CLS(GuiFileSrc, x) \

#define HAL_VNDR_LIST \
	HAL_VNDR(HalUpp) \
	HAL_VNDR(HalSdl) \
	HAL_VNDR(HalHolo) \

#define HAL_CLS(x, v) struct v##x;
#define HAL_VNDR(x) HAL_CLS_LIST(x)
HAL_VNDR_LIST
#undef HAL_VNDR
#undef HAL_CLS

#if defined flagGUI
struct HalUpp {
	#if (defined flagHAL && defined flagAUDIO)
	struct NativeAudioSinkDevice;
	#endif
	#if (defined flagHAL && defined flagSCREEN)
	struct NativeCenterScreenSinkDevice;
	#endif
	#if (defined flagHAL && defined flagFBO && defined flagSCREEN)
	struct NativeCenterFboSinkDevice;
	#endif
	#if (defined flagHAL && defined flagOGL && defined flagSCREEN)
	struct NativeOglScreenSinkDevice;
	#endif
	#if (defined flagHAL && defined flagDX12 && defined flagSCREEN)
	struct NativeD12ScreenSinkDevice;
	#endif
	#if defined flagHAL
	struct NativeContextBase;
	#endif
	#if defined flagHAL
	struct NativeEventsBase;
	#endif
	#if defined flagHAL
	struct NativeGuiSinkBase;
	#endif
	#if defined flagHAL
	struct NativeGuiFileSrc;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
#if defined flagSDL2
struct HalSdl {
	#if (defined flagHAL && defined flagAUDIO)
	struct NativeAudioSinkDevice;
	#endif
	#if (defined flagHAL && defined flagSCREEN)
	struct NativeCenterScreenSinkDevice;
	#endif
	#if (defined flagHAL && defined flagFBO && defined flagSCREEN)
	struct NativeCenterFboSinkDevice;
	#endif
	#if (defined flagHAL && defined flagOGL && defined flagSCREEN)
	struct NativeOglScreenSinkDevice;
	#endif
	#if (defined flagHAL && defined flagDX12 && defined flagSCREEN)
	struct NativeD12ScreenSinkDevice;
	#endif
	#if defined flagHAL
	struct NativeContextBase;
	#endif
	#if defined flagHAL
	struct NativeEventsBase;
	#endif
	#if defined flagHAL
	struct NativeGuiSinkBase;
	#endif
	#if defined flagHAL
	struct NativeGuiFileSrc;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
#if (defined flagUWP && defined flagDX12)
struct HalHolo {
	#if (defined flagHAL && defined flagAUDIO)
	struct NativeAudioSinkDevice;
	#endif
	#if (defined flagHAL && defined flagSCREEN)
	struct NativeCenterScreenSinkDevice;
	#endif
	#if (defined flagHAL && defined flagFBO && defined flagSCREEN)
	struct NativeCenterFboSinkDevice;
	#endif
	#if (defined flagHAL && defined flagOGL && defined flagSCREEN)
	struct NativeOglScreenSinkDevice;
	#endif
	#if (defined flagHAL && defined flagDX12 && defined flagSCREEN)
	struct NativeD12ScreenSinkDevice;
	#endif
	#if defined flagHAL
	struct NativeContextBase;
	#endif
	#if defined flagHAL
	struct NativeEventsBase;
	#endif
	#if defined flagHAL
	struct NativeGuiSinkBase;
	#endif
	#if defined flagHAL
	struct NativeGuiFileSrc;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif

#if (defined flagHAL && defined flagAUDIO)
struct HalAudioSinkDevice : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~HalAudioSinkDevice() {}
};
#endif

#if (defined flagHAL && defined flagSCREEN)
struct HalCenterScreenSinkDevice : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~HalCenterScreenSinkDevice() {}
};
#endif

#if (defined flagHAL && defined flagFBO && defined flagSCREEN)
struct HalCenterFboSinkDevice : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~HalCenterFboSinkDevice() {}
};
#endif

#if (defined flagHAL && defined flagOGL && defined flagSCREEN)
struct HalOglScreenSinkDevice : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~HalOglScreenSinkDevice() {}
};
#endif

#if (defined flagHAL && defined flagDX12 && defined flagSCREEN)
struct HalD12ScreenSinkDevice : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~HalD12ScreenSinkDevice() {}
};
#endif

#if defined flagHAL
struct HalContextBase : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~HalContextBase() {}
};
#endif

#if defined flagHAL
struct HalEventsBase : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~HalEventsBase() {}
};
#endif

#if defined flagHAL
struct HalGuiSinkBase : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~HalGuiSinkBase() {}
};
#endif

#if defined flagHAL
struct HalGuiFileSrc : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~HalGuiFileSrc() {}
};
#endif


#if (defined flagHAL && defined flagAUDIO)
template <class Hal> struct HalAudioSinkDeviceT : HalAudioSinkDevice {
	using CLASSNAME = HalAudioSinkDeviceT<Hal>;
	using HalAudioSinkDevice::HalAudioSinkDevice;
	void Visit(Vis& v) override {
		if (dev) Hal::AudioSinkDevice_Visit(*dev, *this, v);
		VIS_THIS(HalAudioSinkDevice);
	}
	typename Hal::NativeAudioSinkDevice* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Hal::AudioSinkDevice_Create(dev))
			return false;
		if (!Hal::AudioSinkDevice_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Hal::AudioSinkDevice_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Hal::AudioSinkDevice_Start(*dev, *this);
	}
	void Stop() override {
		Hal::AudioSinkDevice_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Hal::AudioSinkDevice_Uninitialize(*dev, *this);
		Hal::AudioSinkDevice_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Hal::AudioSinkDevice_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool Recv(int sink_ch, const Packet& in) override {
		return Hal::AudioSinkDevice_Recv(*dev, *this, sink_ch, in);
	}
	void Finalize(RealtimeSourceConfig& cfg) override {
		return Hal::AudioSinkDevice_Finalize(*dev, *this, cfg);
	}
	void Update(double dt) override {
		return Hal::AudioSinkDevice_Update(*dev, *this, dt);
	}
	bool IsReady(PacketIO& io) override {
		return Hal::AudioSinkDevice_IsReady(*dev, *this, io);
	}
	bool AttachContext(AtomBase& a) override {
		return Hal::AudioSinkDevice_AttachContext(*dev, *this, a);
	}
	void DetachContext(AtomBase& a) override {
		Hal::AudioSinkDevice_DetachContext(*dev, *this, a);
	}
};
#endif
#if (defined flagHAL && defined flagSCREEN)
template <class Hal> struct HalCenterScreenSinkDeviceT : HalCenterScreenSinkDevice {
	using CLASSNAME = HalCenterScreenSinkDeviceT<Hal>;
	using HalCenterScreenSinkDevice::HalCenterScreenSinkDevice;
	void Visit(Vis& v) override {
		if (dev) Hal::CenterScreenSinkDevice_Visit(*dev, *this, v);
		VIS_THIS(HalCenterScreenSinkDevice);
	}
	typename Hal::NativeCenterScreenSinkDevice* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Hal::CenterScreenSinkDevice_Create(dev))
			return false;
		if (!Hal::CenterScreenSinkDevice_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Hal::CenterScreenSinkDevice_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Hal::CenterScreenSinkDevice_Start(*dev, *this);
	}
	void Stop() override {
		Hal::CenterScreenSinkDevice_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Hal::CenterScreenSinkDevice_Uninitialize(*dev, *this);
		Hal::CenterScreenSinkDevice_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Hal::CenterScreenSinkDevice_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool Recv(int sink_ch, const Packet& in) override {
		return Hal::CenterScreenSinkDevice_Recv(*dev, *this, sink_ch, in);
	}
	void Finalize(RealtimeSourceConfig& cfg) override {
		return Hal::CenterScreenSinkDevice_Finalize(*dev, *this, cfg);
	}
	void Update(double dt) override {
		return Hal::CenterScreenSinkDevice_Update(*dev, *this, dt);
	}
	bool IsReady(PacketIO& io) override {
		return Hal::CenterScreenSinkDevice_IsReady(*dev, *this, io);
	}
	bool AttachContext(AtomBase& a) override {
		return Hal::CenterScreenSinkDevice_AttachContext(*dev, *this, a);
	}
	void DetachContext(AtomBase& a) override {
		Hal::CenterScreenSinkDevice_DetachContext(*dev, *this, a);
	}
};
#endif
#if (defined flagHAL && defined flagFBO && defined flagSCREEN)
template <class Hal> struct HalCenterFboSinkDeviceT : HalCenterFboSinkDevice {
	using CLASSNAME = HalCenterFboSinkDeviceT<Hal>;
	using HalCenterFboSinkDevice::HalCenterFboSinkDevice;
	void Visit(Vis& v) override {
		if (dev) Hal::CenterFboSinkDevice_Visit(*dev, *this, v);
		VIS_THIS(HalCenterFboSinkDevice);
	}
	typename Hal::NativeCenterFboSinkDevice* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Hal::CenterFboSinkDevice_Create(dev))
			return false;
		if (!Hal::CenterFboSinkDevice_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Hal::CenterFboSinkDevice_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Hal::CenterFboSinkDevice_Start(*dev, *this);
	}
	void Stop() override {
		Hal::CenterFboSinkDevice_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Hal::CenterFboSinkDevice_Uninitialize(*dev, *this);
		Hal::CenterFboSinkDevice_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Hal::CenterFboSinkDevice_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool Recv(int sink_ch, const Packet& in) override {
		return Hal::CenterFboSinkDevice_Recv(*dev, *this, sink_ch, in);
	}
	void Finalize(RealtimeSourceConfig& cfg) override {
		return Hal::CenterFboSinkDevice_Finalize(*dev, *this, cfg);
	}
	void Update(double dt) override {
		return Hal::CenterFboSinkDevice_Update(*dev, *this, dt);
	}
	bool IsReady(PacketIO& io) override {
		return Hal::CenterFboSinkDevice_IsReady(*dev, *this, io);
	}
	bool AttachContext(AtomBase& a) override {
		return Hal::CenterFboSinkDevice_AttachContext(*dev, *this, a);
	}
	void DetachContext(AtomBase& a) override {
		Hal::CenterFboSinkDevice_DetachContext(*dev, *this, a);
	}
};
#endif
#if (defined flagHAL && defined flagOGL && defined flagSCREEN)
template <class Hal> struct HalOglScreenSinkDeviceT : HalOglScreenSinkDevice {
	using CLASSNAME = HalOglScreenSinkDeviceT<Hal>;
	using HalOglScreenSinkDevice::HalOglScreenSinkDevice;
	void Visit(Vis& v) override {
		if (dev) Hal::OglScreenSinkDevice_Visit(*dev, *this, v);
		VIS_THIS(HalOglScreenSinkDevice);
	}
	typename Hal::NativeOglScreenSinkDevice* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Hal::OglScreenSinkDevice_Create(dev))
			return false;
		if (!Hal::OglScreenSinkDevice_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Hal::OglScreenSinkDevice_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Hal::OglScreenSinkDevice_Start(*dev, *this);
	}
	void Stop() override {
		Hal::OglScreenSinkDevice_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Hal::OglScreenSinkDevice_Uninitialize(*dev, *this);
		Hal::OglScreenSinkDevice_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Hal::OglScreenSinkDevice_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool Recv(int sink_ch, const Packet& in) override {
		return Hal::OglScreenSinkDevice_Recv(*dev, *this, sink_ch, in);
	}
	void Finalize(RealtimeSourceConfig& cfg) override {
		return Hal::OglScreenSinkDevice_Finalize(*dev, *this, cfg);
	}
	void Update(double dt) override {
		return Hal::OglScreenSinkDevice_Update(*dev, *this, dt);
	}
	bool IsReady(PacketIO& io) override {
		return Hal::OglScreenSinkDevice_IsReady(*dev, *this, io);
	}
	bool AttachContext(AtomBase& a) override {
		return Hal::OglScreenSinkDevice_AttachContext(*dev, *this, a);
	}
	void DetachContext(AtomBase& a) override {
		Hal::OglScreenSinkDevice_DetachContext(*dev, *this, a);
	}
};
#endif
#if (defined flagHAL && defined flagDX12 && defined flagSCREEN)
template <class Hal> struct HalD12ScreenSinkDeviceT : HalD12ScreenSinkDevice {
	using CLASSNAME = HalD12ScreenSinkDeviceT<Hal>;
	using HalD12ScreenSinkDevice::HalD12ScreenSinkDevice;
	void Visit(Vis& v) override {
		if (dev) Hal::D12ScreenSinkDevice_Visit(*dev, *this, v);
		VIS_THIS(HalD12ScreenSinkDevice);
	}
	typename Hal::NativeD12ScreenSinkDevice* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Hal::D12ScreenSinkDevice_Create(dev))
			return false;
		if (!Hal::D12ScreenSinkDevice_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Hal::D12ScreenSinkDevice_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Hal::D12ScreenSinkDevice_Start(*dev, *this);
	}
	void Stop() override {
		Hal::D12ScreenSinkDevice_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Hal::D12ScreenSinkDevice_Uninitialize(*dev, *this);
		Hal::D12ScreenSinkDevice_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Hal::D12ScreenSinkDevice_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool Recv(int sink_ch, const Packet& in) override {
		return Hal::D12ScreenSinkDevice_Recv(*dev, *this, sink_ch, in);
	}
	void Finalize(RealtimeSourceConfig& cfg) override {
		return Hal::D12ScreenSinkDevice_Finalize(*dev, *this, cfg);
	}
	void Update(double dt) override {
		return Hal::D12ScreenSinkDevice_Update(*dev, *this, dt);
	}
	bool IsReady(PacketIO& io) override {
		return Hal::D12ScreenSinkDevice_IsReady(*dev, *this, io);
	}
	bool AttachContext(AtomBase& a) override {
		return Hal::D12ScreenSinkDevice_AttachContext(*dev, *this, a);
	}
	void DetachContext(AtomBase& a) override {
		Hal::D12ScreenSinkDevice_DetachContext(*dev, *this, a);
	}
};
#endif
#if defined flagHAL
template <class Hal> struct HalContextBaseT : HalContextBase {
	using CLASSNAME = HalContextBaseT<Hal>;
	using HalContextBase::HalContextBase;
	void Visit(Vis& v) override {
		if (dev) Hal::ContextBase_Visit(*dev, *this, v);
		VIS_THIS(HalContextBase);
	}
	typename Hal::NativeContextBase* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Hal::ContextBase_Create(dev))
			return false;
		if (!Hal::ContextBase_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Hal::ContextBase_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Hal::ContextBase_Start(*dev, *this);
	}
	void Stop() override {
		Hal::ContextBase_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Hal::ContextBase_Uninitialize(*dev, *this);
		Hal::ContextBase_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Hal::ContextBase_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool Recv(int sink_ch, const Packet& in) override {
		return Hal::ContextBase_Recv(*dev, *this, sink_ch, in);
	}
	void Finalize(RealtimeSourceConfig& cfg) override {
		return Hal::ContextBase_Finalize(*dev, *this, cfg);
	}
	void Update(double dt) override {
		return Hal::ContextBase_Update(*dev, *this, dt);
	}
	bool IsReady(PacketIO& io) override {
		return Hal::ContextBase_IsReady(*dev, *this, io);
	}
	bool AttachContext(AtomBase& a) override {
		return Hal::ContextBase_AttachContext(*dev, *this, a);
	}
	void DetachContext(AtomBase& a) override {
		Hal::ContextBase_DetachContext(*dev, *this, a);
	}
};
#endif
#if defined flagHAL
template <class Hal> struct HalEventsBaseT : HalEventsBase {
	using CLASSNAME = HalEventsBaseT<Hal>;
	using HalEventsBase::HalEventsBase;
	void Visit(Vis& v) override {
		if (dev) Hal::EventsBase_Visit(*dev, *this, v);
		VIS_THIS(HalEventsBase);
	}
	typename Hal::NativeEventsBase* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Hal::EventsBase_Create(dev))
			return false;
		if (!Hal::EventsBase_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Hal::EventsBase_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Hal::EventsBase_Start(*dev, *this);
	}
	void Stop() override {
		Hal::EventsBase_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Hal::EventsBase_Uninitialize(*dev, *this);
		Hal::EventsBase_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Hal::EventsBase_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool Recv(int sink_ch, const Packet& in) override {
		return Hal::EventsBase_Recv(*dev, *this, sink_ch, in);
	}
	void Finalize(RealtimeSourceConfig& cfg) override {
		return Hal::EventsBase_Finalize(*dev, *this, cfg);
	}
	void Update(double dt) override {
		return Hal::EventsBase_Update(*dev, *this, dt);
	}
	bool IsReady(PacketIO& io) override {
		return Hal::EventsBase_IsReady(*dev, *this, io);
	}
	bool AttachContext(AtomBase& a) override {
		return Hal::EventsBase_AttachContext(*dev, *this, a);
	}
	void DetachContext(AtomBase& a) override {
		Hal::EventsBase_DetachContext(*dev, *this, a);
	}
};
#endif
#if defined flagHAL
template <class Hal> struct HalGuiSinkBaseT : HalGuiSinkBase {
	using CLASSNAME = HalGuiSinkBaseT<Hal>;
	using HalGuiSinkBase::HalGuiSinkBase;
	void Visit(Vis& v) override {
		if (dev) Hal::GuiSinkBase_Visit(*dev, *this, v);
		VIS_THIS(HalGuiSinkBase);
	}
	typename Hal::NativeGuiSinkBase* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Hal::GuiSinkBase_Create(dev))
			return false;
		if (!Hal::GuiSinkBase_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Hal::GuiSinkBase_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Hal::GuiSinkBase_Start(*dev, *this);
	}
	void Stop() override {
		Hal::GuiSinkBase_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Hal::GuiSinkBase_Uninitialize(*dev, *this);
		Hal::GuiSinkBase_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Hal::GuiSinkBase_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool Recv(int sink_ch, const Packet& in) override {
		return Hal::GuiSinkBase_Recv(*dev, *this, sink_ch, in);
	}
	void Finalize(RealtimeSourceConfig& cfg) override {
		return Hal::GuiSinkBase_Finalize(*dev, *this, cfg);
	}
	void Update(double dt) override {
		return Hal::GuiSinkBase_Update(*dev, *this, dt);
	}
	bool IsReady(PacketIO& io) override {
		return Hal::GuiSinkBase_IsReady(*dev, *this, io);
	}
	bool AttachContext(AtomBase& a) override {
		return Hal::GuiSinkBase_AttachContext(*dev, *this, a);
	}
	void DetachContext(AtomBase& a) override {
		Hal::GuiSinkBase_DetachContext(*dev, *this, a);
	}
};
#endif
#if defined flagHAL
template <class Hal> struct HalGuiFileSrcT : HalGuiFileSrc {
	using CLASSNAME = HalGuiFileSrcT<Hal>;
	using HalGuiFileSrc::HalGuiFileSrc;
	void Visit(Vis& v) override {
		if (dev) Hal::GuiFileSrc_Visit(*dev, *this, v);
		VIS_THIS(HalGuiFileSrc);
	}
	typename Hal::NativeGuiFileSrc* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Hal::GuiFileSrc_Create(dev))
			return false;
		if (!Hal::GuiFileSrc_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Hal::GuiFileSrc_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Hal::GuiFileSrc_Start(*dev, *this);
	}
	void Stop() override {
		Hal::GuiFileSrc_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Hal::GuiFileSrc_Uninitialize(*dev, *this);
		Hal::GuiFileSrc_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Hal::GuiFileSrc_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool Recv(int sink_ch, const Packet& in) override {
		return Hal::GuiFileSrc_Recv(*dev, *this, sink_ch, in);
	}
	void Finalize(RealtimeSourceConfig& cfg) override {
		return Hal::GuiFileSrc_Finalize(*dev, *this, cfg);
	}
	void Update(double dt) override {
		return Hal::GuiFileSrc_Update(*dev, *this, dt);
	}
	bool IsReady(PacketIO& io) override {
		return Hal::GuiFileSrc_IsReady(*dev, *this, io);
	}
	bool AttachContext(AtomBase& a) override {
		return Hal::GuiFileSrc_AttachContext(*dev, *this, a);
	}
	void DetachContext(AtomBase& a) override {
		Hal::GuiFileSrc_DetachContext(*dev, *this, a);
	}
};
#endif

#if defined flagGUI
#if (defined flagHAL && defined flagAUDIO)
using UppAudioSinkDevice = HalAudioSinkDeviceT<HalUpp>;
#endif
#if (defined flagHAL && defined flagSCREEN)
using UppCenterScreenSinkDevice = HalCenterScreenSinkDeviceT<HalUpp>;
#endif
#if (defined flagHAL && defined flagFBO && defined flagSCREEN)
using UppCenterFboSinkDevice = HalCenterFboSinkDeviceT<HalUpp>;
#endif
#if (defined flagHAL && defined flagOGL && defined flagSCREEN)
using UppOglScreenSinkDevice = HalOglScreenSinkDeviceT<HalUpp>;
#endif
#if (defined flagHAL && defined flagDX12 && defined flagSCREEN)
using UppD12ScreenSinkDevice = HalD12ScreenSinkDeviceT<HalUpp>;
#endif
#if defined flagHAL
using UppContextBase = HalContextBaseT<HalUpp>;
#endif
#if defined flagHAL
using UppEventsBase = HalEventsBaseT<HalUpp>;
#endif
#if defined flagHAL
using UppGuiSinkBase = HalGuiSinkBaseT<HalUpp>;
#endif
#if defined flagHAL
using UppGuiFileSrc = HalGuiFileSrcT<HalUpp>;
#endif
#endif
#if defined flagSDL2
#if (defined flagHAL && defined flagAUDIO)
using SdlAudioSinkDevice = HalAudioSinkDeviceT<HalSdl>;
#endif
#if (defined flagHAL && defined flagSCREEN)
using SdlCenterScreenSinkDevice = HalCenterScreenSinkDeviceT<HalSdl>;
#endif
#if (defined flagHAL && defined flagFBO && defined flagSCREEN)
using SdlCenterFboSinkDevice = HalCenterFboSinkDeviceT<HalSdl>;
#endif
#if (defined flagHAL && defined flagOGL && defined flagSCREEN)
using SdlOglScreenSinkDevice = HalOglScreenSinkDeviceT<HalSdl>;
#endif
#if (defined flagHAL && defined flagDX12 && defined flagSCREEN)
using SdlD12ScreenSinkDevice = HalD12ScreenSinkDeviceT<HalSdl>;
#endif
#if defined flagHAL
using SdlContextBase = HalContextBaseT<HalSdl>;
#endif
#if defined flagHAL
using SdlEventsBase = HalEventsBaseT<HalSdl>;
#endif
#if defined flagHAL
using SdlGuiSinkBase = HalGuiSinkBaseT<HalSdl>;
#endif
#if defined flagHAL
using SdlGuiFileSrc = HalGuiFileSrcT<HalSdl>;
#endif
#endif
#if (defined flagUWP && defined flagDX12)
#if (defined flagHAL && defined flagAUDIO)
using HoloAudioSinkDevice = HalAudioSinkDeviceT<HalHolo>;
#endif
#if (defined flagHAL && defined flagSCREEN)
using HoloCenterScreenSinkDevice = HalCenterScreenSinkDeviceT<HalHolo>;
#endif
#if (defined flagHAL && defined flagFBO && defined flagSCREEN)
using HoloCenterFboSinkDevice = HalCenterFboSinkDeviceT<HalHolo>;
#endif
#if (defined flagHAL && defined flagOGL && defined flagSCREEN)
using HoloOglScreenSinkDevice = HalOglScreenSinkDeviceT<HalHolo>;
#endif
#if (defined flagHAL && defined flagDX12 && defined flagSCREEN)
using HoloD12ScreenSinkDevice = HalD12ScreenSinkDeviceT<HalHolo>;
#endif
#if defined flagHAL
using HoloContextBase = HalContextBaseT<HalHolo>;
#endif
#if defined flagHAL
using HoloEventsBase = HalEventsBaseT<HalHolo>;
#endif
#if defined flagHAL
using HoloGuiSinkBase = HalGuiSinkBaseT<HalHolo>;
#endif
#if defined flagHAL
using HoloGuiFileSrc = HalGuiFileSrcT<HalHolo>;
#endif
#endif

END_UPP_NAMESPACE

#endif
