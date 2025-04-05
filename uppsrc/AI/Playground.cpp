#include "AI.h"

NAMESPACE_UPP


CompletionCtrl::CompletionCtrl() {
	CtrlLayout(*this);
	
	/*
	model.Add("gpt-3.5-turbo-instruct-0914");
	model.Add("gpt-3.5-turbo-instruct");
	model.Add("babbage-002");
	model.Add("davinci-002");
	AddModel("gpt-4o", true);
	*/
	
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
	prompt.LineNumbers(true);
	prompt.WithCutLine(true);
}

void CompletionCtrl::AddModel(String name, bool use_chat) {
	auto& m = models.Add();
	m.name = name;
	m.use_chat = use_chat;
}

int CompletionCtrl::GetModelCount(bool use_chat) {
	int c = 0;
	for (auto it : models)
		if (it.use_chat == use_chat)
			c++;
	return c;
}

void CompletionCtrl::UpdateCompletionModels() {
	TaskMgr& m = AiTaskManager();
	ModelArgs args;
	Ptr<Ctrl> p = this;
	m.GetModels(args, [this, p](String res){
		if (!p) return;
		Vector<String> models;
		LoadFromJson(models, res);
		
		Vector<int> rm;
		for(int i = 0; i < this->models.GetCount(); i++)
			if (!this->models[i].use_chat)
				rm << i;
		if (!rm.IsEmpty())
			this->models.Remove(rm);
		
		for (String m : models)
			if (!CannotDoCompletion(m))
				AddModel(m, false);
		
		PostCallback(THISBACK(Data));
	});
	
	
}

bool CompletionCtrl::CannotDoCompletion(String model_name) {
	thread_local static Vector<String> non_completions;
	if (non_completions.IsEmpty()) {
		non_completions	<< "gpt-4"
						<< "dall-e"
						<< "o1"
						<< "tts"
						<< "text-embedding"
						<< "omni-"
						<< "o3"
						<< "computer-"
						<< "chatgpt-"
						<< "whisper-"
						<< "gpt-3.5-turbo-1106"
						<< "gpt-3.5-turbo-0125"
						<< "gpt-3.5-turbo-16k"
						;
	}
	for (const String& n : non_completions)
		if (model_name.GetCount() >= n.GetCount() &&
			model_name.Find(n) == 0)
			return true;
	if (model_name == "gpt-3.5-turbo")
		return true;
	
	return false;
}

void CompletionCtrl::Data() {
	if (!GetModelCount(false)) {
		UpdateCompletionModels();
	}
	else if (model.GetCount() != models.GetCount()) {
		model.Clear();
		for (auto it : models)
			model.Add(it.name);
		if (model.GetCount())
			model.SetIndex(0);
	}
}

void AiThreadCtrlBase::SetThread(AiThread& t) {
	ai_thrd = &t;
}



void CompletionCtrl::Submit() {
	if (!HasThread()) return;
	if (model.GetCount() == 0) return;
	CompletionThread& t = GetCompletionThread();
	TaskMgr& m = AiTaskManager();
	
	String txt = this->prompt.GetData();
	txt.Replace("\r","");
	
	
	int model_i = model.GetIndex();
	String model_name = models[model_i].name;
	bool model_uses_chat_mode = models[model_i].use_chat;
	
	if (model_uses_chat_mode) {
		ChatArgs args;
		TODO
	}
	else {
		CompletionArgs args;
		args.prompt = txt;
		args.model_name = model_name;
		args.temperature = this->temperature.GetData();
		args.max_length = this->max_length.GetData();
		args.stop_seq = this->stop_seq.GetData();
		args.top_prob = this->top_p.GetData();
		args.frequency_penalty = this->freq_penalty.GetData();
		args.presence_penalty = this->presence_penalty.GetData();
		args.best_of = this->best_of.GetData();
		args.inject_start_text = this->inject_start.GetData();
		args.inject_restart_text = this->inject_restart.GetData();
		args.show_probabilities = (CompletionArgs::ShowProbs)this->show_probs.GetIndex();
		
		if (args.stop_seq.IsEmpty())
			args.stop_seq = "<|endoftext|>";
		
		m.GetCompletion(args, [this, txt](String res) {
			String new_data = txt + res;
			GuiLock __;
			this->prompt.SetData(new_data);
		});
	}
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
	tabs.Add(chat.SizePos(), "Chat");
	tabs.Add(placeholder.SizePos(), "JSON chat");
	tabs.Add(edit_img.SizePos(), "Image");
	tabs.Add(img_aspect.SizePos(), "Image Aspect Fixer");
	tabs.Add(placeholder.SizePos(), "Transcribe");
	tabs.Add(tts.SizePos(), "TTS");
	tabs.Add(rt.SizePos(), "Realtime");
	tabs.Add(ass.SizePos(), "Assistant");
	tabs.Add(bias.SizePos(), "Custom Biases");
	tabs.Add(tasks.SizePos(), "Tasks");
	
	tabs.WhenSet = THISBACK(Data);
	
	PostCallback(THISBACK(Data));
	/*
	TODO
	
	void Data();
	void ToolMenu(Bar& bar);
	*/
}

PlaygroundCtrl::~PlaygroundCtrl() {
	StoreThis();
}

void PlaygroundCtrl::CreateThread() {
	omni.Create();
	LoadThis();
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
		case 0: completion.Data(); break;
		case 8: tasks.Data(); break;
		default: break;
	}
}

void PlaygroundCtrl::StoreThis() {
	if (!omni) return;
	VisitToJsonFile(*omni, ConfigFile("playground.json"));
}

void PlaygroundCtrl::LoadThis() {
	if (!omni) return;
	VisitFromJsonFile(*omni, ConfigFile("playground.json"));
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
