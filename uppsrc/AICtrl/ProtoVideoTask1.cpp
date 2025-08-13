#include "AICtrl.h"

/*
- outputit / videon ai käyttö
	- sora presetti
	- slideshow promptit
	- tiktok / instagram / youtube description
	- mainosviesti: X / facebook / threads / bluesky
	- kuva-generaattori prompt
	- englanniksi käännetyt lyriikat
	- video lyrics tekstit
- inputit
	- video-idea / tausta-info
	- lyriikka ja sunossa käytetty tyyli
	- numero (päivä/video)
	- onko: itse-tehty (sävellys / sanoitus / sovitus)
	- onko: 100% alkuperäinen (sävellys / sanoitus / sovitus)
	- milloin tehty: (sävellys / sanoitus / sovitus)
	- extra infoa / ajatuslause / jne.
- prosessit
	- query + response hardwired

*/

NAMESPACE_UPP


ProtoVideoTask1Ctrl::ProtoVideoTask1Ctrl() {
	Add(pages.SizePos());
	
	CtrlLayout(input);
	CtrlLayout(sora_preset);
	CtrlLayout(slideshow_prompts);
	CtrlLayout(descriptions);
	
	pages.Add(input, "Input");
	pages.Add(sora_preset, "Sora preset");
	pages.Add(slideshow_prompts, "Slideshow prompts");
	pages.Add(descriptions, "Descriptions");
	
	input.title.WhenAction = THISBACK(OnChange);
	input.authors_arrangement.WhenAction = THISBACK(OnChange);
	input.authors_composition.WhenAction = THISBACK(OnChange);
	input.authors_lyrics.WhenAction = THISBACK(OnChange);
	input.extra.WhenAction = THISBACK(OnChange);
	input.lyrics.WhenAction = THISBACK(OnChange);
	input.musicstyle.WhenAction = THISBACK(OnChange);
	input.num.WhenAction = THISBACK(OnChange);
	input.original_arrangement.WhenAction = THISBACK(OnChange);
	input.original_composition.WhenAction = THISBACK(OnChange);
	input.original_lyrics.WhenAction = THISBACK(OnChange);
	input.selfmade_arrangement.WhenAction = THISBACK(OnChange);
	input.selfmade_composition.WhenAction = THISBACK(OnChange);
	input.selfmade_lyrics.WhenAction = THISBACK(OnChange);
	input.videoidea.WhenAction = THISBACK(OnChange);
	input.videolyrics.WhenAction = THISBACK(OnChange);
	input.whenmade_arrangement.WhenAction = THISBACK(OnChange);
	input.whenmade_composition.WhenAction = THISBACK(OnChange);
	input.whenmade_lyrics.WhenAction = THISBACK(OnChange);
	input.whenmade_lyrics_today.WhenAction = THISBACK(OnChange);
	
	sora_preset.refresh.WhenAction = THISBACK(DoSora);
	slideshow_prompts.refresh.WhenAction = THISBACK(DoSlideshowPrompts);
	descriptions.refresh.WhenAction = THISBACK(DoDescriptions);
	
}

ProtoVideoTask1Ctrl::~ProtoVideoTask1Ctrl() {
	Save();
}

void ProtoVideoTask1Ctrl::Initialize(Value args) {
	// ??
}

void ProtoVideoTask1Ctrl::OnChange() {
	tc.Set(500, THISBACK(Save));
}

void ProtoVideoTask1Ctrl::Load() {
	ProtoVideoTask1& comp = this->GetExt<ProtoVideoTask1>();
	
	if (comp.val.value.Is<ValueMap>()) {
		ValueMap map = comp.val.value;
		#define LOAD(x) input.x.SetData(map.Get(#x, Value()));
		LOAD(title)
		LOAD(authors_arrangement)
		LOAD(authors_composition)
		LOAD(authors_lyrics)
		LOAD(extra)
		LOAD(lyrics)
		LOAD(musicstyle)
		LOAD(num)
		LOAD(original_arrangement)
		LOAD(original_composition)
		LOAD(original_lyrics)
		LOAD(selfmade_arrangement)
		LOAD(selfmade_composition)
		LOAD(selfmade_lyrics)
		LOAD(videoidea)
		LOAD(videolyrics)
		LOAD(whenmade_arrangement)
		LOAD(whenmade_composition)
		LOAD(whenmade_lyrics)
		LOAD(whenmade_lyrics_today)
		#undef LOAD
		
		sora_preset.txt.SetData(map.Get("sora_preset", Value()));
		slideshow_prompts.txt.SetData(map.Get("slideshow_prompts", Value()));
		descriptions.txt.SetData(map.Get("descriptions", Value()));
		
	}
	
}

void ProtoVideoTask1Ctrl::Save() {
	ProtoVideoTask1& comp = this->GetExt<ProtoVideoTask1>();
	
	if (!comp.val.value.Is<ValueMap>())
		comp.val.value = ValueMap();
	ValueMap map = comp.val.value;
	#define SAVE(x) map.Set(#x, input.x.GetData());
	SAVE(title)
	SAVE(authors_arrangement)
	SAVE(authors_composition)
	SAVE(authors_lyrics)
	SAVE(extra)
	SAVE(lyrics)
	SAVE(musicstyle)
	SAVE(num)
	SAVE(original_arrangement)
	SAVE(original_composition)
	SAVE(original_lyrics)
	SAVE(selfmade_arrangement)
	SAVE(selfmade_composition)
	SAVE(selfmade_lyrics)
	SAVE(videoidea)
	SAVE(videolyrics)
	SAVE(whenmade_arrangement)
	SAVE(whenmade_composition)
	SAVE(whenmade_lyrics)
	SAVE(whenmade_lyrics_today)
	#undef SAVE
	
	map.Set("sora_preset", sora_preset.txt.GetData());
	map.Set("slideshow_prompts", slideshow_prompts.txt.GetData());
	map.Set("descriptions", descriptions.txt.GetData());
	
	comp.val.value = map;
}

