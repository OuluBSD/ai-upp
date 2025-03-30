#include "Sound.h"

NAMESPACE_UPP


SoundDaemon::SoundDaemon() {
	
}

SoundDaemon::~SoundDaemon() {
	
}

SoundDaemon& SoundDaemon::Static() {
	static SoundDaemon sd;
	return sd;
}

SoundDaemon::ThreadBase& SoundDaemon::GetAddThread(SoundDevice dev, SampleFormat sample, int channels) {
	CombineHash ch;
	ch.Do(dev);
	ch.Do((int)sample);
	ch.Do(channels);
	hash_t h = ch;
	int i = thrds.Find(h);
	if (i >= 0)
		return thrds[i];
	if (sample == paUInt8) {
		auto& t = thrds.Add(h, new SoundDaemon::Thread<uint8>(*this));
		t.SetDevice(dev, channels);
		return t;
	}
	else if (sample == paInt16) {
		auto& t = thrds.Add(h, new SoundDaemon::Thread<int16>(*this));
		t.SetDevice(dev, channels);
		return t;
	}
	else {
		TODO
		static SoundDaemon::Thread<int16> t(*this);
		return t;
	}
}

void SoundDaemon::ThreadBase::CheckEnd(StreamCallbackArgs& args) {
	double samplerate = snd.GetSampleRate();
	double ts = args.fpb / (double)samplerate;
	time_duration += ts;
	double vol = GetVolume();
	bool do_recording = vol > silence_treshold;
	if (!is_recording) {
		if (do_recording) {
			is_recording = true;
		}
	}
	else /*is_recording*/ {
		if (!do_recording) {
			silence_duration += ts;
			if (silence_duration >= silence_timelimit) {
				is_recording = false;
				if (phrase) {
					phrase->Finish();
					phrase = 0;
				}
			}
		}
		else /*do_recording*/ {
			silence_duration = 0;
		}
	}
}


END_UPP_NAMESPACE
