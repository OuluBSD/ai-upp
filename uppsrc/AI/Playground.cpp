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
	
	model_name.WhenAction = [this]{this->model_i = this->model_name.GetIndex();};
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

void AiThreadCtrlBase::AddModel(String name, bool use_chat) {
	auto& m = models.Add();
	m.name = name;
	m.use_chat = use_chat;
}

void AiThreadCtrlBase::MainMenu(Bar& bar) {
	bar.Add("Update", [this]{this->Data();}).Key(K_F5);
	bar.Add("Submit", [this]{this->Submit();}).Key(K_CTRL_ENTER);
}

void AiThreadCtrlBase::Serialize(Stream& s) {
	int ver = 0;
	s % ver;
	
	if (ver >= 0)
		s % model_i;
}

int AiThreadCtrlBase::GetModelCount(bool use_chat) {
	int c = 0;
	for (auto it : models)
		if (it.use_chat == use_chat)
			c++;
	return c;
}

void AiThreadCtrlBase::UpdateCompletionModels() {
	UpdateModels(true);
}

void AiThreadCtrlBase::UpdateChatModels() {
	UpdateModels(false);
}

void AiThreadCtrlBase::UpdateModels(bool completion) {
	TaskMgr& m = AiTaskManager();
	ModelArgs args;
	Ptr<Ctrl> p = GetCtrl();
	m.GetModels(args, [this, p, completion](String res){
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
			if (CannotDoCompletion(m) == !completion)
				AddModel(m, false);
		
		PostCallback([this]{this->Data();});
	});
}

bool AiThreadCtrlBase::CannotDoCompletion(String model_name) {
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
	else if (model_name.GetCount() != models.GetCount()) {
		model_name.Clear();
		for (auto it : models)
			model_name.Add(it.name);
		if (model_i >= 0 && model_i < model_name.GetCount())
			model_name.SetIndex(model_i);
		else if (model_name.GetCount())
			model_name.SetIndex(0);
	}
}

void AiThreadCtrlBase::SetThread(AiThread& t) {
	ai_thrd = &t;
}



