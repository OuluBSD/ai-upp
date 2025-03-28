#ifndef _Sound_SoundDaemon_h_
#define _Sound_SoundDaemon_h_

NAMESPACE_UPP

class SoundDaemon {
public:
	struct ThreadBase : Pte<ThreadBase> {
		Sound snd;
		bool running = false, stopped = true;
		double meter_duration = 0.1;
		double time_duration = 0;
		double silence_duration = 0;
		bool is_silence = false;
		StreamParameters param;
		Ptr<DiscussionManager> mgr;
		Ptr<SoundDiscussion> discussion;
		Ptr<SoundMessage> msg;
		Ptr<SoundPhrase> phrase;
		
		// User params
		double silence_treshold = 0.1;
		double silence_timelimit = 1.0;
		
		Event<void*> WhenFinished;
		Event<String> WhenError;
		
		typedef ThreadBase CLASSNAME;
		ThreadBase();
		virtual ~ThreadBase();
		bool IsRunning() const;
		bool IsOpen() const;
		void OnFinish(void*);
		const Sound& GetSound() const {return snd;}
		void SetDevice(SoundDevice dev, int channels);
		void Start();
		void Stop();
		void Attach(DiscussionManager& m);
		virtual SampleFormat GetSampleFormat() const = 0;
		virtual void ClearData() = 0;
		virtual void* GetDataPtr() = 0;
		virtual double GetVolume() const = 0;
		virtual void RecordCallback(StreamCallbackArgs& args) = 0;
	};
	
	template <class Sample>
	struct Thread : ThreadBase {
		using Clip = SoundClip<Sample>;
		Clip current;
		Clip meter;
		int meter_index = 0;
		
		Thread() {}
		SampleFormat GetSampleFormat() const override {return ::UPP::template GetSampleFormat<Sample>();}
		void ClearData() override {current.Clear(); meter.Clear();}
		void* GetDataPtr() override {return &current;}
		double GetVolume() const override;
		void RecordCallback(StreamCallbackArgs& args) override;
		
		Event<Clip> WhenClipBegin, WhenClipEnd;
	};
private:
	ArrayMap<hash_t, ThreadBase> thrds;
	
public:
	typedef SoundDaemon CLASSNAME;
	SoundDaemon();
	~SoundDaemon();
	
	static SoundDaemon& Static();
	
	ThreadBase& GetAddThread(SoundDevice dev, SampleFormat sample, int channels);
	
	template <class T>
	SoundDaemon::Thread<T>& GetAddThread(SoundDevice dev, int channels, Event<SoundClip<T>> WhenCapture=Event<SoundClip<T>>()) {
		SampleFormat sample = GetSampleFormat<T>();
		CombineHash ch;
		ch.Do(dev);
		ch.Do((int)sample);
		ch.Do(channels);
		hash_t h = ch;
		int i = thrds.Find(h);
		if (i >= 0)
			return dynamic_cast<SoundDaemon::Thread<T>&>(thrds[i]);
		auto* p = new SoundDaemon::Thread<T>();
		p->SetDevice(dev, channels);
		thrds.Add(h, p);
		return *p;
	}

};

#include "SoundDaemon.inl"

END_UPP_NAMESPACE

#endif
