#include "Audio.h"
#include <Sound/Sound.h>
#include "DebugAudioPattern.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cmath>

#if defined flagBUILTIN_PORTAUDIO || defined flagPORTAUDIO
NAMESPACE_UPP

#undef CHECK_ERR

#ifdef LOG_SOUND_ERRORS
  #define CHECK_ERROR(STREAM) if((STREAM).IsError()) LOG(__FILE__<<" (line "<<__LINE__<<"): "<<(STREAM).GetError());
#else
  #define CHECK_ERROR(STREAM)
#endif
#define CHECK_ERR CHECK_ERROR(PortaudioStatic::Single())

struct PortaudioFormat {
	int				channels = 0;
	int				freq = 0;
	int				sample_rate = 0;
	SampleFormat	fmt = SND_UNKNOWN;
};

ValueFormat ConvertPortaudioFormat(PortaudioFormat pa_fmt) {
	ValueFormat fmt;
	fmt.vd = VD(CENTER,AUDIO);
	AudioFormat& r = fmt;
	r.res[0] = pa_fmt.channels;
	r.freq = pa_fmt.freq;
	r.sample_rate = pa_fmt.sample_rate;
	switch (pa_fmt.fmt) {
		#if CPU_BIG_ENDIAN
		#error TODO
		#endif
		
		case SND_FLOAT32:
			r.SetType(SoundSample::FLT_LE);
			break;
		case SND_INT32:
			r.SetType(SoundSample::S32_LE);
			break;
		case SND_INT24:
			r.SetType(SoundSample::S24_LE);
			break;
		case SND_INT16:
			r.SetType(SoundSample::S16_LE);
			break;
		case SND_INT8:
			r.SetType(SoundSample::S8_LE);
			break;
		case SND_UINT8:
			r.SetType(SoundSample::U8_LE);
			break;
		default:
			throw (Exc("invalid portaudio sound sample format"));
	}
	return fmt;
}

struct PortaudioCallbackData;
static bool ValidateDebugAudioBuffer(PortaudioCallbackData& cb, byte* output, unsigned long frames);

struct PortaudioTimeInfo {
	double			input_adc;
	double			current;
	double			output_adc;
	
	PortaudioTimeInfo(	const PaStreamCallbackTimeInfo* timeinfo):
						input_adc(timeinfo->inputBufferAdcTime),
						output_adc(timeinfo->outputBufferDacTime),
						current(timeinfo->currentTime){}
	
	operator const PaStreamCallbackTimeInfo*(){
		return reinterpret_cast<const PaStreamCallbackTimeInfo*>(this);
	}
};

struct PortaudioCallbackArgs {
	const void*			input;
	void*				output;
	void*				data;
	int					state;
	unsigned long		fpb;
	PortaudioTimeInfo	timeinfo;
	unsigned long		flags;
	
	PortaudioCallbackArgs(const void *input, void *output, unsigned long fpb,
	                   PortaudioTimeInfo timeinfo, unsigned long flags,
	                   void* data) : input(input),output(output),fpb(fpb),timeinfo(timeinfo),
	                                 flags(flags),data(data),state(SND_CONTINUE){};
};

struct PortaudioCallbackData {
	Callback1<void*>					finish;
	void*								data = 0;
	bool								dbg_async_race = false;
	PaStream*							dev = 0;
	AtomBase*							atom = 0;
	ValueFormat							fmt;
	bool								debug_sound_enabled = false;
	String								debug_sound_output;
	int									debug_sound_seed = 0;
	uint64								debug_frame_cursor = 0;
	bool								debug_preview_logged = false;
	bool								debug_mismatch_logged = false;
	bool								debug_print_enabled = false;
	
	void Set(PaStream* d, AtomBase* atom, ValueFormat fmt,
	         Callback1<void*> whenfinish, void* userdata) {
	    this->atom = atom;
	    this->fmt = fmt;
	    dev = d;
	    finish = whenfinish;
	    data = userdata;
	    debug_sound_enabled = false;
	    debug_sound_output.Clear();
	    debug_sound_seed = 0;
	    debug_frame_cursor = 0;
	    debug_preview_logged = false;
	    debug_mismatch_logged = false;
	    debug_print_enabled = false;
	    if (auto* sink = dynamic_cast<PortaudioSinkDevice*>(atom)) {
			debug_sound_enabled = 1; //sink->IsDebugSoundEnabled();
			debug_sound_output = 0; //sink->GetDebugSoundOutput();
			debug_sound_seed = 0; //sink->GetDebugSoundSeed();
			debug_print_enabled = 1; //sink->IsDebugPrintEnabled();
	    }
	}
	
