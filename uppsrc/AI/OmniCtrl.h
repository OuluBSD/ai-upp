#ifndef _AI_OmniCtrl_h_
#define _AI_OmniCtrl_h_

NAMESPACE_UPP


struct OmniDetailedCtrl : Ctrl {
	Splitter hsplit;
	ArrayCtrl discussions, messages, phrases;
	Splitter split[3];
	WithSoundDaemonClip<Ctrl> waveform;
	WaveformCtrl wavectrl;
	Ptr<SoundThreadBase> thrd;
	
	typedef OmniDetailedCtrl CLASSNAME;
	OmniDetailedCtrl();
	void SetThread(Ptr<SoundThreadBase> thrd);
	void Data();
	void DataManager();
	void DataDiscussion();
	void DataMessage();
	void DataPhrase();
	void ClearWaveform();
};


struct OmniSoundIO : Ctrl {
	struct VolumeMeterCtrl : Ctrl {
		OmniSoundIO& c;
		VolumeMeterCtrl(OmniSoundIO* c);
		void Paint(Draw& d) override;
	};
	
	WithSoundDaemon<Ctrl> form;
	VolumeMeterCtrl meter;
	Ptr<SoundThreadBase> thrd;
	TimeCallback tc;
	
	typedef OmniSoundIO CLASSNAME;
	OmniSoundIO();
	void Data();
	void SetThread(Ptr<SoundThreadBase> thrd);
	void PopulateSrc();
	void OnFinish(void*);
	void EnableMeter();
	void DisableMeter();
	void OnStart();
	void Stop();
	void OnRecord();
	void OnError(String s);
};

class OmniCtrl : public TabCtrl {
	OmniDetailedCtrl detailed;
	OmniSoundIO snd;
	
public:
	typedef OmniCtrl CLASSNAME;
	OmniCtrl();
	
	void Data();
	void SetThread(Ptr<SoundThreadBase> thrd);
};


END_UPP_NAMESPACE

#endif