void CompletionCtrl::Submit() {
	if (!HasThread()) return;
	if (this->model_name.GetCount() == 0) return;
	CompletionThread& t = GetCompletionThread();
	TaskMgr& m = AiTaskManager();
	
	String txt = this->prompt.GetData();
	txt.Replace("\r","");
	
	
	int model_i = this->model_name.GetIndex();
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
			this->prompt.SetCursor(this->prompt.GetLength());
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

void TextToSpeechCtrl::Data() {
	
}

AssistantCtrl::AssistantCtrl() {
	CtrlLayout(*this);
	
}

void AssistantCtrl::Data() {
	
}

RealtimeAiCtrl::RealtimeAiCtrl() {
	CtrlLayout(*this);
	
}

void RealtimeAiCtrl::Data() {
	
}

ChatAiCtrl::ChatAiCtrl() {
	CtrlLayout(*this);
	chat_ctrl.Add(chat.SizePos());
	frequency_penalty.SetData(1);
	presence_penalty.SetData(1);
	top_p.SetData(1);
	temperature.SetData(1);
	model_name.WhenAction = [this]{this->model_i = this->model_name.GetIndex();};
	this->system_instructions.SetData("You are a helpful assistant...");
	
	sessions.AddColumn("Name");
	sessions.AddColumn("Changed");
	sessions.AddIndex("IDX");
	
	submit <<= THISBACK(Submit);
}

void ChatAiCtrl::Data() {
	if (!GetModelCount(false)) {
		UpdateChatModels();
	}
	else if (model_name.GetCount() != models.GetCount()) {
		model_name.Clear();
		for (auto it : models)
			model_name.Add(it.name);
		if (model_i >= 0 && model_i < model_name.GetCount())
			model_name.SetIndex(model_i);
		else if (model_name.GetCount())
			model_name.SetIndex(0);
	}
	
	// Update sessions
	if (!HasThread()) return;
	ChatThread& t = GetChatThread();
	if (t.sessions.IsEmpty())
		AddSession();
	if (sessions.GetCount() != t.sessions.GetCount()) {
		for(int i = 0; i < t.sessions.GetCount(); i++) {
			auto& session = t.sessions[i];
			sessions.Set(i, "IDX", i);
			sessions.Set(i, 0, session.name);
			sessions.Set(i, 1, session.changed);
		}
		sessions.SetCount(t.sessions.GetCount());
		sessions.SetSortColumn(1, true);
		if (!sessions.IsCursor())
			sessions.SetCursor(0);
	}
	
	DataSession();
}

void ChatAiCtrl::ClearSessionCtrl() {
	
}

void ChatAiCtrl::AddSession() {
	ChatThread& t = GetChatThread();
	auto& session = t.sessions.Add();
	session.created = GetSysTime();
	session.name = "Unnamed";
}

void ChatAiCtrl::DataSession() {
	if (!sessions.IsCursor()) {
		ClearSessionCtrl();
		return;
	}
	
	session_i = sessions.Get("IDX");
	ChatThread& t = GetChatThread();
	const auto& session = t.sessions[session_i];
	
	
}

void ChatAiCtrl::Submit() {
	if (!HasThread()) return;
	if (this->model_name.GetCount() == 0) return;
	if (session_i < 0) return;
	
	ChatThread& t = GetChatThread();
	TaskMgr& m = AiTaskManager();
	
	String txt = this->prompt.GetData();
	txt.Replace("\r","");
	
	{
		if (session_i < 0 || session_i >= t.sessions.GetCount())
			return;
		auto& session = t.sessions[session_i];
		auto& item = session.items.Add();
		item.type = MSG_USER;
		item.text = txt;
		item.username = "";
		session.changed = item.created = GetSysTime();
		PostCallback([this]{
			this->prompt.Clear();
			DataSession();
		});
	}
	
	
	ChatArgs args;
	{
		bool found = false;
		String c = this->system_instructions.GetData();
		for (int i = args.messages.GetCount()-1; i >= 0; i--) {
			auto& msg = args.messages[i];
			if (msg.type == MSG_SYSTEM) {
				found = msg.content == c;
				break;
			}
		}
		if (!found) {
			auto& msg = args.messages.Add();
			msg.type = MSG_SYSTEM;
			msg.content = c;
		}
	}
	{
		auto& msg = args.messages.Add();
		msg.type = MSG_USER;
		msg.content = txt;
		chat.AddMessage("User", txt);
	}
	args.model_name = models[this->model_name.GetIndex()].name;
	//args.response_format = this->response_format.GetData();
	args.temperature = this->temperature.GetData();
	args.stop_seq = this->stop_seq.GetData();
	args.top_prob = this->top_p.GetData();
	args.frequency_penalty = this->frequency_penalty.GetData();
	args.presence_penalty = this->presence_penalty.GetData();
	
	m.GetChat(args, [this, txt](String res) {
		PostCallback([this,res]{
			this->prompt.Clear();
			chat.AddMessage("System", res);
		});
	});
}

CustomBiasesCtrl::CustomBiasesCtrl() {
	CtrlLayout(*this);
	
}

void CustomBiasesCtrl::Data() {
	
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
	
	tabs.WhenSet = [this]{
		this->Data();
		WhenTab();
	};
	
	PostCallback(THISBACK(LoadThis));
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

void PlaygroundCtrl::TabMenu(Bar& b) {
	int tab = tabs.Get();
	
	switch (tab) {
		case 0: completion.MainMenu(b); break;
		case 1: chat.MainMenu(b); break;
		default: break;
	}
}

void PlaygroundCtrl::Data() {
	int tab = tabs.Get();
	
	switch (tab) {
		case 0: completion.Data(); break;
		case 1: chat.Data(); break;
		case 8: tasks.Data(); break;
		default: break;
	}
}

void PlaygroundCtrl::Serialize(Stream& s) {
	int ver = 0;
	s % ver;
	
	int tab = 0;
	
	if (ver >= 0) {
		tab = tabs.Get();
		s % tab;
		
		s % (AiThreadCtrlBase&)completion;
		s % (AiThreadCtrlBase&)chat;
		//s % tts;
		//s % ass;
		//s % rt;
		//s % bias;
		//s % edit_img;
		//s % img_aspect;
		//s % tasks;
	}
	
	if (s.IsLoading())
		tabs.Set(tab);
}

void PlaygroundCtrl::StoreThis() {
	if (omni)
		VisitToJsonFile(*omni, ConfigFile("playground.json"));
	StoreToFile(*this, ConfigFile("playground-gui.bin"));
}

void PlaygroundCtrl::LoadThis() {
	if (omni)
		VisitFromJsonFile(*omni, ConfigFile("playground.json"));
	LoadFromFile(*this, ConfigFile("playground-gui.bin"));
}



PlaygroundApp::PlaygroundApp() {
	Title("Playground").MaximizeBox().MinimizeBox();
	Maximize();
	
	Add(pg.SizePos());
	AddFrame(menu);
	PostCallback(THISBACK(UpdateMenu));
	
	pg.WhenTab << THISBACK(UpdateMenu);
	pg.CreateThread();
}

void PlaygroundApp::UpdateMenu() {
	menu.Set(THISBACK(MainMenu));
}

void PlaygroundApp::MainMenu(Bar& bar) {
	bar.Sub("App", [this](Bar& bar) {
		bar.Add("Quit", [this]{this->Close();});
	});
	bar.Sub("Tab", [this](Bar& bar) {pg.TabMenu(bar);});
}

void RunAiPlayground() {
	PlaygroundApp().Run();
}

END_UPP_NAMESPACE
