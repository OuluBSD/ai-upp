// This file have been generated automatically.
// DO NOT MODIFY THIS FILE!

#ifndef _IMidiHw_IMidiHw_h_
#define _IMidiHw_IMidiHw_h_

#include <Eon/Eon.h>
#include <MidiFile/MidiFile.h>
#include <plugin/portmidi/portmidi.h>

NAMESPACE_UPP

#define MID_CLS_LIST(x) \
	MID_CLS(Source, x) \

#define MID_VNDR_LIST \
	MID_VNDR(MidPortmidi) \

#define MID_CLS(x, v) struct v##x;
#define MID_VNDR(x) MID_CLS_LIST(x)
MID_VNDR_LIST
#undef MID_VNDR
#undef MID_CLS

#if (defined flagPORTMIDI) || (defined flagBUILTIN_PORTMIDI)
struct MidPortmidi {
	#if defined flagMIDI
	struct NativeSource;
	#endif
	
	struct Thread {
		
	};
	
	static Thread& Local() {thread_local static Thread t; return t;}
	
	#include "IfaceFuncs.inl"
	
};
#endif

#if defined flagMIDI
struct MidSource : public Atom {
	//RTTI_DECL1(MidSource, Atom)
	void Visit(Vis& vis) override {VIS_THIS(Atom);}
	
	virtual ~MidSource() {}
};
#endif


#if defined flagMIDI
template <class Mid> struct MidiHwSourceT : MidSource {
	using CLASSNAME = MidiHwSourceT<Mid>;
	//RTTI_DECL1(CLASSNAME, MidSource)
	void Visit(Vis& vis) override {
		if (dev) Mid::Source_Visit(*dev, *this, vis);
		vis.VisitThis<MidSource>(this);
	}
	typename Mid::NativeSource* dev = 0;
	bool Initialize(const WorldState& ws) override {
		if (!Mid::Source_Create(dev))
			return false;
		if (!Mid::Source_Initialize(*dev, *this, ws))
			return false;
		return true;
	}
	bool PostInitialize() override {
		if (!Mid::Source_PostInitialize(*dev, *this))
			return false;
		return true;
	}
	bool Start() override {
		return Mid::Source_Start(*dev, *this);
	}
	void Stop() override {
		Mid::Source_Stop(*dev, *this);
	}
	void Uninitialize() override {
		ASSERT(this->GetDependencyCount() == 0);
		Mid::Source_Uninitialize(*dev, *this);
		Mid::Source_Destroy(dev);
	}
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {
		if (!Mid::Source_Send(*dev, *this, cfg, out, src_ch))
			return false;
		return true;
	}
	bool IsReady(PacketIO& io) override {
		return Mid::Source_IsReady(*dev, *this, io);
	}
};
#endif

#if (defined flagPORTMIDI) || (defined flagBUILTIN_PORTMIDI)
#if defined flagMIDI
using PortmidiSource = MidiHwSourceT<MidPortmidi>;
#endif
#endif

END_UPP_NAMESPACE

#endif
