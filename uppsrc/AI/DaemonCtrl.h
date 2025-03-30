#ifndef _AI_DaemonCtrl_h_
#define _AI_DaemonCtrl_h_

NAMESPACE_UPP

class DaemonCtrl : public Ctrl {
	WithSoundDaemon<Ctrl> form;
	ArrayCtrl discussions, messages, phrases;
	WithSoundDaemonClip<Ctrl> waveform;
	TimeCallback tc;
	
	struct VolumeMeterCtrl : Ctrl {
		DaemonCtrl& c;
		VolumeMeterCtrl(DaemonCtrl* c);
		void Paint(Draw& d) override;
	};
	VolumeMeterCtrl meter;
	Ptr<SoundDaemon::ThreadBase> thrd;
	
	// User params
	double silence_treshold = 0.1;
	double silence_timelimit = 1.0;
	
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
