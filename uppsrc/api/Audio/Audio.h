// This file have been generated automatically.
// DO NOT MODIFY THIS FILE!

#ifndef _IAudio_IAudio_h_
#define _IAudio_IAudio_h_

#include <Eon/Eon.h>

#if !defined flagSYS_PORTAUDIO
#include <plugin/portaudio/portaudio.h>
#else
#include <portaudio.h>
#endif


NAMESPACE_UPP

#define AUD_CLS_LIST(x) \
	AUD_CLS(SinkDevice, x) \
	AUD_CLS(SourceDevice, x) \

#define AUD_VNDR_LIST \
	AUD_VNDR(AudPortaudio) \

#define AUD_CLS(x, v) struct v##x;
#define AUD_VNDR(x) AUD_CLS_LIST(x)
AUD_VNDR_LIST
#undef AUD_VNDR
#undef AUD_CLS

#if (!defined flagSYS_PORTAUDIO) || (defined flagPORTAUDIO)
struct AudPortaudio {
	struct NativeSinkDevice;
	struct NativeSourceDevice;
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif

struct AudSinkDevice : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~AudSinkDevice() {}
};

struct AudSourceDevice : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~AudSourceDevice() {}
};


template <class Aud> struct AudioSinkDeviceT : AudSinkDevice {
	AudioSinkDeviceT(VfsValue& n) : AudSinkDevice(n) {}
	using CLASSNAME = AudioSinkDeviceT<Aud>;
	TypeCls GetTypeCls() const override {return typeid(CLASSNAME);}
	void Visit(Vis& v) override {
		if (dev) Aud::SinkDevice_Visit(*dev, *this, v);
		VIS_THIS(AudSinkDevice);
	}
	typename Aud::NativeSinkDevice* dev = 0;
	bool Initialize(const WorldState& ws) override {
		debug_sound_output = ws.Get(".debug_sound_output", "");
		debug_sound_seed = ws.GetInt(".debug_sound_seed", 0);
		debug_sound_enabled = false;
		debug_print_enabled = false;
		String trimmed = TrimBoth(debug_sound_output);
		if (trimmed.IsEmpty()) {
			debug_sound_output.Clear();
		}
		else {
			String lowered = ToLower(trimmed);
			if (lowered == "0" || lowered == "false" || lowered == "off" || lowered == "no" || lowered == "none") {
				debug_sound_output.Clear();
			}
			else {
				debug_sound_enabled = true;
				if (lowered == "1" || lowered == "true" || lowered == "on" || lowered == "yes")
					debug_sound_output.Clear();
				else
					debug_sound_output = trimmed;
			}
		}
		if (!debug_sound_enabled)
			debug_sound_output.Clear();
		if (debug_sound_enabled && debug_sound_output.IsEmpty())
			debug_sound_output = "on";
		debug_print_enabled = debug_sound_enabled;
		String debug_print_token = TrimBoth(ws.Get(".debug_print", ""));
		if (!debug_print_token.IsEmpty()) {
			String lowered = ToLower(debug_print_token);
			if (lowered == "0" || lowered == "false" || lowered == "off" || lowered == "no" || lowered == "none")
				debug_print_enabled = false;
			else
				debug_print_enabled = true;
		}
		if (!Aud::SinkDevice_Create(dev))
			return false;
		if (!dev)
			return false;
		if (!Aud::SinkDevice_Initialize(*dev, *this, ws)) {
			Aud::SinkDevice_Destroy(dev);
			dev = nullptr;
			return false;
		}
		return true;
	}
	bool PostInitialize() override {
		if (!dev)
			return false;
		if (!Aud::SinkDevice_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		if (!dev)
			return false;
		return Aud::SinkDevice_Start(*dev, *this);
	}
	void Stop() override {
		if (!dev)
			return;
		Aud::SinkDevice_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		if (!dev)
			return;
		Aud::SinkDevice_Uninitialize(*dev, *this);
		Aud::SinkDevice_Destroy(dev);
		dev = nullptr;
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!dev)
			return false;
		if (!Aud::SinkDevice_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool NegotiateSinkFormat(LinkBase& link, int sink_ch, const ValueFormat& new_fmt) override {
		if (!dev)
			return false;
		return Aud::SinkDevice_NegotiateSinkFormat(*dev, *this, link, sink_ch, new_fmt);
	}
	const String& GetDebugSoundOutput() const { return debug_sound_output; }
	int GetDebugSoundSeed() const { return debug_sound_seed; }
	bool IsDebugSoundEnabled() const { return debug_sound_enabled; }
	bool IsDebugPrintEnabled() const { return debug_print_enabled; }
protected:
	String debug_sound_output;
	int debug_sound_seed = 0;
	bool debug_sound_enabled = false;
	bool debug_print_enabled = false;
};
template <class Aud> struct AudioSourceDeviceT : AudSourceDevice {
	using CLASSNAME = AudioSourceDeviceT<Aud>;
	//RTTI_DECL1(CLASSNAME, AudSourceDevice)
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

#if (!defined flagSYS_PORTAUDIO) || (defined flagPORTAUDIO)
using PortaudioSinkDevice = AudioSinkDeviceT<AudPortaudio>;
using PortaudioSourceDevice = AudioSourceDeviceT<AudPortaudio>;
#endif

END_UPP_NAMESPACE

#endif
