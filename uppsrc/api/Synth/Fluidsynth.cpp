#include "Synth.h"
#include "../Audio/DebugAudioPattern.h"
#include <algorithm>
#include <MidiFile/MidiFile.h>

#ifdef flagFLUIDLITE
	#include <plugin/fluidlite/fluidlite.h>
	#include <plugin/fluidlite/types.h>
	#include <plugin/fluidlite/fluid_list.h>
	#include <plugin/fluidlite/fluid_synth.h>
	#include <plugin/fluidlite/fluid_defsfont.h>
	#include <plugin/fluidlite/fluid_sfont.h>
#endif

#ifdef flagFLUIDSYNTH
	#include <fluidsynth.h>
#endif

#if defined flagFLUIDSYNTH || defined flagFLUIDLITE


#ifndef HAVE_PACKETTIMING
	#error HAVE_PACKETTIMING not defined
#endif


NAMESPACE_UPP

#define DETECT_DELAY 1

struct SynFluidsynth::NativeInstrument {
    fluid_settings_t* settings;
    fluid_synth_t* synth;
    int sfont_id;
    bool sf_loaded;
    int sample_rate;       // Hz
    int packet_frames;     // frames per audio packet
    int output_channels;
    Vector<float> buffer;
    Vector<float*> dry;
    Vector<float*> fx;
    RunningFlag flag;
    Array<Vector<byte>> packets;
    Mutex lock;
    bool prevent_patch_change;
    int max_cache;
    int packet_count;
    bool realtime;
    bool debug_sound_enabled;
    String debug_sound_output;
    int debug_sound_seed;
    uint64 debug_frame_cursor;
    bool debug_logged_header;
    bool debug_print_enabled;
    
    #if DETECT_DELAY
    TimeStop ts;
    bool silence;
    bool detection_ready;
    #endif
    
};



void SynFluidsynth_ConfigureTrack(SynFluidsynth::NativeInstrument& dev, const MidiIO::File& file, int track_i);
bool SynFluidsynth_LoadSoundfontFile(SynFluidsynth::NativeInstrument& dev, String path);
bool SynFluidsynth_LoadAnyPreset(SynFluidsynth::NativeInstrument& dev);
bool SynFluidsynth_InitializeSoundfont(SynFluidsynth::NativeInstrument& dev, int patch);
void SynFluidsynth_Instrument_Update(SynFluidsynth::NativeInstrument& dev, AtomBase& a, Vector<byte>& out);
void SynFluidsynth_HandleEvent(SynFluidsynth::NativeInstrument& dev, const MidiIO::Event& e, int track_i=-1);
void SynFluidsynth_ProcessThread(SynFluidsynth::NativeInstrument* dev, AtomBase* a);


