#include "Synth.h"

#if defined flagAUDIO && defined flagMIDI && defined flagLV2

#include <plugin/lilv/lilv.h>
#include <plugin/lilv/lilv_config.h>
#include <plugin/lilv/lilvmm.hpp>
#include <AudioHost/AudioHost.h>


NAMESPACE_UPP


struct SynLV2::NativeInstrument {
    Index<String> lv2_list;
    One<Lv2Host> host;
    int nframes;
    bool debug_sound_enabled = false;
    String debug_sound_output;
    int debug_sound_seed = 0;
    bool debug_print_enabled = false;
};


bool SynLV2::Instrument_Create(NativeInstrument*& dev){
	dev = new NativeInstrument;
	return true;
}

void SynLV2::Instrument_Destroy(NativeInstrument*& dev){
	delete dev;
}

bool SynLV2::Instrument_Initialize(NativeInstrument& dev, AtomBase& a, const WorldState& ws){
	String preset = ws.GetString("preset", "piano");
	dev.debug_sound_enabled = ws.GetBool(".debug_sound_enabled", false);
	dev.debug_sound_output = ws.GetString(".debug_sound_output", "");
	dev.debug_sound_seed = ws.GetInt(".debug_sound_seed", 0);
	dev.debug_print_enabled = ws.GetBool(".debug_print_enabled", false);
	
	LoadAllLV2Plugins(dev.lv2_list);
	
	dev.nframes = ws.GetInt(".nframes", 128);
	
	String plugin_uri = ws.GetString(".path", "");
	Index<String> plugin_uris;
	
	if (plugin_uri.GetCount()) {
		plugin_uris.Add(plugin_uri);
	}
	else {
		GetLv2InstrumentCandidates(preset, dev.lv2_list, plugin_uris);
	}
	
	if (plugin_uris.IsEmpty()) {
		LOG("SynLV2::Instrument_Initialize: error: empty lv2 uri");
		return false;
	}
	
	ISourcePtr src = a.GetSource();
	int c = src->GetSourceCount();
	ValueBase& v = src->GetSourceValue(c-1);
	ValueFormat fmt = v.GetFormat();
	if (!fmt.IsAudio()) {
		LOG("SynLV2::Instrument_Initialize: error: internal error");
		return false;
	}
	
	AudioFormat& afmt = fmt;
	int sample_rate = afmt.GetSampleRate();
	int freq = afmt.GetFrequency();
	
	for (String plugin_uri : plugin_uris) {
		ASSERT(plugin_uri.GetCount());
		dev.host = new Lv2Host(0, freq, sample_rate, plugin_uri);
		if (dev.host && dev.host->IsInitialized()) {
			return true;
		}
	}
	
	LOG("SynLV2::Instrument_Initialize: error: could not load lv2 preset '" + preset + "' with"
		" frequency " << freq <<
		", sample-rate " << sample_rate);
	
	return false;
}

bool SynLV2::Instrument_PostInitialize(NativeInstrument&, AtomBase&) {
	return true;
}

bool SynLV2::Instrument_Start(NativeInstrument&, AtomBase&) {
	return true;
}

void SynLV2::Instrument_Stop(NativeInstrument&, AtomBase&) {
	
}

void SynLV2::Instrument_Uninitialize(NativeInstrument&, AtomBase&) {
	
}

