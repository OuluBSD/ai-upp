#ifndef _AI_DaemonCtrl_h_
#define _AI_DaemonCtrl_h_

NAMESPACE_UPP

class DaemonCtrl : public Ctrl {
	WithSoundDaemon<Ctrl> form;
	TimeCallback tc;
	
	struct VolumeMeterCtrl : Ctrl {
		VolumeMeterCtrl();
		void Paint(Draw& d) override;
	};
	VolumeMeterCtrl meter;
	
	
	void EnableMeter();
	void DisableMeter();
	void OnStart();
	void Stop();
	void PopulateSrc();
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