void ProtoVideoTask1Ctrl::Data() {
	Load();
}

void ProtoVideoTask1Ctrl::ToolMenu(Bar& bar) {
	
}

void ProtoVideoTask1Ctrl::UpdateData() {
	
}

void ProtoVideoTask1Ctrl::DoSora() {
	TaskMgr& m = AiTaskManager();
	
	if (flag.IsRunning()) {
		PromptOK("Task already running");
		return;
	}
	
	Save();
	ProtoVideoTask1& comp = this->GetExt<ProtoVideoTask1>();
	ValueMap map = comp.val.value;
	
	flag.Start();
	
	Vector<String> keys;
	keys << "extra" << "musicstyle" << "lyrics";
	
	String input;
	for (String key : keys)
		if (map.Find(key) >= 0)
			input += (String)map.Get(key, "") + "\n\n";
	
	if (input.IsEmpty()) {
		PromptOK("Error: empty output");
		flag.Stop();
		return;
	}
	
	TaskArgs args;
	args.fn = FN_SORA_PRESETS;
	args.params("input") = input;
	
	Ptr<Ctrl> c = this;
	m.GetJson(args, [this,c](String s) {
		if (c) c->PostCallback([this,s,c]{
			if (!c) return;
			ProtoVideoTask1& comp = this->GetExt<ProtoVideoTask1>();
			ValueMap map = comp.val.value;
			map.Set("sora_preset", s);
			comp.val.value = map;
			
			sora_preset.txt.SetData(s);
			flag.SetStopped();
		});
	}, "Sora preset");
}

void ProtoVideoTask1Ctrl::DoSlideshowPrompts() {
	TaskMgr& m = AiTaskManager();
	
	if (flag.IsRunning()) {
		PromptOK("Task already running");
		return;
	}
	
	Save();
	ProtoVideoTask1& comp = this->GetExt<ProtoVideoTask1>();
	ValueMap map = comp.val.value;
	
	flag.Start();
	
	Vector<String> keys;
	keys << "sora_preset" << "extra" << "musicstyle" << "lyrics";
	
	String input;
	for (String key : keys)
		if (map.Find(key) >= 0)
			input += (String)map.Get(key, "") + "\n\n";
	
	if (input.IsEmpty()) {
		PromptOK("Error: empty output");
		flag.Stop();
		return;
	}
	
	TaskArgs args;
	args.fn = FN_SLIDESHOW_PROMPTS;
	args.params("input") = input;
	
	Ptr<Ctrl> c = this;
	m.GetJson(args, [this,c](String s) {
		if (c) c->PostCallback([this,s,c]{
			if (!c) return;
			ProtoVideoTask1& comp = this->GetExt<ProtoVideoTask1>();
			ValueMap map = comp.val.value;
			map.Set("slideshow_prompts", s);
			comp.val.value = map;
			
			slideshow_prompts.txt.SetData(s);
			flag.SetStopped();
		});
	}, "Sora preset");
}

void ProtoVideoTask1Ctrl::DoDescriptions() {
	TaskMgr& m = AiTaskManager();
	
	if (flag.IsRunning()) {
		PromptOK("Task already running");
		return;
	}
	
	Save();
	ProtoVideoTask1& comp = this->GetExt<ProtoVideoTask1>();
	ValueMap map = comp.val.value;
	
	flag.Start();
	
	Vector<String> keys;
	keys << "slideshow_prompts" << "extra" << "musicstyle" << "lyrics";
	
	String input;
	
	int year = map.Get("whenmade_composition", 0);
	int born = 1989;
	int age = max(0, min(100, year - born));
	
	input += "I'm releasing my old songs one a day.\n";
	input += Format("This is the song number %d.", map.Get("num", Value(0)));
	input += Format("I'm releasing a composition from %d, when I was %d years old.\n", year, age);
	
	if (map.Get("whenmade_lyrics_today", false))
		input += "I made the lyrics today.\n";
	
	if (map.Get("original_composition", false))
		input += "The composition of the song is made by hand without any use of AI.\n";
	
	if (map.Get("original_lyrics", false))
		input += "The lyrics of the song is made by hand without any use of AI.\n";
	
	if (map.Get("original_arrangement", false))
		input += "The arrangement of the song is made by hand without any use of AI.\n";
	
	input += "The video is generated by Sora AI.\n";
	input += "I edited the video myself by hand.\n";
	input += "\n\n";
	
	input += "Title: \"" + map.Get("title", Value()).ToString() + "\"\n";
	
	for (String key : keys)
		if (map.Find(key) >= 0)
			input += (String)map.Get(key, "") + "\n\n";
	
	if (input.IsEmpty()) {
		PromptOK("Error: empty output");
		flag.Stop();
		return;
	}
	
	TaskArgs args;
	args.fn = FN_VIDEO_WEBSITE_DESCRIPTIONS;
	args.params("input") = input;
	
	Ptr<Ctrl> c = this;
	m.GetJson(args, [this,c](String s) {
		if (c) c->PostCallback([this,s,c]{
			if (!c) return;
			ProtoVideoTask1& comp = this->GetExt<ProtoVideoTask1>();
			ValueMap map = comp.val.value;
			map.Set("descriptions", s);
			comp.val.value = map;
			
			descriptions.txt.SetData(s);
			flag.SetStopped();
		});
	}, "Sora preset");
}


INITIALIZER_COMPONENT_CTRL(ProtoVideoTask1, ProtoVideoTask1Ctrl)


END_UPP_NAMESPACE
