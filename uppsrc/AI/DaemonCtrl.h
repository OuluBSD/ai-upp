#ifndef _AI_DaemonCtrl_h_
#define _AI_DaemonCtrl_h_

NAMESPACE_UPP

#define SOUNDDAEMON_8BIT 1

class SoundDaemon {
	#if SOUNDDAEMON_8BIT
	typedef uint8 Sample;
	#else
	typedef int16 Sample;
	#endif
	int samplerate = 44100;
	Sound snd;
	struct Data{
		int index;  /* Index into sample array. */
		Vector<SoundDaemon::Sample> capture;
		Vector<SoundDaemon::Sample> meter_loop;
		int meter_index = 0;
	};
	Data userdata;
	bool running = false, stopped = true;
	double meter_duration = 0.1;
	double time_duration = 0;
	double silence_duration = 0;
	bool is_silence = false;
	
	
public:
	typedef SoundDaemon CLASSNAME;
	SoundDaemon();
	~SoundDaemon();
	void StartRecord(SoundDevice dev);
	void Stop();
	void RecordCallback(StreamCallbackArgs& args);
	double GetVolume() const;
	bool IsRunning() const {return running;}
	void OnFinish(void*);
	
	const Sound& GetSound() const {return snd;}
	
	static SoundDaemon& Static();
	
	Event<void*> WhenFinished;
	Event<String> WhenError;
	Event<> WhenClipBegin, WhenClipEnd;
	
	// User params
	double silence_treshold = 0;
	double silence_timelimit = 0;
};

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