	void SinkCallback(PortaudioCallbackArgs& args) {
		if (!args.output) return;
		
		// This is remaining for old code. Probably useless here.
		#ifdef flagDEBUG
		ASSERT(!this->dbg_async_race);
		this->dbg_async_race = true;
		#endif
		
		
		AudioFormat& afmt = fmt;
		int size = fmt.GetFrameSize();
		byte* out = static_cast<byte*>(args.output);
		if (!Serial_Link_ForwardAsyncMem(atom->GetLink(), out, size)) {
			RTLOG("PortaudioCallbackData::SinkCallback: reading memory failed");
			memset(args.output, 0, size);
		}
		else {
			if (debug_sound_enabled) {
				ValidateDebugAudioBuffer(*this, out, args.fpb);
				memset(out, 0, size); // mute hardware output while keeping debug validation
			}
		}
		
		#ifdef flagDEBUG
		this->dbg_async_race = false;
		#endif
	}
	
};

static float PortaudioDebugExtractSample(const byte* data, int sample_bytes, bool is_float, bool is_signed) {
	if (is_float) {
		if (sample_bytes == (int)sizeof(float)) {
			float value;
			memcpy(&value, data, sizeof(float));
			return value;
		}
		if (sample_bytes == (int)sizeof(double)) {
			double value;
			memcpy(&value, data, sizeof(double));
			return (float)value;
		}
	}
	if (is_signed) {
		switch (sample_bytes) {
		case 1: return (float)*(const int8*)data / 127.f;
		case 2: return (float)*(const int16*)data / 32767.f;
		case 3: {
			int32 raw = (int32)data[0] | ((int32)data[1] << 8) | ((int32)(int8)data[2] << 16);
			return (float)raw / 8388607.f;
		}
		case 4: return (float)*(const int32*)data / 2147483647.f;
		default: break;
		}
	}
	else {
		switch (sample_bytes) {
		case 1: return ((float)*(const uint8*)data / 255.f) * 2.f - 1.f;
		case 2: {
			int32 raw = (int32)*(const uint16*)data;
			return ((float)raw / 65535.f) * 2.f - 1.f;
		}
		default: break;
		}
	}
	return 0.f;
}

static bool ValidateDebugAudioBuffer(PortaudioCallbackData& cb, byte* output, unsigned long frames) {
	const AudioFormat& afmt = cb.fmt;
	int channels = afmt.res[0];
	if (channels <= 0)
		channels = 1;
	int sample_bytes = afmt.GetSampleSize();
	bool is_float = afmt.IsSampleFloat();
	bool is_signed = afmt.IsSampleSigned();
	double tolerance = 0.02;
	byte* cursor = output;
	for (unsigned long frame = 0; frame < frames; ++frame) {
		for (int ch = 0; ch < channels; ++ch) {
			float actual = PortaudioDebugExtractSample(cursor, sample_bytes, is_float, is_signed);
			float expected = DebugAudioPatternValue(cb.debug_frame_cursor + frame, ch, cb.debug_sound_seed);
			if (!cb.debug_mismatch_logged && fabsf(actual - expected) > tolerance) {
				int64 idx = (int64)(cb.debug_frame_cursor + frame);
				String msg = Format("PortaudioSink debug mismatch (mode '%s'): frame %lld, channel %d -> %.5f (expected %.5f)",
				                    cb.debug_sound_output, (long long)idx, ch, actual, expected);
				LOG(msg);
				if (cb.debug_print_enabled)
					Cout() << msg << '\n';
				cb.debug_mismatch_logged = true;
			}
			cursor += sample_bytes;
		}
	}
	if (!cb.debug_preview_logged && frames > 0) {
		int preview_frames = min<int>((int)frames, 3);
		String msg;
		const byte* preview_ptr = output;
		for (int frame = 0; frame < preview_frames; ++frame) {
			msg.Cat(Format(" frame%02d:", frame));
			for (int ch = 0; ch < channels; ++ch) {
				const byte* sample_ptr = preview_ptr + (frame * channels + ch) * sample_bytes;
				float val = PortaudioDebugExtractSample(sample_ptr, sample_bytes, is_float, is_signed);
				msg.Cat(Format(" ch%d=%.3f", ch, val));
			}
		}
		String full = "PortaudioSink debug input preview ->" + msg;
		LOG(full);
		if (cb.debug_print_enabled)
			Cout() << full << '\n';
		cb.debug_preview_logged = true;
	}
	cb.debug_frame_cursor += frames;
	return true;
}