bool SynFluidsynth::Instrument_Initialize(NativeInstrument& dev, AtomBase& a, const WorldState& ws) {
	int queue = max(1, ws.GetInt(".queue", DEFAULT_AUDIO_QUEUE_SIZE));
	dev.packet_count = 0;
	dev.packet_frames = max(1, ws.GetInt(".packet_frames", ws.GetInt(".frames", 128)));
	dev.sample_rate = max(1, ws.GetInt(".sample_rate", 44100));
	dev.output_channels = 0;
	dev.realtime = ws.GetBool(".realtime", false);
	dev.max_cache = dev.realtime ? 1 : queue;
	dev.debug_sound_enabled = false;
	dev.debug_sound_output.Clear();
	dev.debug_sound_seed = 0;
	dev.debug_frame_cursor = 0;
	dev.debug_logged_header = false;
	dev.debug_print_enabled = false;
	
	#if DETECT_DELAY
    dev.silence = true;
    dev.detection_ready = false;
	#endif
	
	dev.settings = new_fluid_settings();
	fluid_settings_setnum(dev.settings, "synth.sample-rate", dev.sample_rate);
	fluid_settings_setstr(dev.settings, "synth.verbose", ws.GetBool(".verbose", false) ? "yes" : "no");
		
	dev.synth = new_fluid_synth(dev.settings);
	
	int patch = ws.GetInt(".patch", -1);
	dev.prevent_patch_change = patch >= 0;
	SynFluidsynth_InitializeSoundfont(dev, patch);
	
	
	ISourcePtr src = a.GetSource();
	int c = src->GetSourceCount();
	ValueBase& v = src->GetSourceValue(c-1);
	ValueFormat fmt = v.GetFormat();
	if (fmt.IsAudio()) {
		AudioFormat& afmt = fmt;
		afmt.SetType(BinarySample::FLT_LE);
		afmt.SetSampleRate(dev.packet_frames);
		afmt.SetFrequency(dev.sample_rate);
		int ch = afmt.res[0];
		if (ch <= 0)
			ch = ws.GetInt(".channels", 2);
		if (ch <= 0)
			ch = 2;
		if (ch < 2)
			ch = 2;
		afmt.res[0] = ch;
		dev.output_channels = ch;
		v.SetFormat(fmt);
	}
	
	if (auto* synth = dynamic_cast<SynthInstrumentT<SynFluidsynth>*>(&a)) {
		dev.debug_sound_enabled = synth->IsDebugSoundEnabled();
		dev.debug_sound_output = synth->GetDebugSoundOutput();
		dev.debug_sound_seed = synth->GetDebugSoundSeed();
		dev.debug_print_enabled = synth->IsDebugPrintEnabled();
		dev.debug_frame_cursor = 0;
		dev.debug_logged_header = false;
		if (dev.debug_sound_enabled) {
			LOG("SynFluidsynth: debug sound output mode '" << dev.debug_sound_output << "', seed " << dev.debug_sound_seed);
			if (dev.debug_print_enabled)
				Cout() << "SynFluidsynth: debug sound output mode '" << dev.debug_sound_output << "', seed " << dev.debug_sound_seed << '\n';
		}
	}
	
	a.SetQueueSize(dev.realtime ? 1 : dev.max_cache);
	//a.SetQueueSize(DEFAULT_AUDIO_QUEUE_SIZE);
		
    return true;
}

bool SynFluidsynth::Instrument_PostInitialize(NativeInstrument& dev, AtomBase& a) {
    return true;
}

bool SynFluidsynth::Instrument_Start(NativeInstrument& dev, AtomBase& a) {
	InterfaceSourcePtr src_iface = a.GetSource();
	int src_c = src_iface->GetSourceCount();
	if (src_c > 0) {
		ValueBase& src_val = src_iface->GetSourceValue(src_c - 1);
		LOG("SynFluidsynth::Instrument_Start: output queue min=" << src_val.GetMinPackets() << " max=" << src_val.GetMaxPackets());
		if (dev.debug_print_enabled)
			Cout() << "SynFluidsynth::Instrument_Start: output queue min=" << src_val.GetMinPackets() << " max=" << src_val.GetMaxPackets() << '\n';
	}
	dev.flag.Start(1);
	UPP::Thread::Start(callback2(&SynFluidsynth_ProcessThread, &dev, &a));
	return true;
}

void SynFluidsynth::Instrument_Stop(NativeInstrument& dev, AtomBase& a) {
	dev.flag.Stop();
}

void SynFluidsynth::Instrument_Uninitialize(NativeInstrument& dev, AtomBase& a) {
	if (dev.settings) {
		delete_fluid_synth(dev.synth);
		delete_fluid_settings(dev.settings);
	}
	dev.settings = 0;
	dev.synth = 0;
	dev.sfont_id = -1;
    dev.sf_loaded = false;
}

