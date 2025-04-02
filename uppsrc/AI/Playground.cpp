#include "AI.h"

NAMESPACE_UPP


CompletionCtrl::CompletionCtrl() {
	CtrlLayout(*this);
	
	model.Add("gpt-3.5-turbo-instruct-0914");
	model.Add("gpt-3.5-turbo-instruct");
	model.Add("babbage-002");
	model.Add("davinci-002");
	model.SetIndex(0);
	temperature.SetData(1);
	max_length.SetData(2048);
	top_p.SetData(1);
	freq_penalty.SetData(0);
	presence_penalty.SetData(0);
	best_of.SetData(1);
	show_probs.Add("Off");
	show_probs.Add("Most likely");
	show_probs.Add("Least likely");
	show_probs.Add("Full spectrum");
	show_probs.SetIndex(0);
	submit <<= THISBACK(Submit);
}

void AiThreadCtrlBase::SetThread(AiThread& t) {
	ai_thrd = &t;
}



void CompletionCtrl::Submit() {
	if (!HasThread()) return;
	CompletionThread& t = GetCompletionThread();
	TaskMgr& m = AiTaskManager();
	
	String txt = this->text.GetData();
	
	m.RawCompletion(txt, [this,txt](String res) {
		String new_data = txt + res;
		GuiLock __;
		this->text.SetData(new_data);
	});
}


TextToSpeechCtrl::TextToSpeechCtrl() {
	CtrlLayout(*this);
	
	model.Add("Alloy");
	model.Add("Ash");
	model.Add("Coral");
	model.Add("Echo");
	model.Add("Fable");
	model.Add("Onyx");
	model.Add("Nova");
	model.Add("Sage");
	model.Add("Shimmer");
	model.SetIndex(0);
	quality.Add("tts-1");
	quality.Add("tts-1-hf");
	quality.SetIndex(0);
	speed.Add("0.25x");
	speed.Add("0.5x");
	speed.Add("1x");
	speed.Add("2x");
	speed.Add("4x");
	speed.SetIndex(2);
	fileformat.Add("mp3");
	fileformat.Add("opus");
	fileformat.Add("aac");
	fileformat.Add("flac");
	fileformat.Add("wav");
	fileformat.Add("pcm");
	fileformat.SetIndex(0);
	generations.AddColumn("Time");
	generations.AddColumn("Mode");
	generations.AddColumn("Text");
	generations.AddIndex("IDX");
	generations.ColumnWidths("1 1 2");
	
}

AssistantCtrl::AssistantCtrl() {
	CtrlLayout(*this);
	
}

RealtimeAiCtrl::RealtimeAiCtrl() {
	CtrlLayout(*this);
	
}

ChatAiCtrl::ChatAiCtrl() {
	CtrlLayout(*this);
	frequency_penalty.SetData(1);
	presence_penalty.SetData(1);
}

CustomBiasesCtrl::CustomBiasesCtrl() {
	CtrlLayout(*this);
	
}

PlaygroundCtrl::PlaygroundCtrl() {
	Add(tabs.SizePos());
	
	tabs.Add(completion.SizePos(), "Completion");
	tabs.Add(tts.SizePos(), "TTS");
	tabs.Add(ass.SizePos(), "Assistant");
	tabs.Add(rt.SizePos(), "Realtime");
	tabs.Add(chat.SizePos(), "Chat");
	tabs.Add(bias.SizePos(), "Custom Biases");
	tabs.Add(edit_img.SizePos(), "Image Generation");
	tabs.Add(img_aspect.SizePos(), "Image Aspect Fixer");
	tabs.Add(tasks.SizePos(), "Tasks");
	
	tabs.WhenSet = THISBACK(Data);
	
	/*
	TODO
	
	void Data();
	void ToolMenu(Bar& bar);
	*/
}

void PlaygroundCtrl::CreateThread() {
	omni.Create();
	SetThread(*omni);
}

void PlaygroundCtrl::SetThread(OmniThread& t) {
	completion.SetThread(t);
	tts.SetThread(t);
	ass.SetThread(t);
	rt.SetThread(t);
	chat.SetThread(t);
	bias.SetThread(t);
}

void PlaygroundCtrl::Data() {
	int tab = tabs.Get();
	
	switch (tab) {
		case 8: tasks.Data(); break;
		default: break;
	}
}



PlaygroundApp::PlaygroundApp() {
	Title("Playground").MaximizeBox().MinimizeBox();
	Maximize();
	
	Add(pg.SizePos());
	AddFrame(menu);
	
	pg.CreateThread();
}

void RunAiPlayground() {
	PlaygroundApp().Run();
}

END_UPP_NAMESPACE