extern "C"{
	//this is a C callable function, to wrap U++ Callback into PaStreamCallback
	int ApiAudioStreamCallback(const void *input, void *output, unsigned long frames,
	          const PaStreamCallbackTimeInfo *timeinfo, unsigned long flags, void *data)
	{
		PortaudioCallbackData *d = static_cast<PortaudioCallbackData*>(data);
		PortaudioCallbackArgs a(input, output, frames, timeinfo, flags, d->data);
		d->SinkCallback(a);
		return a.state;
	}

	//this is a C callable function, to wrap WhenFinish into PaStreamFinishedCallback
	void ApiAudioStreamFinishedCallback(void *data){
		PortaudioCallbackData *d = static_cast<PortaudioCallbackData*>(data);
		d->finish(d->data);
	}
	
}




// PortaudioStatic is used for wrapping static portaudio state for clean destructing

class PortaudioStatic {
	mutable PaError err;
	static bool		exists;
	
	Array<PortaudioCallbackData> cb_data;
	
	
public:
	PortaudioStatic() {
		ASSERT_(!exists, "PortaudioStatic already instantiated!");
		err = Pa_Initialize();
		exists = err == paNoError;
	}
	
	~PortaudioStatic() {
		Clear();
	}
	
	void Clear() {
		if (exists) {
			err = Pa_Terminate();
			CHECK_ERR;
			exists = false;
		}
	}
	
	PortaudioCallbackData& Add() {
		return cb_data.Add();
	}
	
	void Remove(PaStream* s) {
		for(int i = 0; i < cb_data.GetCount(); i++)
			if (cb_data[i].dev == s)
				cb_data.Remove(i--);
	}
	
	static bool Exists() {return exists;}
	
	bool IsError() const {return err != paNoError;}
	PaError GetError() const {return err;}
	
	static PortaudioStatic& Single() {
		static One<PortaudioStatic> s;
		if (s.IsEmpty())
			s.Create();
		return *s;
	}
};

bool PortaudioStatic::exists = false;








#if (defined flagBUILTIN_PORTAUDIO) || (defined flagWIN32 && defined flagMSC)
struct AudPortaudio::NativeSinkDevice {
	PaStream* p;
	bool started;
	NativeSinkDevice() : p(nullptr), started(false) {}
};

struct AudPortaudio::NativeSourceDevice {
	PaStream* p;
};

void AudPortaudio::SinkDevice_Visit(NativeSinkDevice&, AtomBase&, Visitor& vis) {}

bool AudPortaudio::SinkDevice_Create(NativeSinkDevice*& dev) {
	dev = new NativeSinkDevice;
	return true;
}

void AudPortaudio::SinkDevice_Destroy(NativeSinkDevice*& dev) {
	delete dev;
}

