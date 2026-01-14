#include "AudioFileOut.h"
#include <SoftAudio/SoftAudio.h>


#if defined flagAUDIO


NAMESPACE_UPP


struct AFOCoreAudio::NativeSink {
	Audio::FileWaveOut waveout;
	int size = 0;
	bool debug_sound_enabled = false;
	String debug_sound_output;
	int debug_sound_seed = 0;
	bool debug_print_enabled = false;
	
};


bool AFOCoreAudio::Sink_Create(NativeSink*& dev) {
	dev = new NativeSink;
	return true;
}

void AFOCoreAudio::Sink_Destroy(NativeSink*& dev) {
	delete dev;
}

bool AFOCoreAudio::Sink_Initialize(NativeSink& dev, AtomBase& a, const WorldState& ws) {
	String filepath = ws.GetString(".filepath", "");
	dev.debug_sound_enabled = ws.GetBool(".debug_sound_enabled", false);
	dev.debug_sound_output = ws.GetString(".debug_sound_output", "");
	dev.debug_sound_seed = ws.GetInt(".debug_sound_seed", 0);
	dev.debug_print_enabled = ws.GetBool(".debug_print_enabled", false);
	
	if (filepath.IsEmpty()) {
		filepath = AppendFileName(GetHomeDirectory(), "audio-out.wav");
	}
	
	Audio::Audio::AudioFormat fmt = Audio::Audio::AUDIO_FLOAT32;
	if (!dev.waveout.OpenFile(filepath, 2, Audio::FileWrite::FILE_WAV, fmt)) {
		LOG("AFOCoreAudio::Sink_Initialize: error: could not open file " << filepath);
		return false;
	}
	
	return true;
}

bool AFOCoreAudio::Sink_PostInitialize(NativeSink& dev, AtomBase& a) {
	return true;
}

bool AFOCoreAudio::Sink_Start(NativeSink&, AtomBase&) {
	return true;
}

void AFOCoreAudio::Sink_Stop(NativeSink&, AtomBase&) {
	
}

void AFOCoreAudio::Sink_Uninitialize(NativeSink& dev, AtomBase& a) {
	LOG("AFOCoreAudio::Sink_Uninitialize: info: wrote " << dev.size << " bytes");
	dev.waveout.CloseFile();
}

bool AFOCoreAudio::Sink_Send(NativeSink&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	return true;
}

void AFOCoreAudio::Sink_Visit(NativeSink&, AtomBase&, Visitor& vis) {
	
}

bool AFOCoreAudio::Sink_Recv(NativeSink& dev, AtomBase& a, int ch, const Packet& p) {
	ValueFormat fmt = p->GetFormat();
	
	if (fmt.IsAudio()) {
		AudioFormat& afmt = fmt;
		
		if (afmt.IsSampleFloat()) {
			const Vector<byte>& data = p->GetData();
			int count = data.GetCount() / sizeof(float);
			if (count % 2) count--;
			const float* from = (const float*)(const byte*)data.Begin();
			const float* end = from + count;
			
			
			while (from != end) {
				dev.waveout.Tick(from[0], from[1]);
				from += 2;
			}
			
			dev.size += data.GetCount();
			
			float frame_time = (float)afmt.GetFrameSeconds();
			GlobalAudioTime::Local().Add(frame_time);
		}
	}
	return true;
}

void AFOCoreAudio::Sink_Finalize(NativeSink&, AtomBase&, RealtimeSourceConfig&) {
	
}

bool AFOCoreAudio::Sink_IsReady(NativeSink& dev, AtomBase& a, PacketIO& io) {
	return io.active_sink_mask && !io.full_src_mask;
}

bool AFOCoreAudio::Sink_IsDebugSoundEnabled(const NativeSink& dev, const AtomBase&) {
	return dev.debug_sound_enabled;
}

String AFOCoreAudio::Sink_GetDebugSoundOutput(const NativeSink& dev, const AtomBase&) {
	return dev.debug_sound_output;
}

int AFOCoreAudio::Sink_GetDebugSoundSeed(const NativeSink& dev, const AtomBase&) {
	return dev.debug_sound_seed;
}

bool AFOCoreAudio::Sink_IsDebugPrintEnabled(const NativeSink& dev, const AtomBase&) {
	return dev.debug_print_enabled;
}


END_UPP_NAMESPACE


#endif
