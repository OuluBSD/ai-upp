#include "Sound.h"

NAMESPACE_UPP

SoundDaemon::ThreadBase::ThreadBase(SoundDaemon& owner) : owner(owner) {
	snd.WhenFinished = THISBACK(OnFinish);
	
}

SoundDaemon::ThreadBase::~ThreadBase() {
	if (snd.IsOpen())
		snd.Stop();
}

void SoundDaemon::ThreadBase::Start() {
	ASSERT(param.device >= 0);
	Stop();
	time_duration = 0;
	silence_duration = 0;
	is_recording = false;
	ClearData();
	
	snd <<= THISBACK(RecordCallback);
	
	void* userdata = GetDataPtr();
	snd.Open(&userdata, param, Null);
	if(snd.IsError()){
		LOG(snd.GetError());
		WhenError(snd.GetError());
		return;
	}
	snd.Start();
	if(snd.IsError()){
		LOG(snd.GetError());
		WhenError(snd.GetError());
		return;
	}
	running = true;
	stopped = false;
}

void SoundDaemon::ThreadBase::SetDevice(SoundDevice dev, int channels) {
	SampleFormat fmt = GetSampleFormat();
	this->param = StreamParameters(dev,channels,fmt,dev.LowInputLatency);
}

bool SoundDaemon::ThreadBase::IsRunning() const {
	return running;
}

bool SoundDaemon::ThreadBase::IsOpen() const {
	return snd.IsOpen();
}

void SoundDaemon::ThreadBase::OnFinish(void* p) {
	running = false;
	stopped = true;
	WhenFinished(p);
}

void SoundDaemon::ThreadBase::Stop() {
	running = false;
	snd.Stop();
	while (!stopped)
		Sleep(10);
}

void SoundDaemon::ThreadBase::Attach(DiscussionManager& m) {
	mgr = &m;
	discussion = &mgr->Add();
	msg = &discussion->Add();
	phrase = 0;
}


END_UPP_NAMESPACE
