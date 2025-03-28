#include "AI.h"
#include <ide/ide.h>

NAMESPACE_UPP

DaemonCtrl::DaemonCtrl() : meter(this) {
	CtrlLayout(form);
	Add(form.SizePos());
	form.volume.Add(meter.SizePos());
	form.autostart.Set(TheIde()->autostart_audio_src);
	form.split.Horz() << discussions << messages << phrases;
	
	discussions.AddColumn("#");
	discussions.AddColumn("Discussion");
	discussions.AddIndex("IDX");
	messages.AddColumn("#");
	messages.AddColumn("Message");
	messages.AddIndex("IDX");
	phrases.AddColumn("#");
	phrases.AddColumn("Phrase");
	phrases.AddIndex("IDX");
	
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
		silence_timelimit = t;
		form.time_val.SetData(t);
	};
	form.time_val.WhenEnter = [this]{
		double t = form.time_val.GetData();
		TheIde()->audio_timelimit = t;
		silence_timelimit = t;
		form.time_slide.SetData(t * 100);
	};
	
	// Silence detection volume treshold
	form.volume_slide.MinMax(1,100);
	form.volume_slide.SetData(ide->audio_volumetreshold * 100);
	form.volume_val.SetData(ide->audio_volumetreshold);
	form.volume_slide.WhenSlideFinish =[this]{
		double t = (double)form.volume_slide.GetData() * 0.01;
		TheIde()->audio_volumetreshold = t;
		this->silence_treshold = t;
		form.volume_val.SetData(t);
	};
	form.volume_val.WhenEnter = [this]{
		double t = form.volume_val.GetData();
		TheIde()->audio_volumetreshold = t;
		this->silence_treshold = t;
		form.volume_slide.SetData(t * 100);
	};
	
	silence_timelimit = TheIde()->audio_timelimit;
	silence_treshold = TheIde()->audio_volumetreshold;
	
	PopulateSrc();
}

void DaemonCtrl::Data() {
	
	DataManager();
}

void DaemonCtrl::DataManager() {
	AiDiscussionManager& dm = AiDiscussionManager::Single();
	
	for(int i = 0; i < dm.discussions.GetCount(); i++) {
		const SoundDiscussion& sd = dm.discussions[i];
		discussions.Set(i, 0, i);
		discussions.Set(i, "IDX", i);
	}
	discussions.SetCount(dm.discussions.GetCount());
	if (!discussions.IsCursor() && discussions.GetCount())
		discussions.SetCursor(0);
	
}

void DaemonCtrl::DataDiscussion() {
	AiDiscussionManager& dm = AiDiscussionManager::Single();
	
	if (!discussions.IsCursor()) {
		messages.Clear();
		phrases.Clear();
		return;
	}
	int dis_i = discussions.Get("IDX");
	const SoundDiscussion& sd = dm.discussions[dis_i];
	
	for(int i = 0; i < sd.messages.GetCount(); i++) {
		const auto& it = sd.messages[i];
		messages.Set(i, 0, i);
		messages.Set(i, "IDX", i);
	}
	messages.SetCount(sd.messages.GetCount());
	if (!messages.IsCursor() && messages.GetCount())
		messages.SetCursor(0);
	
}

void DaemonCtrl::DataMessage() {
	AiDiscussionManager& dm = AiDiscussionManager::Single();
	
	if (!discussions.IsCursor() || !messages.IsCursor()) {
		phrases.Clear();
		return;
	}
	
	int dis_i = discussions.Get("IDX");
	const SoundDiscussion& sd = dm.discussions[dis_i];
	int msg_i = messages.Get("IDX");
	const SoundMessage& sm = sd.messages[msg_i];
	
	for(int i = 0; i < sm.phrases.GetCount(); i++) {
		const auto& it = sm.phrases[i];
		phrases.Set(i, 0, i);
		phrases.Set(i, "IDX", i);
	}
	phrases.SetCount(sm.phrases.GetCount());
	if (!phrases.IsCursor() && phrases.GetCount())
		phrases.SetCursor(0);
	
	
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

void DaemonCtrl::OnCapture(SoundClip<uint8> data) {
	LOG("DATA");
	TODO
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
	hash_t stream_hash = dev.GetHashValue();
	thrd = &sd.template GetAddThread<uint8>(dev, 1, THISBACK(OnCapture));
	
	AiDiscussionManager& aidm = AiDiscussionManager::Single();
	thrd->Attach(aidm);
	
	Ptr<Ctrl> p = this;
	thrd->WhenFinished = [this,p](void* arg){if (p) OnFinish(arg);};
	
	thrd->Start();
	
	OnStart();
}

void DaemonCtrl::OnStart() {
	form.rec.SetImage(Image());
	form.rec.SetLabel(t_("Stop"));
	form.rec.WhenAction = THISBACK(Stop);
	EnableMeter();
}

void DaemonCtrl::Stop() {
	if (thrd) {
		thrd->Stop();
		OnFinish(0);
	}
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

DaemonCtrl::VolumeMeterCtrl::VolumeMeterCtrl(DaemonCtrl* c) : c(*c) {
	SetFrame(InsetFrame());
}

void DaemonCtrl::VolumeMeterCtrl::Paint(Draw& d) {
	Size sz = GetSize();
	d.DrawRect(sz, White());
	if (c.thrd) {
		double vol = c.thrd->GetVolume();
		int h = sz.cy * vol;
		Color clr = Blue();
		d.DrawRect(RectC(0,sz.cy-h,sz.cx,h), clr);
	}
}


END_UPP_NAMESPACE
