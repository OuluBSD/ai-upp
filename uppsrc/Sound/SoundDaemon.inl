
template <class Sample>
void SoundDaemon::Thread<Sample>::RecordCallback(StreamCallbackArgs& args) {
	double samplerate = snd.GetSampleRate();
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
				if (phrase) {
					phrase->Finish();
					phrase = 0;
				}
				WhenClipEnd(current);
			}
		}
		else {
			silence_duration = 0;
		}
	}
	
	Clip *data = &current;// (Clip*)args.data;
	if (!data) return;
	
	if (meter.IsNull())
		meter.Create();
	Clip::Data& meter = *this->meter.data;
	int meter_sample_count = max<int>(1, samplerate * meter_duration);
	meter.data.SetCount(meter_sample_count, 0);
	meter_index = meter_index % meter_sample_count;
	Sample* meter_begin = meter.data.Begin();
	Sample* meter_end = meter.data.End();
	Sample* meter_it = meter_begin + meter_index;
	const Sample *rptr = (const Sample*)args.input;
	if (!is_silence) {
		if (was_silence) {
			data->Create();
			Clip::Data& cdata = *data->data;
			cdata.channels = 1;
			cdata.samplerate = samplerate;
			WhenClipBegin(current);
			if (msg) {
				phrase = &msg->Add();
				phrase->BeginClip(*data);
			}
		}
		Clip::Data& cdata = *data->data;
		int index = cdata.data.GetCount();
		cdata.data.Reserve(index + args.fpb);
		if (args.input != NULL) {
			for (uint32 i=0; i<args.fpb; i++) {
				auto v = *rptr++;
				cdata.data.Add(v);
				*meter_it++ = v;
				if (meter_it == meter_end)
					meter_it =meter_begin ;
			}
		}
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
	meter_index = (meter_index + args.fpb) % meter_sample_count;
	
	if(!running) {
		args.state=SND_COMPLETE;
		stopped = true;
	}
}

template<class Sample>
double SoundDaemon::Thread<Sample>::GetVolume() const {
	if (meter.IsEmpty())
		return 0;
	
	if (!meter.IsEmpty()) {
		int count = meter.GetCount();
		double sum = 0;
		const Sample* it = meter.Begin();
		const Sample* it_end = meter.End();
		while (it != it_end) {
			Sample val = *it++;
			double dbl = SampleToDouble<Sample>(val);
			sum += dbl;
		}
		double av = sum / (double)count;
		return av;
	}
	return 0;
}