bool AudPortaudio::SinkDevice_Initialize(NativeSinkDevice& dev_, AtomBase& a, const WorldState& ws) {
	PaStream*& dev = dev_.p;
	
	bool realtime = ws.GetBool(".realtime", false);
	
	// Housekeeping vars
	PaError err = paNoError;
	dev = 0;
	dev_.started = false;
	
	// Audio format
	PortaudioFormat pa_fmt;
	pa_fmt.freq = 44100;
	pa_fmt.sample_rate = 128;
	pa_fmt.channels = 2;
	pa_fmt.fmt = SND_FLOAT32;
	int in_channels = 0;
	
	// Adjust sink format
	const int sink_ch_i = 0;
	ValueBase& sink_val = a.GetSink()->GetValue(sink_ch_i);
	ValueFormat fmt = ConvertPortaudioFormat(pa_fmt);
	ASSERT(fmt.IsValid());
	sink_val.SetFormat(fmt);
	sink_val.LockFormat();
	
	// Initalize static portaudio instance (if not initialized)
	PortaudioStatic::Single();
	
	// Callbacks (currently unused)
	Callback1<void*> WhenFinished;
	
	// Add static callback data (could be elsewhere than in "PaStatic()". Improve.)
	PortaudioCallbackData& scallback = PortaudioStatic::Single().Add();
	scallback.Set(dev, &a, fmt, WhenFinished, NULL);
	
	// Make our's callback data for native audio sink callback
	PaStreamCallback* cb = &ApiAudioStreamCallback;
	void* data = static_cast<void *>(&scallback);
	
	// Call native stream opener
	ASSERT(PortaudioStatic::Exists());
	err = Pa_OpenDefaultStream(&dev, in_channels, pa_fmt.channels, pa_fmt.fmt, pa_fmt.freq, pa_fmt.sample_rate, cb, data);
	CHECK_ERR;
	if (err != paNoError) // Bail out on errors
		return false;
	
	// Configure native audio stream to have call our finish function on end of stream
	err = Pa_SetStreamFinishedCallback(dev, &ApiAudioStreamFinishedCallback);
	CHECK_ERR;
	if (err != paNoError) // Bail out on errors
		return false;
	
	a.SetQueueSize(realtime ? 1 : DEFAULT_AUDIO_QUEUE_SIZE);
	
	return true;
}

bool AudPortaudio::SinkDevice_PostInitialize(NativeSinkDevice& dev, AtomBase& a) {
	InterfaceSinkPtr sink_iface = a.GetSink();
	if (sink_iface && sink_iface->GetSinkCount() > 0) {
		ValueBase& sink_val = sink_iface->GetValue(0);
		String msg = Format("AudPortaudio::SinkDevice_PostInitialize: input queue min=%d max=%d current=%d", sink_val.GetMinPackets(), sink_val.GetMaxPackets(), sink_val.GetQueueSize());
		LOG(msg);
	}
	return true;
}

bool AudPortaudio::SinkDevice_Start(NativeSinkDevice& dev, AtomBase&) {
	PaError err = paNoError;
	if (!dev.p || dev.started)
		return false;
	
	err = Pa_StartStream(dev.p);
	CHECK_ERR;
	if (err != paNoError) // Bail out on errors
		return false;
	
	ASSERT(!Pa_IsStreamStopped(dev.p));
	dev.started = true;
	return true;
}

void AudPortaudio::SinkDevice_Stop(NativeSinkDevice& dev, AtomBase&) {
	if (!dev.p || !dev.started)
		return;
	PaError err = paNoError;
	err = Pa_StopStream(dev.p);
	CHECK_ERR;
	if (err == paNoError)
		dev.started = false;
}

void AudPortaudio::SinkDevice_Uninitialize(NativeSinkDevice& dev, AtomBase&) {
	if (!dev.p)
		return;
	PaError err = paNoError;
	
	err = Pa_CloseStream(dev.p);
	CHECK_ERR;
	
	PortaudioStatic::Single().Remove(dev.p);
	dev.p = nullptr;
	dev.started = false;
}

bool AudPortaudio::SinkDevice_Send(NativeSinkDevice& dev, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	Panic("won't implement");
	NEVER();
	return false;
}

bool AudPortaudio::SinkDevice_NegotiateSinkFormat(NativeSinkDevice& dev, AtomBase& a, LinkBase& link, int sink_ch, const ValueFormat& new_fmt) {
	
	// accept all valid audio formats for now (because packets can be converted)
	if (new_fmt.IsValid() && new_fmt.IsAudio()) {
		return true;
	}
	
	return false;
}







bool AudPortaudio::SourceDevice_Initialize(NativeSourceDevice& dev, AtomBase& a, const WorldState& ws) {
	TODO
}

void AudPortaudio::SourceDevice_Visit(NativeSourceDevice&, AtomBase&, Visitor& vis) {
	
}

bool AudPortaudio::SourceDevice_Start(NativeSourceDevice& dev, AtomBase&) {
	TODO
}

void AudPortaudio::SourceDevice_Stop(NativeSourceDevice& dev, AtomBase&) {
	TODO
}

void AudPortaudio::SourceDevice_Uninitialize(NativeSourceDevice& dev, AtomBase&) {
	TODO
}

bool AudPortaudio::SourceDevice_Send(NativeSourceDevice& dev, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	TODO
}

#endif




END_UPP_NAMESPACE
#endif
