// This file have been generated automatically.
// DO NOT MODIFY THIS FILE!

#ifndef _ISynth_ISynth_h_
#define _ISynth_ISynth_h_

#include <Eon/Eon.h>
#if defined flagSOFTINSTRU
#include <SoftInstru/SoftInstru.h>
#endif
#if defined flagSOFTSYNTH
#include <SoftSynth/SoftSynth.h>
#endif
#if defined flagSOFTAUDIO
#include <SoftAudio/SoftAudio.h>
#endif
#include <api/AudioHost/AudioHost.h>
#if defined flagFLUIDLITE
#include <plugin/fluidlite/fluidlite.h>
#endif
#if defined flagLV2
#include <plugin/lilv/lilv.h>
#endif

NAMESPACE_UPP

#define SYN_CLS_LIST(x) \
	SYN_CLS(Instrument, x) \

#define SYN_VNDR_LIST \
	SYN_VNDR(SynSoft) \
	SYN_VNDR(SynFluidsynth) \
	SYN_VNDR(SynFmSynth) \
	SYN_VNDR(SynCoreSynth) \
	SYN_VNDR(SynCoreDrummer) \
	SYN_VNDR(SynLV2) \

#define SYN_CLS(x, v) struct v##x;
#define SYN_VNDR(x) SYN_CLS_LIST(x)
SYN_VNDR_LIST
#undef SYN_VNDR
#undef SYN_CLS

#if defined flagSOFTINSTRU
struct SynSoft {
	#if (defined flagAUDIO && defined flagMIDI)
	struct NativeInstrument;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
#if (defined flagFLUIDSYNTH) || (defined flagFLUIDLITE)
struct SynFluidsynth {
	#if (defined flagAUDIO && defined flagMIDI)
	struct NativeInstrument;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
#if defined flagSOFTSYNTH
struct SynFmSynth {
	#if (defined flagAUDIO && defined flagMIDI)
	struct NativeInstrument;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
#if defined flagSOFTAUDIO
struct SynCoreSynth {
	#if (defined flagAUDIO && defined flagMIDI)
	struct NativeInstrument;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
#if defined flagSOFTAUDIO
struct SynCoreDrummer {
	#if (defined flagAUDIO && defined flagMIDI)
	struct NativeInstrument;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
#if defined flagLV2
struct SynLV2 {
	#if (defined flagAUDIO && defined flagMIDI)
	struct NativeInstrument;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif

#if (defined flagAUDIO && defined flagMIDI)
struct SynInstrument : public Atom {
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~SynInstrument() {}
};
#endif


#if (defined flagAUDIO && defined flagMIDI)
template <class Syn> struct SynthInstrumentT : SynInstrument {
	using CLASSNAME = SynthInstrumentT<Syn>;
	using SynInstrument::SynInstrument;
	void Visit(Vis& v) override {
		if (dev) Syn::Instrument_Visit(*dev, *this, v);
		VIS_THIS(SynInstrument);
	}
	typename Syn::NativeInstrument* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Syn::Instrument_Create(dev))
			return false;
		if (!Syn::Instrument_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Syn::Instrument_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Syn::Instrument_Start(*dev, *this);
	}
	void Stop() override {
		Syn::Instrument_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Syn::Instrument_Uninitialize(*dev, *this);
		Syn::Instrument_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Syn::Instrument_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool Recv(int sink_ch, const Packet& in) override {
		return Syn::Instrument_Recv(*dev, *this, sink_ch, in);
	}
	void Finalize(RealtimeSourceConfig& cfg) override {
		return Syn::Instrument_Finalize(*dev, *this, cfg);
	}
	bool IsReady(PacketIO& io) override {
		return Syn::Instrument_IsReady(*dev, *this, io);
	}
	bool IsDebugSoundEnabled() const {
		return Syn::Instrument_IsDebugSoundEnabled(*dev, *this);
	}
	String GetDebugSoundOutput() const {
		return Syn::Instrument_GetDebugSoundOutput(*dev, *this);
	}
	int GetDebugSoundSeed() const {
		return Syn::Instrument_GetDebugSoundSeed(*dev, *this);
	}
	bool IsDebugPrintEnabled() const {
		return Syn::Instrument_IsDebugPrintEnabled(*dev, *this);
	}
};
#endif

#if defined flagSOFTINSTRU
#if (defined flagAUDIO && defined flagMIDI)
using SoftInstrument = SynthInstrumentT<SynSoft>;
#endif
#endif
#if (defined flagFLUIDSYNTH) || (defined flagFLUIDLITE)
#if (defined flagAUDIO && defined flagMIDI)
using FluidsynthInstrument = SynthInstrumentT<SynFluidsynth>;
#endif
#endif
#if defined flagSOFTSYNTH
#if (defined flagAUDIO && defined flagMIDI)
using FmSynthInstrument = SynthInstrumentT<SynFmSynth>;
#endif
#endif
#if defined flagSOFTAUDIO
#if (defined flagAUDIO && defined flagMIDI)
using CoreSynthInstrument = SynthInstrumentT<SynCoreSynth>;
#endif
#endif
#if defined flagSOFTAUDIO
#if (defined flagAUDIO && defined flagMIDI)
using CoreDrummerInstrument = SynthInstrumentT<SynCoreDrummer>;
#endif
#endif
#if defined flagLV2
#if (defined flagAUDIO && defined flagMIDI)
using LV2Instrument = SynthInstrumentT<SynLV2>;
#endif
#endif

END_UPP_NAMESPACE

#endif
