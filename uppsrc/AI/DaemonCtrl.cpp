#include "AI.h"
#include <ide/ide.h>

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
	StreamParameters param(dev,1,SND_UINT8,dev.LowInputLatency);
	snd <<= THISBACK(RecordCallback);
	snd.Open(&userdata, param, Null);
	if(snd.IsError()){
		WhenError(snd.GetError());
		return;
	}
	snd.Start();
	if(snd.IsError()){
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
	uint8* meter_begin = data->meter_loop.Begin();
	uint8* meter_end = data->meter_loop.End();
	uint8* meter_it = meter_begin + data->meter_index;
	const uint8 *rptr = (const uint8*)args.input;
	if (!is_silence) {
		if (was_silence) {
			data->index = 0;
			data->capture.Clear();
			WhenClipBegin();
		}
		data->capture.Reserve(data->index + args.fpb);
		if (args.input != NULL)
			for (int i=0; i<args.fpb; i++) {
				uint8 v = *rptr++;
				data->capture.Add(v);
				*meter_it++ = v;
				if (meter_it == meter_end)
					meter_it =meter_begin ;
			}
		data->index += args.fpb;
	}
	else {
		if (args.input != NULL)
			for (int i=0; i<args.fpb; i++) {
				*meter_it++ = *rptr++;
				if (meter_it == meter_end)
					meter_it = meter_begin ;
			}
	}
	data->meter_index = (data->meter_index + args.fpb) % meter_sample_count;
	
	if(!running)
		args.state=SND_COMPLETE;
}

void SoundDaemon::Stop() {
	running = false;
	while (!stopped)
		Sleep(10);
}

double SoundDaemon::GetVolume() const {
	if (userdata.meter_loop.IsEmpty())
		return 0;
	
	if (!userdata.meter_loop.IsEmpty()) {
		int count = userdata.meter_loop.GetCount();
		int64 sum = 0;
		const uint8* it = userdata.meter_loop.Begin();
		const uint8* it_end = userdata.meter_loop.End();
		while (it != it_end) {
			uint8 val = *it++;
			int c = (int)val - 128;
			sum += c > 0 ? c : -c;
		}
		double av = sum / (double)count / 128.0;
		return av;
	}
	return 0;
}





DaemonCtrl::DaemonCtrl() {
	CtrlLayout(form);
	Add(form.SizePos());
	form.volume.Add(meter.SizePos());
	
	form.autostart.Set(TheIde()->autostart_audio_src);
	
	Ptr<Ctrl> p = this;
	auto* ide = TheIde();
	
	OnFinish(0);
	form.refresh <<= THISBACK(PopulateSrc);
	form.autostart.WhenAction = [this]{TheIde()->autostart_audio_src = form.autostart.Get();};
	
	// Silence detection time limit
	form.time_slide.MinMax(2,500);
	form.time_slide.SetData(ide->audio_timelimit * 100);
	form.time_val.SetData(ide->audio_timelimit);
	form.time_slide.WhenSlideFinish =[this]{
		double t = (double)form.time_slide.GetData() * 0.01;
		TheIde()->audio_timelimit = t;
		SoundDaemon::Static().silence_timelimit = t;
		form.time_val.SetData(t);
	};
	form.time_val.WhenEnter = [this]{
		double t = form.time_val.GetData();
		TheIde()->audio_timelimit = t;
		SoundDaemon::Static().silence_timelimit = t;
		form.time_slide.SetData(t * 100);
	};
	
	// Silence detection volume treshold
	form.volume_slide.MinMax(1,100);
	form.volume_slide.SetData(ide->audio_volumetreshold * 100);
	form.volume_val.SetData(ide->audio_volumetreshold);
	form.volume_slide.WhenSlideFinish =[this]{
		double t = (double)form.volume_slide.GetData() * 0.01;
		TheIde()->audio_volumetreshold = t;
		SoundDaemon::Static().silence_treshold = t;
		form.volume_val.SetData(t);
	};
	form.volume_val.WhenEnter = [this]{
		double t = form.volume_val.GetData();
		TheIde()->audio_volumetreshold = t;
		SoundDaemon::Static().silence_treshold = t;
		form.volume_slide.SetData(t * 100);
	};
	
	SoundDaemon& sd = SoundDaemon::Static();
	sd.WhenFinished = [this,p](void* arg){if (p) OnFinish(arg);};
	sd.silence_timelimit = TheIde()->audio_timelimit;
	sd.silence_treshold = TheIde()->audio_volumetreshold;
	
	if (sd.GetSound().IsOpen())
		OnStart();
	
	PopulateSrc();
}

void DaemonCtrl::Data() {
	
}

void DaemonCtrl::PopulateSrc() {
	const SoundSystem& s=SoundSys();
	form.src.Clear();
	form.src->SetRoot(Null,-1,"Use default input device");
	for(int i=0;i<s.GetAPICount();i++){
		SoundAPI a=s.GetAPI(i);
		form.src->Add(0,AIImages::API(),a.index,a.name + String(" (default input device)"),true);
	}
	for(int i=0;i<s.GetCount();i++){
		SoundDevice d=s[i];
		if(d.InputChannels)
			form.src->Add(form.src->Find(d.API),AIImages::Device(),d.index+1024,d.name,false);
	}
	form.src->OpenDeep(0);
	
	Ide* ide = TheIde();
	form.src <<= ide->audio_src;
}

void DaemonCtrl::OnRecord() {
	form.error.Clear();
	
	SoundDevice dev;
	int n=~form.src;
	if(n==-1)
		dev=SoundSys().GetDefaultInput();
	else if(n<1024)
		dev=SoundSys().GetAPI(n).defaultInputDevice;
	else
		dev=SoundSys().GetDevices()[n-1024];
	if (IsNull(dev)){
		form.error <<= "[1 Error: No default input device.";
		return;
	}
	
	Ide* ide = TheIde();
	ide->audio_src = n;
	
	SoundDaemon& sd = SoundDaemon::Static();
	sd.StartRecord(dev);
	OnStart();
}

void DaemonCtrl::OnStart() {
	form.rec.SetImage(Image());
	form.rec.SetLabel(t_("Stop"));
	form.rec.WhenAction = THISBACK(Stop);
	EnableMeter();
}

void DaemonCtrl::Stop() {
	SoundDaemon& sd = SoundDaemon::Static();
	sd.Stop();
	OnFinish(0);
}

void DaemonCtrl::EnableMeter() {
	tc.Set(-100, [this]{meter.Refresh();});
}

void DaemonCtrl::DisableMeter() {
	tc.Kill();
}

void DaemonCtrl::OnError(String s) {
	form.error <<= "[1 Error: " + s;
	DisableMeter();
}

void DaemonCtrl::OnFinish(void*) {
	GuiLock __;
	form.rec.SetImage(AIImages::Record());
	form.rec.SetLabel(t_("Record"));
	form.rec.WhenAction = THISBACK(OnRecord);
	DisableMeter();
}

DaemonCtrl::VolumeMeterCtrl::VolumeMeterCtrl() {
	SetFrame(InsetFrame());
}

void DaemonCtrl::VolumeMeterCtrl::Paint(Draw& d) {
	Size sz = GetSize();
	d.DrawRect(sz, White());
	double vol = SoundDaemon::Static().GetVolume();
	int h = sz.cy * vol;
	Color clr = Blue();
	d.DrawRect(RectC(0,sz.cy-h,sz.cx,h), clr);
}


END_UPP_NAMESPACE
