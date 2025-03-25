#include "Sound.h"

NAMESPACE_UPP


SoundDaemon::SoundDaemon() {
	snd.WhenFinished = THISBACK(OnFinish);
	
}

SoundDaemon::~SoundDaemon() {
	if (snd.IsOpen())
		snd.Stop();
}

SoundDaemon& SoundDaemon::Static() {
	static SoundDaemon sd;
	return sd;
}

void SoundDaemon::StartRecord(SoundDevice dev) {
	Stop();
	time_duration = 0;
	silence_duration = 0;
	is_silence = true;
	userdata.capture.Clear();
	#if SOUNDDAEMON_8BIT
	SampleFormat fmt = SND_UINT8;
	#else
	SampleFormat fmt = SND_INT16;
	#endif
	StreamParameters param(dev,1,fmt,dev.LowInputLatency);
	snd <<= THISBACK(RecordCallback);
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

void SoundDaemon::OnFinish(void* p) {
	running = false;
	stopped = true;
	WhenFinished(p);
}

void SoundDaemon::RecordCallback(StreamCallbackArgs& args) {
	double ts = args.fpb / (double)samplerate;
	time_duration += ts;
	
	bool was_silence = is_silence;
	double vol = GetVolume();
	bool to_be_silenced = vol <= silence_treshold;
	if (is_silence) {
		if (!to_be_silenced) {
			is_silence = false;
			silence_duration = 0;
		}
	}
	else {
		if (to_be_silenced) {
			silence_duration += ts;
			if (silence_duration >= silence_timelimit) {
				is_silence = true;
				WhenClipEnd();
			}
		}
		else {
			silence_duration = 0;
		}
	}
	
	Data *data = (Data*)args.data;
	int meter_sample_count = max<int>(1, samplerate * meter_duration);
	data->meter_loop.SetCount(meter_sample_count, 0);
	data->meter_index = data->meter_index % meter_sample_count;
	#if SOUNDDAEMON_8BIT
	uint8* meter_begin = data->meter_loop.Begin();
	uint8* meter_end = data->meter_loop.End();
	uint8* meter_it = meter_begin + data->meter_index;
	const uint8 *rptr = (const uint8*)args.input;
	#else
	int16* meter_begin = data->meter_loop.Begin();
	int16* meter_end = data->meter_loop.End();
	int16* meter_it = meter_begin + data->meter_index;
	const int16 *rptr = (const int16*)args.input;
	#endif
	if (!is_silence) {
		if (was_silence) {
			data->index = 0;
			data->capture.Clear();
			WhenClipBegin();
		}
		data->capture.Reserve(data->index + args.fpb);
		if (args.input != NULL) {
			for (uint32 i=0; i<args.fpb; i++) {
				auto v = *rptr++;
				data->capture.Add(v);
				*meter_it++ = v;
				if (meter_it == meter_end)
					meter_it =meter_begin ;
			}
		}
		data->index += args.fpb;
	}
	else {
		if (args.input != NULL) {
			for (uint32 i=0; i<args.fpb; i++) {
				*meter_it++ = *rptr++;
				if (meter_it == meter_end)
					meter_it = meter_begin ;
			}
		}
	}
	data->meter_index = (data->meter_index + args.fpb) % meter_sample_count;
	
	if(!running) {
		args.state=SND_COMPLETE;
		stopped = true;
	}
}

void SoundDaemon::Stop() {
	running = false;
	snd.Stop();
	while (!stopped)
		Sleep(10);
}

double SoundDaemon::GetVolume() const {
	if (userdata.meter_loop.IsEmpty())
		return 0;
	
	if (!userdata.meter_loop.IsEmpty()) {
		int count = userdata.meter_loop.GetCount();
		int64 sum = 0;
		#if SOUNDDAEMON_8BIT
		const uint8* it = userdata.meter_loop.Begin();
		const uint8* it_end = userdata.meter_loop.End();
		while (it != it_end) {
			uint8 val = *it++;
			int c = (int)val - 128;
			sum += c > 0 ? c : -c;
		}
		double av = sum / (double)count / 128.0;
		#else
		const int16* it = userdata.meter_loop.Begin();
		const int16* it_end = userdata.meter_loop.End();
		while (it != it_end) {
			int16 val = *it++;
			sum += (int64)val;
		}
		double av16 = sum / (double)count;
		double av = av16 / (double)INT16_MAX;
		#endif
		return av;
	}
	return 0;
}


END_UPP_NAMESPACE