bool SynFluidsynth::Instrument_Send(NativeInstrument& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	ValueFormat fmt = out.GetFormat();
	if (fmt.IsAudio()) {
		#if DETECT_DELAY
		if (!dev.detection_ready && !dev.silence) {
			LOG("SynFluidsynth::Instrument_Send: Silence lost in " << dev.ts.ToString());
			dev.detection_ready = true;
		}
		#endif
		AudioFormat& afmt = fmt;
		int sr = afmt.GetSampleRate();
		ASSERT(sr == dev.packet_frames);
	ASSERT(afmt.res[0] == 2);
		ASSERT(afmt.IsSampleFloat());
		
		if (dev.packets.GetCount()) {
			dev.lock.Enter();
			if (!dev.realtime) {
				Swap(out.Data(), dev.packets[0]);
				dev.packets.Remove(0);
			}
			else {
				Swap(out.Data(), dev.packets.Top());
				dev.packets.Clear();
			}
		int pending_after = dev.packets.GetCount();
		dev.lock.Leave();
		if (dev.debug_print_enabled)
			Cout() << "SynFluidsynth::Instrument_Send: pending packets=" << pending_after << '\n';
		else
			RTLOG("SynFluidsynth::Instrument_Send: pending packets=" << pending_after);
		}
		else out.Data().SetCount(afmt.GetFrameSize(), 0);
		
		#if HAVE_PACKETTIMING
		out.SetBeginTime();
		#endif
		
		#if HAVE_PACKETTIMINGCHECK
		out.SetTimingLimit(0.050f);
		#endif
		
		if (a.packet_router && !a.router_source_ports.IsEmpty() && fmt.IsValid()) {
			int credits = a.RequestCredits(src_ch, 1);
			if (credits <= 0) {
				RTLOG("SynFluidsynth::Instrument_Send: credit request denied for src_ch=" << src_ch);
				return false;
			}
			Packet route_pkt = CreatePacket(out.GetOffset());
			route_pkt.Pick(out);
			route_pkt->SetFormat(fmt);
			bool routed = a.EmitViaRouter(src_ch, route_pkt);
			a.AckCredits(src_ch, credits);
			out.Pick(*route_pkt);
			if (!routed)
				return false;
		}
	}
	return true;
}

bool SynFluidsynth::Instrument_IsReady(NativeInstrument& dev, AtomBase& a, PacketIO& io) {
	// Primary sink is required always (continuous audio) so ignore midi input, which is mixed
	// to primary occasionally.
	return (io.active_sink_mask & 0x1) && io.full_src_mask == 0;
}

