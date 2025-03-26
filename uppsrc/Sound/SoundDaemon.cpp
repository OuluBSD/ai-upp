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
		auto& t = thrds.Add(h, new SoundDaemon::Thread<uint8>());
		t.SetDevice(dev, channels);
		return t;
	}
	else if (sample == paInt16) {
		auto& t = thrds.Add(h, new SoundDaemon::Thread<int16>());
		t.SetDevice(dev, channels);
		return t;
	}
	else {
		TODO
		static SoundDaemon::Thread<int16> t;
		return t;
	}
}


END_UPP_NAMESPACE
