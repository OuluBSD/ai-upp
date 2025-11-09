// This file have been generated automatically.
// DO NOT MODIFY THIS FILE!

#ifndef _ISynth_ISynth_h_
#define _ISynth_ISynth_h_

#include <Eon/Eon.h>
#include <SoftInstru/SoftInstru.h>
#include <SoftSynth/SoftSynth.h>
#include <SoftAudio/SoftAudio.h>

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

struct SynSoft {
	struct NativeInstrument;
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#if (defined flagFLUIDSYNTH) || (defined flagFLUIDLITE)
struct SynFluidsynth {
	struct NativeInstrument;
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif
struct SynFmSynth {
	struct NativeInstrument;
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
struct SynCoreSynth {
	struct NativeInstrument;
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
struct SynCoreDrummer {
	struct NativeInstrument;
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#if defined flagLV2
struct SynLV2 {
	struct NativeInstrument;
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif

struct SynInstrument : public Atom {
	//RTTI_DECL1(SynInstrument, Atom)
	using Atom::Atom;
	void Visit(Vis& v) override {VIS_THIS(Atom);}
	
	virtual ~SynInstrument() {}
};


template <class Syn> struct SynthInstrumentT : SynInstrument {
	SynthInstrumentT(VfsValue& n) : SynInstrument(n) {}
	using CLASSNAME = SynthInstrumentT<Syn>;
	TypeCls GetTypeCls() const override {return typeid(CLASSNAME);}
	void Visit(Vis& v) override {
		if (dev) Syn::Instrument_Visit(*dev, *this, v);
		VIS_THIS(SynInstrument);
	}
	typename Syn::NativeInstrument* dev = 0;
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
		// default debug_print to follow debug_sound
		debug_print_enabled = debug_sound_enabled;
		String debug_print_token = TrimBoth(ws.Get(".debug_print", ""));
		if (!debug_print_token.IsEmpty()) {
			String lowered = ToLower(debug_print_token);
			if (lowered == "0" || lowered == "false" || lowered == "off" || lowered == "no" || lowered == "none")
				debug_print_enabled = false;
			else
				debug_print_enabled = true;
		}
		if (!Syn::Instrument_Create(dev))
			return false;
		if (!dev)
			return false;
		if (!Syn::Instrument_Initialize(*dev, *this, ws)) {
			Syn::Instrument_Destroy(dev);
			dev = nullptr;
			return false;
		}
		return true;
	}
	bool PostInitialize() override {
		if (!dev)
			return false;
		if (!Syn::Instrument_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		if (!dev)
			return false;
		return Syn::Instrument_Start(*dev, *this);
	}
	void Stop() override {
		if (!dev)
			return;
		Syn::Instrument_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		if (!dev)
			return;
		Syn::Instrument_Uninitialize(*dev, *this);
		Syn::Instrument_Destroy(dev);
		dev = nullptr;
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!dev)
			return false;
		if (!Syn::Instrument_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool Recv(int sink_ch, const Packet& in) override {
		if (!dev)
			return false;
		return Syn::Instrument_Recv(*dev, *this, sink_ch, in);
	}
	void Finalize(RealtimeSourceConfig& cfg) override {
		if (!dev)
			return;
		return Syn::Instrument_Finalize(*dev, *this, cfg);
	}
	bool IsReady(PacketIO& io) override {
		if (!dev)
			return false;
		return Syn::Instrument_IsReady(*dev, *this, io);
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

using SoftInstrument = SynthInstrumentT<SynSoft>;
#if (defined flagFLUIDSYNTH) || (defined flagFLUIDLITE)
using FluidsynthInstrument = SynthInstrumentT<SynFluidsynth>;
#endif
using FmSynthInstrument = SynthInstrumentT<SynFmSynth>;
using CoreSynthInstrument = SynthInstrumentT<SynCoreSynth>;
using CoreDrummerInstrument = SynthInstrumentT<SynCoreDrummer>;
#if defined flagLV2
using LV2Instrument = SynthInstrumentT<SynLV2>;
#endif

END_UPP_NAMESPACE

#endif
