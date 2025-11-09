#include "Synth.h"
#include <MidiFile/MidiFile.h>
#include <atomic>
#include <cmath>


NAMESPACE_UPP

struct SynSoft::NativeInstrument {
    SoftInstru::Instrument instrument;
    bool sf_loaded;
    int sample_rate;      // Hz
    int packet_frames;    // frames rendered per packet
    int output_channels;
    bool realtime;
    int max_queue;
    bool received_midi;
    bool emitted_audio;
    bool debug_logging;
};



bool SynSoft::Instrument_Create(NativeInstrument*& dev) {
	dev = new NativeInstrument;
	return true;
}

void SynSoft::Instrument_Destroy(NativeInstrument*& dev) {
	delete dev;
}

void SynSoft::Instrument_Visit(NativeInstrument& dev, AtomBase&, Visitor& vis) {
	
}

bool SynSoft::Instrument_Initialize(NativeInstrument& dev, AtomBase& a, const WorldState& ws) {
	dev.sample_rate = ws.GetInt(".sample_rate", 44100);
	if (dev.sample_rate < 1)
		dev.sample_rate = 44100;
	dev.packet_frames = ws.GetInt(".packet_frames", ws.GetInt(".frames", 128));
	if (dev.packet_frames < 1)
		dev.packet_frames = 128;
	dev.output_channels = ws.GetInt(".channels", 2);
	if (dev.output_channels < 1)
		dev.output_channels = 2;
	dev.realtime = ws.GetBool(".realtime", false);
	dev.max_queue = ws.GetInt(".queue", DEFAULT_AUDIO_QUEUE_SIZE);
	if (dev.max_queue < 1)
		dev.max_queue = DEFAULT_AUDIO_QUEUE_SIZE;
	dev.received_midi = false;
	dev.emitted_audio = false;
	dev.debug_logging = ws.GetBool(".debug", ws.GetBool(".verbose", false));
	
	//String sf2 = ws.GetString(".filepath", "FluidR3_GM.sf2");
	String sf2 = ws.GetString(".filepath", "TimGM6mb.sf2");
	sf2 = RealizeShareFile(sf2);
	
	if (!dev.instrument.LoadFilename(sf2)) {
		LOG("SynSoft::Instrument_Initialize: error: could not load sf2: "<< sf2);
		return false;
	}
	
	ISourcePtr src = a.GetSource();
	int c = src->GetSourceCount();
	ValueBase& v = src->GetSourceValue(c-1);
	ValueFormat fmt = v.GetFormat();
	if (!fmt.IsAudio())
		return false;
	
	AudioFormat& afmt = fmt;
	afmt.SetType(BinarySample::FLT_LE);
	afmt.SetSampleRate(dev.packet_frames);
	afmt.SetFrequency(dev.sample_rate);
	afmt.res[0] = dev.output_channels;
	v.SetFormat(fmt);
	
	
	// Initialize preset on special 10th MIDI channel to use percussion sound bank (128) if available
	dev.instrument.SetChannelBankPreset(9, 128, 0);
	dev.instrument.SetOutput(SoftInstru::STEREO_INTERLEAVED, (int)afmt.freq, -9);
	
	
	a.SetQueueSize(dev.realtime ? 1 : dev.max_queue);
	
	return true;
}

bool SynSoft::Instrument_PostInitialize(NativeInstrument& dev, AtomBase& a) {
    return true;
}

bool SynSoft::Instrument_Start(NativeInstrument& dev, AtomBase& a) {
    return true;
}

void SynSoft::Instrument_Stop(NativeInstrument& dev, AtomBase& a) {
	
}

void SynSoft::Instrument_Uninitialize(NativeInstrument& dev, AtomBase& a) {
	
}

bool SynSoft::Instrument_Send(NativeInstrument& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	ValueFormat fmt = out.GetFormat();
	if (fmt.IsAudio()) {
		AudioFormat& afmt = fmt;
		int frames = afmt.GetSampleRate();
		int channels = afmt.res[0] > 0 ? afmt.res[0] : dev.output_channels;
		ASSERT(frames == dev.packet_frames);
		ASSERT(channels == dev.output_channels);
		
		Vector<byte>& data = out.Data();
		int sz = afmt.GetFrameSize();
		data.SetCount(sz);
		
		if (afmt.GetType() == BinarySample::S16_LE)
			dev.instrument.RenderShort((short*)(byte*)data.Begin(), dev.packet_frames, 0);
		else if (afmt.GetType() == BinarySample::FLT_LE)
			dev.instrument.RenderFloat((float*)(byte*)data.Begin(), dev.packet_frames, 0);
		
		const float* samples = reinterpret_cast<const float*>(data.Begin());
		int sample_count = sz / sizeof(float);
		float peak = 0.f;
		for (int i = 0; i < sample_count; i++) {
			float val = (float)fabs(samples[i]);
			if (val > peak)
				peak = val;
		}
		if (dev.debug_logging) {
			static int send_log_total = 0;
			static int send_nonzero_log = 0;
			if (send_log_total < 5 || (peak > 0.f && send_nonzero_log < 5)) {
				LOG("SynSoft::Instrument_Send: frames=" << frames << " channels=" << channels << " peak=" << peak);
				send_log_total++;
				if (peak > 0.f)
					send_nonzero_log++;
			}
		}
		if (peak > 0.f)
			dev.emitted_audio = true;
		else if (dev.received_midi && !dev.emitted_audio) {
			static std::atomic<int> silent_warn_count{0};
			if (silent_warn_count.fetch_add(1) < 5)
				LOG("SynSoft::Instrument_Send: warning: received MIDI but audio peak still zero");
		}
		
		out.SetAge(0.5);
	}
	return true;
}

bool SynSoft::Instrument_Recv(NativeInstrument& dev, AtomBase& a, int sink_ch, const Packet& in) {
	ValueFormat fmt = in->GetFormat();
	if (fmt.IsMidi()) {
		const Vector<byte>& data = in->Data();
		int count = data.GetCount() / sizeof(MidiIO::Event);
		
		const MidiIO::Event* ev  = (const MidiIO::Event*)(const byte*)data.Begin();
		const MidiIO::Event* end = ev + count;
		
		dev.received_midi = true;
		while (ev != end) {
			dev.instrument.HandleEvent(*ev);
			ev++;
		}
		if (dev.debug_logging) {
			static std::atomic<int> recv_log_count{0};
			int logged = recv_log_count.load();
			if (logged < 5) {
				if (recv_log_count.fetch_add(1) < 5)
					LOG("SynSoft::Instrument_Recv: midi events processed=" << count);
			}
		}
	}
	else if (fmt.IsOrder()) {
		// pass
	}
	else {
		TODO
	}
	return true;
}

void SynSoft::Instrument_Finalize(NativeInstrument& dev, AtomBase& a, RealtimeSourceConfig& cfg) {
	
}

bool SynSoft::Instrument_IsReady(NativeInstrument& dev, AtomBase& a, PacketIO& io) {
	// Primary sink is required always (continuous audio) so ignore midi input, which is mixed
	// to primary occasionally.
	return (io.active_sink_mask & 0x1) && io.full_src_mask == 0;
}





END_UPP_NAMESPACE
