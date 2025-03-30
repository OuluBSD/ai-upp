#ifndef _AI_DaemonCtrl_h_
#define _AI_DaemonCtrl_h_

NAMESPACE_UPP

class DaemonCtrl : public Ctrl {
	WithSoundDaemon<Ctrl> form;
	ArrayCtrl discussions, messages, phrases;
	Splitter split[3];
	WithSoundDaemonClip<Ctrl> waveform;
	TimeCallback tc;
	WaveformCtrl wavectrl;
	
	struct VolumeMeterCtrl : Ctrl {
		DaemonCtrl& c;
		VolumeMeterCtrl(DaemonCtrl* c);
		void Paint(Draw& d) override;
	};
	VolumeMeterCtrl meter;
	Ptr<SoundDaemon::ThreadBase> thrd;
	
	void EnableMeter();
	void DisableMeter();
	void OnStart();
	void Stop();
	void PopulateSrc();
	void OnCapture(SoundClip<uint8> data);
	void OnRecord();
	void OnFinish(void*);
	void OnError(String s);
	void ClearWaveform();
public:
	typedef DaemonCtrl CLASSNAME;
	DaemonCtrl();
	
	void Data();
	void DataManager();
	void DataDiscussion();
	void DataMessage();
	void DataPhrase();
};


END_UPP_NAMESPACE

#endif