bool SynLV2::Instrument_Send(NativeInstrument& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	if (!dev.host || !dev.host->IsInitialized())
		return false;

	ValueFormat fmt = out.GetFormat();
	if (!fmt.IsAudio())
		return false;

	AudioFormat& afmt = fmt;
	int frames = afmt.GetSampleRate();
	if (frames <= 0)
		frames = dev.nframes;
	if (frames <= 0)
		return false;

	int actual_frames = frames;
	if (dev.nframes > 0 && actual_frames > dev.nframes)
		actual_frames = dev.nframes;
	if (actual_frames <= 0)
		actual_frames = dev.nframes;

	dev.host->Process(dev.nframes);

	Vector<float*>& audio_bufs = dev.host->GetAudioBuffers();
	Vector<float*> outputs;
	for (float* buf : audio_bufs) {
		if (buf)
			outputs.Add(buf);
	}
	if (outputs.IsEmpty())
		return false;

	int channels = afmt.res[0];
	if (channels <= 0 || channels > outputs.GetCount())
		channels = outputs.GetCount();
	if (channels <= 0)
		return false;

	int sample_count = actual_frames * channels;
	Vector<byte>& data = out.Data();

	if (afmt.IsSampleFloat()) {
		data.SetCount(sample_count * sizeof(float));
		float* dst = (float*)(byte*)data.Begin();
		for (int f = 0; f < actual_frames; f++) {
			for (int ch = 0; ch < channels; ch++) {
				dst[f * channels + ch] = outputs[ch][f];
			}
		}
	}
	else if (afmt.GetType() == BinarySample::S16_LE) {
		data.SetCount(sample_count * sizeof(short));
		short* dst = (short*)(byte*)data.Begin();
		for (int f = 0; f < actual_frames; f++) {
			for (int ch = 0; ch < channels; ch++) {
				float sample = outputs[ch][f];
				int scaled = (int)(sample * 32767.0f);
				if (scaled < -32768)
					scaled = -32768;
				else if (scaled > 32767)
					scaled = 32767;
				dst[f * channels + ch] = (short)scaled;
			}
		}
	}
	else {
		data.SetCount(sample_count * sizeof(float));
		float* dst = (float*)(byte*)data.Begin();
		for (int f = 0; f < actual_frames; f++) {
			for (int ch = 0; ch < channels; ch++) {
				dst[f * channels + ch] = outputs[ch][f];
			}
		}
	}

	out.SetAge(0.5);

	if (a.packet_router && !a.router_source_ports.IsEmpty() && fmt.IsValid()) {
		int credits = a.RequestCredits(src_ch, 1);
		if (credits <= 0) {
			RTLOG("SynLV2::Instrument_Send: credit request denied for src_ch=" << src_ch);
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

	return true;
}

void SynLV2::Instrument_Visit(NativeInstrument&, AtomBase&, Visitor& vis) {
	
}

bool SynLV2::Instrument_Recv(NativeInstrument& dev, AtomBase& a, int src_ch, const Packet& p) {
	ValueFormat fmt = p->GetFormat();
	
	if (fmt.IsMidi()) {
		const Vector<byte>& data = p->Data();
		int count = data.GetCount() / sizeof(MidiIO::Event);
		
		const MidiIO::Event* ev  = (const MidiIO::Event*)(const byte*)data.Begin();
		const MidiIO::Event* end = ev + count;
		
		while (ev != end) {
			dev.host->HandleEvent(*ev);
			ev++;
		}
		return true;
	}
	else if (fmt.IsOrder()) {
		// pass
	}
	else return false;
	
	return true;
}

void SynLV2::Instrument_Finalize(NativeInstrument&, AtomBase&, RealtimeSourceConfig&) {
	
}

bool SynLV2::Instrument_IsReady(NativeInstrument&, AtomBase&, PacketIO& io) {
	return io.active_sink_mask & 0x1;
}

bool SynLV2::Instrument_IsDebugSoundEnabled(const NativeInstrument& dev, const AtomBase&) {
	return dev.debug_sound_enabled;
}

String SynLV2::Instrument_GetDebugSoundOutput(const NativeInstrument& dev, const AtomBase&) {
	return dev.debug_sound_output;
}

int SynLV2::Instrument_GetDebugSoundSeed(const NativeInstrument& dev, const AtomBase&) {
	return dev.debug_sound_seed;
}

bool SynLV2::Instrument_IsDebugPrintEnabled(const NativeInstrument& dev, const AtomBase&) {
	return dev.debug_print_enabled;
}


END_UPP_NAMESPACE

#endif
