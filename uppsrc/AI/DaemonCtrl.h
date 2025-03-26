#ifndef _AI_DaemonCtrl_h_
#define _AI_DaemonCtrl_h_

NAMESPACE_UPP

class DaemonCtrl : public Ctrl {
	WithSoundDaemon<Ctrl> form;
	TimeCallback tc;
	
	struct VolumeMeterCtrl : Ctrl {
		DaemonCtrl& c;
		VolumeMeterCtrl(DaemonCtrl* c);
		void Paint(Draw& d) override;
	};
	VolumeMeterCtrl meter;
	
	// User params
	double silence_treshold = 0.1;
	double silence_timelimit = 1.0;
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
public:
	typedef DaemonCtrl CLASSNAME;
	DaemonCtrl();
	
	void Data();
};


END_UPP_NAMESPACE

#endif
