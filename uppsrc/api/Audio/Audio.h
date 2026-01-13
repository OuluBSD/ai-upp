// This file have been generated automatically.
// DO NOT MODIFY THIS FILE!

#ifndef _IAudio_IAudio_h_
#define _IAudio_IAudio_h_

#include <Eon/Eon.h>
#include <Sound/Sound.h>

NAMESPACE_UPP

#define AUD_CLS_LIST(x) \
	AUD_CLS(SinkDevice, x) \
	AUD_CLS(SourceDevice, x) \

#define AUD_VNDR_LIST \
	AUD_VNDR(AudPortaudio) \

#define AUD_CLS(x, v) struct v##x;
#define AUD_VNDR(x) AUD_CLS_LIST(x)
AUD_VNDR_LIST
#undef HAL_VNDR
#undef HAL_CLS

#if (defined flagBUILTIN_PORTAUDIO) || (defined flagPORTAUDIO)
struct AudPortaudio {
	#if defined flagAUDIO
	struct NativeSinkDevice;
	#endif
	#if defined flagAUDIO
	struct NativeSourceDevice;
	#endif
	
	#if defined flagAUDIO
	static bool SinkDevice_Create(NativeSinkDevice*& dev);
	static void SinkDevice_Destroy(NativeSinkDevice*& dev);
	static bool SinkDevice_Initialize(NativeSinkDevice& dev_, AtomBase& a, const WorldState& ws);
	static bool SinkDevice_PostInitialize(NativeSinkDevice& dev, AtomBase& a);
	static bool SinkDevice_Start(NativeSinkDevice& dev, AtomBase&);
	static void SinkDevice_Stop(NativeSinkDevice& dev, AtomBase&);
	static void SinkDevice_Uninitialize(NativeSinkDevice& dev, AtomBase&);
	static bool SinkDevice_Send(NativeSinkDevice& dev, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
	static bool SinkDevice_NegotiateSinkFormat(NativeSinkDevice& dev, AtomBase& a, LinkBase& link, int sink_ch, const ValueFormat& new_fmt);
	static void SinkDevice_Visit(NativeSinkDevice&, AtomBase&, Visitor& vis);
	
	static bool SourceDevice_Create(NativeSourceDevice*& dev);
	static void SourceDevice_Destroy(NativeSourceDevice*& dev);
	static bool SourceDevice_Initialize(NativeSourceDevice& dev, AtomBase& a, const WorldState& ws);
	static bool SourceDevice_PostInitialize(NativeSourceDevice& dev, AtomBase& a);
	static bool SourceDevice_Start(NativeSourceDevice& dev, AtomBase&);
	static void SourceDevice_Stop(NativeSourceDevice& dev, AtomBase&);
	static void SourceDevice_Uninitialize(NativeSourceDevice& dev, AtomBase&);
	static bool SourceDevice_Send(NativeSourceDevice& dev, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
	static bool SourceDevice_NegotiateSinkFormat(NativeSourceDevice& dev, AtomBase& a, LinkBase& link, int sink_ch, const ValueFormat& new_fmt);
	static void SourceDevice_Visit(NativeSourceDevice&, AtomBase&, Visitor& vis);
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
};
#endif

#if defined flagAUDIO
struct AudSinkDevice : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~AudSinkDevice() {}
};
#endif

#if defined flagAUDIO
struct AudSourceDevice : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~AudSourceDevice() {}
};
#endif


#if defined flagAUDIO
template <class Aud> struct AudioSinkDeviceT : AudSinkDevice {
	using CLASSNAME = AudioSinkDeviceT<Aud>;
	using AudSinkDevice::AudSinkDevice;
	void Visit(Vis& v) override {
		if (dev) Aud::SinkDevice_Visit(*dev, *this, v);
		VIS_THIS(AudSinkDevice);
	}
	typename Aud::NativeSinkDevice* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Aud::SinkDevice_Create(dev))
			return false;
		if (!Aud::SinkDevice_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Aud::SinkDevice_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Aud::SinkDevice_Start(*dev, *this);
	}
	void Stop() override {
		Aud::SinkDevice_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Aud::SinkDevice_Uninitialize(*dev, *this);
		Aud::SinkDevice_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Aud::SinkDevice_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool NegotiateSinkFormat(LinkBase& link, int sink_ch, const ValueFormat& new_fmt) override {
		return Aud::SinkDevice_NegotiateSinkFormat(*dev, *this, link, sink_ch, new_fmt);
	}
};
#endif
#if defined flagAUDIO
template <class Aud> struct AudioSourceDeviceT : AudSourceDevice {
	using CLASSNAME = AudioSourceDeviceT<Aud>;
	using AudSourceDevice::AudSourceDevice;
	void Visit(Vis& v) override {
		if (dev) Aud::SourceDevice_Visit(*dev, *this, v);
		VIS_THIS(AudSourceDevice);
	}
	typename Aud::NativeSourceDevice* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Aud::SourceDevice_Create(dev))
			return false;
		if (!Aud::SourceDevice_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Aud::SourceDevice_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Aud::SourceDevice_Start(*dev, *this);
	}
	void Stop() override {
		Aud::SourceDevice_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Aud::SourceDevice_Uninitialize(*dev, *this);
		Aud::SourceDevice_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Aud::SourceDevice_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool NegotiateSinkFormat(LinkBase& link, int sink_ch, const ValueFormat& new_fmt) override {
		return Aud::SourceDevice_NegotiateSinkFormat(*dev, *this, link, sink_ch, new_fmt);
	}
};
#endif

#if (defined flagBUILTIN_PORTAUDIO) || (defined flagPORTAUDIO)
#if defined flagAUDIO
using PortaudioSinkDevice = AudioSinkDeviceT<AudPortaudio>;
#endif
#if defined flagAUDIO
using PortaudioSourceDevice = AudioSourceDeviceT<AudPortaudio>;
#endif
#endif

END_UPP_NAMESPACE

#endif