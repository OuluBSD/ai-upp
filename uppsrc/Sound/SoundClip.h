#ifndef _Sound_SoundClip_h_
#define _Sound_SoundClip_h_

NAMESPACE_UPP

struct SoundClipBase : Pte<SoundClipBase> {
	virtual ~SoundClipBase() {}
};

template <class Sample>
struct SoundClip : Moveable<SoundClip<Sample>>, SoundClipBase {
	struct Data {
		Vector<Sample> data;
		int samplerate = 0;
		int channels = 0;
		Atomic refs;
		Data() {refs = 0;}
		void SetFormat(int samplerate, int channels) {this->channels = channels; this->samplerate = samplerate;}
	};
	Data* data = 0;
	
	SoundClip() {}
	SoundClip(const SoundClip& c) {*this = c;}
	SoundClip(SoundClip&& c) {data = c.data; c.data = 0;}
	void Clear() {if (data) {data->refs-=1; if (data->refs <= 0) delete data; data = 0;}}
	void Create() {Clear(); data = new Data(); data->refs++;}
	void operator=(const SoundClip& c) {Clear(); data = c.data; if (data) data->refs++;}
	bool IsEmpty() const {return !data || data->data.IsEmpty();}
	bool IsNull() const {return !data;}
	int GetCount() const {return data ? data->data.GetCount() : 0;}
	Sample* Begin() {return data ? data->data.Begin() : 0;}
	Sample* End() {return data ? data->data.End() : 0;}
	const Sample* Begin() const {return data ? data->data.Begin() : 0;}
	const Sample* End() const {return data ? data->data.End() : 0;}
	bool Read(Vector<float>& out);
	bool Write(Vector<float>& out);
	static SampleFormat GetSampleFormat() {return ::UPP::GetSampleFormat<Sample>();}
};

END_UPP_NAMESPACE

#endif