bool SynFluidsynth::Instrument_Recv(NativeInstrument& dev, AtomBase& a, int sink_i, const Packet& in) {
	ValueFormat fmt = in->GetFormat();
	if (fmt.IsMidi()) {
		#if HAVE_PACKETTIMINGCHECK
		in->CheckTiming();
		Cout() << "SynFluidsynth::Instrument_Recv: consuming age " << in->GetAge() << EOL;
		LOG("SynFluidsynth::Instrument_Recv: consuming age " << in->GetAge());
		#endif
		
		const Vector<byte>& data = in->Data();
		int count = data.GetCount() / sizeof(MidiIO::Event);
		
		const MidiIO::Event* ev  = (const MidiIO::Event*)(const byte*)data.Begin();
		const MidiIO::Event* end = ev + count;
		bool prevent_patch_change = dev.prevent_patch_change;
		
		while (ev != end) {
			#if DETECT_DELAY
			if (!dev.packet_count) {
				dev.silence = true;
				dev.detection_ready = false;
				dev.ts.Reset();
			}
			#endif
			
			if (prevent_patch_change && ev->IsPatchChange())
				; // pass
			else
				SynFluidsynth_HandleEvent(dev, *ev);
			ev++;
			dev.packet_count++;
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

void SynFluidsynth::Instrument_Finalize(NativeInstrument& dev, AtomBase&, RealtimeSourceConfig&) {
	
}









bool SynFluidsynth::Instrument_Create(NativeInstrument*& dev) {
	dev = new NativeInstrument;
	return true;
}

void SynFluidsynth::Instrument_Destroy(NativeInstrument*& dev) {
	delete dev;
}

void SynFluidsynth::Instrument_Visit(NativeInstrument& dev, AtomBase&, Visitor& vis) {
	
}

bool SynFluidsynth_LoadAnyPreset(SynFluidsynth::NativeInstrument& dev) {
	int bank = -1, prog = -1;
	
	int sf_count = fluid_synth_sfcount(dev.synth);
	
	for(int i = 0; i < sf_count; i++) {
		fluid_sfont_t* sfont = fluid_synth_get_sfont(dev.synth, i);
		const char* name = fluid_sfont_get_name(sfont);
		int preset_i = 0;
		#ifdef flagFLUIDLITE
		fluid_defsfont_t* defsfont = (fluid_defsfont_t*)sfont->data;
		if (defsfont) {
			fluid_defpreset_t* preset = defsfont->preset;
			if (preset != NULL) {
				bank = preset->bank;
				prog = preset->num;
			}
		}
		#else
		fluid_sfont_iteration_start(sfont);
		while (1) {
			fluid_preset_t* preset = fluid_sfont_iteration_next(sfont);
			if (!preset) break;
			bank = fluid_preset_get_banknum(preset);
			prog = fluid_preset_get_num(preset);
			preset_i++;
		}
		#endif
	}
	
	if (bank >= 0 && prog >= 0) {
		int channels = fluid_synth_count_midi_channels(dev.synth);
		for (int i = 0; i < channels; i++){
			//int prog = fluid_channel_get_prognum(dev.synth->channel[i]);
			fluid_synth_bank_select(dev.synth, i, bank);
			fluid_synth_program_change(dev.synth, i, prog);
		}
	}
	return true;
}

bool SynFluidsynth_InitializeSoundfont(SynFluidsynth::NativeInstrument& dev, int patch) {
	bool loaded = false;
	
	if (patch >= 0) {
		String dir = ShareDirFile("sf2");
		Index<String> sf2_dirs;
		GetDirectoryFiles(dir, sf2_dirs);
		String patch_str = (patch < 10 ? "0":"") + IntStr(patch+1) + "_";
		//DUMP(patch_str);
		//DUMPC(sf2_dirs);
		for (String sf2_dir : sf2_dirs) {
			String dir_title = GetFileTitle(sf2_dir);
			if (dir_title.Left(patch_str.GetCount()) == patch_str) {
				String instrument = AppendFileName(sf2_dir, "instrument.sf2");
				DUMP(instrument);
				if (FileExists(instrument)) {
					LOG("Loading soundfont: " << instrument);
					if (SynFluidsynth_LoadSoundfontFile(dev, instrument)) {
						SynFluidsynth_LoadAnyPreset(dev);
						loaded = true;
						break;
					}
				}
			}
		}
	}
	
	if (!loaded) {
		Index<String> dirs;
		dirs.Add( ShareDirFile("soundfonts") );
		dirs.Add( ConfigFile("") );
		dirs.Add( GetHomeDirectory() );
		dirs.Add( "/usr/share/sounds/sf2" );
		dirs.Add( "/usr/share/soundfonts" );
		dirs.Add( "/usr/local/share/soundfonts" );
		
		Index<String> files;
		files.Add("FluidR3 GM.sf2");
		files.Add("FluidR3_GM.sf2");
		files.Add("TimGM6mb.sf2");
		files.Add("FluidR3_GS.sf2");
		files.Add("ChoriumRevA.sf2");
		files.Add("WeedsGM3.sf2");
		files.Add("UHD3.sf2");
		
		for(String file : files) {
			for(String dir : dirs) {
				String path = AppendFileName(dir, file);
				if (FileExists(path)) {
					LOG("Loading soundfont: " << path);
					if (SynFluidsynth_LoadSoundfontFile(dev, path))
						loaded = true;
				}
				if (loaded) break;
			}
			if (loaded) break;
		}
	}
	
	if (!loaded) {
		LOG("FluidsynthSystem: error: could not load any soundfont");
		return false;
	}
	
	return true;
}

void SynFluidsynth_ProcessThread(SynFluidsynth::NativeInstrument* dev, AtomBase* a) {
	int channels = dev->output_channels ? dev->output_channels : 2;
	int buf_size = channels * dev->packet_frames * sizeof(float);
	float wait_time = (float)dev->packet_frames / (float)dev->sample_rate;
	
	while (dev->flag.IsRunning()) {
		if (dev->packets.GetCount() >= dev->max_cache) {
			Sleep(1);
			continue;
		}
		
		Vector<byte>* p = new Vector<byte>();
		p->SetCount(buf_size);
		
		SynFluidsynth_Instrument_Update(*dev, *a, *p);
		
		dev->lock.Enter();
		dev->packets.Add(p);
		int pending = dev->packets.GetCount();
		dev->lock.Leave();
		if (dev->debug_print_enabled)
			Cout() << "SynFluidsynth thread buffered packets=" << pending << '\n';
		else
			RTLOG("SynFluidsynth thread buffered packets=" << pending);
	}
	
	dev->flag.DecreaseRunning();
	
}

void SynFluidsynth_Instrument_Update(SynFluidsynth::NativeInstrument& dev, AtomBase& a, Vector<byte>& out) {
	int channels = dev.output_channels ? dev.output_channels : 2;
	if (dev.debug_sound_enabled) {
		int frames = dev.packet_frames;
		int out_samples = frames * channels;
		int out_size = out_samples * sizeof(float);
		out.SetCount(out_size);
		float* out_begin = (float*)(byte*)out.Begin();
		DebugAudioPatternFill(out_begin, frames, channels, dev.debug_frame_cursor, dev.debug_sound_seed);
		if (!dev.debug_logged_header && frames > 0) {
			Vector<float> preview;
			int preview_frames = min(frames, 3);
			preview.SetCount(preview_frames * channels);
			DebugAudioPatternFill(preview.Begin(), preview_frames, channels, dev.debug_frame_cursor, dev.debug_sound_seed);
			String msg;
			for (int i = 0; i < preview_frames; ++i) {
				msg.Cat(Format(" frame%02d:", i));
				for (int ch = 0; ch < channels; ++ch) {
					msg.Cat(Format(" ch%d=%.3f", ch, preview[i * channels + ch]));
				}
			}
			LOG("SynFluidsynth debug output preview ->" << msg);
			if (dev.debug_print_enabled)
				Cout() << "SynFluidsynth debug output preview ->" << msg << '\n';
			dev.debug_logged_header = true;
		}
		dev.debug_frame_cursor += frames;
		#if DETECT_DELAY
		dev.silence = false;
		dev.detection_ready = true;
		#endif
		return;
	}
	
	// lookup number of audio and effect (stereo-)channels of the synth
    // see "synth.audio-channels", "synth.effects-channels" and "synth.effects-groups" settings respectively
    int n_aud_chan = fluid_synth_count_audio_channels(dev.synth);
    
    // by default there are two effects stereo channels (reverb and chorus) ...
    int n_fx_chan = fluid_synth_count_effects_channels(dev.synth);
    
    // ... for each effects unit. Each unit takes care of the effects of one MIDI channel.
    // If there are less units than channels, it wraps around and one unit may render effects of multiple
    // MIDI channels.
    //n_fx_chan *= fluid_synth_count_effects_groups(dev.synth);
    
    // for simplicity, allocate one single sample pool
    int sample_count = dev.packet_frames * (n_aud_chan + n_fx_chan) * 2;
    dev.buffer.SetCount(sample_count);
    float* samp_buf = (float*)dev.buffer.Begin();
    
    // array of buffers used to setup channel mapping
    Vector<float*>& dry = dev.dry;
    Vector<float*>& fx = dev.fx;
    dry.SetCount(n_aud_chan * 2);
    fx.SetCount(n_fx_chan * 2);
    
    // setup buffers to mix dry stereo audio to
    // buffers are alternating left and right for each n_aud_chan,
    // please review documentation of fluid_synth_process()
    for(int i = 0; i < n_aud_chan * 2; i++)
    {
        dry[i] = &samp_buf[i * dev.packet_frames];
    }
    
    // setup buffers to mix effects stereo audio to
    // similar channel layout as above, revie fluid_synth_process()
    for(int i = 0; i < n_fx_chan * 2; i++)
    {
        fx[i] = &samp_buf[n_aud_chan * 2 * dev.packet_frames + i * dev.packet_frames];
    }
    
    int out_samples = dev.packet_frames * channels;
    int out_size = out_samples * sizeof(float);
    out.SetCount(out_size);
    float* out_begin = (float*)(byte*)out.Begin();
    float* out_write = out_begin;
    
    #if 1
    
    fluid_synth_write_float(dev.synth, dev.packet_frames,
		out_begin, 0, channels,
		out_begin, 1, channels);
	
	
    #elif 1
    
    
    // dont forget to zero sample buffer(s) before each rendering
    memset(samp_buf, 0, sample_count * sizeof(float));
    int err = fluid_synth_process(dev.synth, dev.packet_frames, n_fx_chan * 2, fx.Begin(), n_aud_chan * 2, dry.Begin());
    if(err == FLUID_FAILED) {
        ASSERT_(0, "oops");
    }
    
    const float* l_from = dry[0];
    const float* r_from = dry[1];
    for(int i = 0; i < dev.packet_frames; i++) {
		*out_write++ = *l_from++;
		*out_write++ = *r_from++;
    }
    
    
    #else
    
    
    #define FLT_TO_S16(x) (x) * INT16_MAX
    #define FLT_TO_U16(x) ((x) + 1) * 0.5 * UINT16_MAX
    static int sample_i;
    int len = 1.0 / 440.0 * 44100;
    for(int i = 0; i < dev.packet_frames; i++) {
        float f = sin(sample_i / (float)len * 2 * M_PI);
		*out_write++ = FLT_TO_S16(f);
		*out_write++ = FLT_TO_S16(f);
		sample_i = (sample_i + 1) % len;
    }
    
    
    #endif
    
    
    #if DETECT_DELAY
    if (dev.silence) {
		float* it = out_begin;
        float* end = it + dev.packet_frames * channels;
        while (it < end) {
            if (fabsf(*it) > 0.01) {
                dev.silence = false;
                break;
            }
            it++;
        }
    }
    #endif
}

bool SynFluidsynth_LoadSoundfontFile(SynFluidsynth::NativeInstrument& dev, String path) {
	ASSERT(dev.synth);
	ASSERT(dev.settings);
	
    dev.sf_loaded = false;
    
	dev.sfont_id = fluid_synth_sfload(dev.synth, path.Begin(), 1);
    if (dev.sfont_id < 0)
        return false;
    
    dev.sf_loaded = true;
    return true;
}

void SynFluidsynth_ConfigureTrack(SynFluidsynth::NativeInstrument& dev, const MidiIO::File& file, int track_i) {
	int tracks = file.GetTrackCount();
	int fs_tracks = fluid_synth_count_midi_channels(dev.synth);
	
	if (track_i < 0 || track_i >= tracks) {
		LOG("FluidsynthSystem: error: invalid track_i " << track_i << " based on file");
		return;
	}
	
	if (track_i < 0 || track_i >= fs_tracks) {
		LOG("FluidsynthSystem: error: invalid track_i " << track_i << " based on fluidsynth");
		return;
	}
	
	const MidiIO::EventList& track = file[track_i];
	for(int i = 0; i < track.GetCount(); i++) {
		const MidiIO::Event& e = track[i];
		if (e.tick > 0)
			break;
		
		SynFluidsynth_HandleEvent(dev, e, track_i);
	}
}

void SynFluidsynth_HandleEvent(SynFluidsynth::NativeInstrument& dev, const MidiIO::Event& e, int track_i) {
	int channel = e.GetChannel();

	if (e.IsController()) {
		if (track_i < 0 || channel == track_i) {
			int num = e.GetP1();
			int value = e.GetP2();
			LOG("Configure event: " <<
				e.ToString() << ": " <<
				MidiIO::GetEventCtrlString(num) << " = " << value);
			fluid_synth_cc(dev.synth, channel, num, value);
		}
	}
	else if (e.IsMeta()) {
		bool send = false;
		int mnum = e.GetP1();
		int strlen = e.GetP2();
		String str;
		for(int i = 3; i < 3 + strlen; i++) {
			int chr = e[i];
			str.Cat(chr);
		}
		switch (mnum) {
			case MidiIO::MIDIMETA_SEQNUM:
				break;
			case MidiIO::MIDIMETA_TEXT:
				LOG("Midi-string: " << str);
				break;
			case MidiIO::MIDIMETA_COPYRIGHT:
				LOG("Copyright: " << str);
				break;
			case MidiIO::MIDIMETA_TRACKNAME:
				LOG("Track name: " << str);
				break;
			case MidiIO::MIDIMETA_INSTRNAME:
				LOG("Instrument name: " << str);
				break;
			case MidiIO::MIDIMETA_LYRICS:
				break;
			case MidiIO::MIDIMETA_MARKER:
				break;
			case MidiIO::MIDIMETA_CUEPOINT:
				break;
			case MidiIO::MIDIMETA_CHPREFIX:
				break;
			case MidiIO::MIDIMETA_TRACKEND:
				break;
			case MidiIO::MIDIMETA_TEMPO:
				break;
			case MidiIO::MIDIMETA_SMPTE:
				break;
			case MidiIO::MIDIMETA_TIMESIG:
				break;
			case MidiIO::MIDIMETA_KEYSIG:
				break;
			case MidiIO::MIDIMETA_CUSTOM:
				break;
		}
		
	}
	else if (e.IsNoteOff()) {
		fluid_synth_noteoff(dev.synth, channel, e.GetP1());
	}
	else if (e.IsNoteOn()) {
		fluid_synth_noteon(dev.synth, channel, e.GetP1(), e.GetP2());
	}
	else if (e.IsNote()) {
		LOG("Ignore note: " << e.ToString());
	}
	else if (e.IsAftertouch()) {
		LOG("Ignore aftertouch: " << e.ToString());
	}
	else if (e.IsTimbre() || e.IsPatchChange()) {
		int channel = e.GetChannel();
		int value = e.GetP1();
		if (track_i < 0 || channel == track_i) {
			LOG("Changing channel patch: channel " << channel << " to " << value << ": " << e.ToString());
			int bank = 0;
			// if channel is the midi-standard drum channel
			if (channel == 9) {
				bank = 128;
			}
			fluid_synth_program_select(dev.synth, channel, dev.sfont_id, bank, value);
		}
	}
	else if (e.IsPressure()) {
		LOG("Ignore pressure: " << e.ToString());
	}
	else if (e.IsPitchbend()) {
		int channel = e.GetChannel();
		int value = e.GetP1();
		if (track_i < 0 || channel == track_i) {
			LOG("Setting pitch bend: channel " << channel << " to " << value);
			int fs_pbend = (128 + value) * 0x4000 / 256;
			fluid_synth_pitch_bend(dev.synth, channel, fs_pbend);
		}
	}
	else {
		LOG("Unexpected configure event: " << e.ToString());
	}
}


END_UPP_NAMESPACE

#endif
