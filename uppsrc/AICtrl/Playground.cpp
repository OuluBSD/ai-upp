#include "AICtrl.h"

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

void AiThreadCtrlBase::Visit(Vis& s) {
	s.Ver(1)
	(1)	("model_i", model_i)
		;
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
		
		Ptr<AiThreadCtrlBase> c = this;
		PostCallback([c]{if (c) c->Data();});
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

void AiThreadCtrlBase::SetNode(MetaNode& n) {
	node = &n;
}

StageThread& AiThreadExt::GetStageThread() {
	auto& o = GetNode();
	return o.GetExt<StageThread>();
}

ChainThread& AiThreadExt::GetChainThread() {
	auto& o = GetNode();
	return o.GetExt<ChainThread>();
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






INITIALIZER_COMPONENT_CTRL(StageThread, AiStageCtrl)

AiStageCtrl::AiStageCtrl() {
	Add(vsplit.SizePos());
	
	vsplit.Vert() << hsplit << btabs;
	vsplit.SetPos(8000);
	
	btabs.Add(log.SizePos(), "Log");
	
	hsplit.Horz() << proglist << prog << stagelist << stage;
	hsplit.SetPos(1000,0).SetPos(5000,1).SetPos(6000,2);
	
	proglist.AddColumn("Program");
	proglist.AddIndex("IDX");
	proglist.WhenBar = THISBACK(ProgramMenu);
	proglist.WhenCursor = THISBACK(DataProgram);
	
	stagelist.AddColumn("Stage");
	stagelist.AddIndex("IDX");
	stagelist.WhenBar = THISBACK(StageMenu);
	stagelist.WhenCursor = THISBACK(DataStage);
	
	prog.Highlight("cpp");
	prog.LineNumbers(true);
	prog.WhenAction = [this] {
		if (!proglist.IsCursor()) return;
		int idx = proglist.Get("IDX");
		MetaNode& n = *programs[idx];
		n.value = prog.GetData();
	};
	stage.Highlight("cpp");
	stage.LineNumbers(true);
	stage.WhenAction = [this] {
		if (!stagelist.IsCursor()) return;
		int idx = stagelist.Get("IDX");
		MetaNode& n = *stages[idx];
		n.value = stage.GetData();
	};
}

void AiStageCtrl::Data() {
	if (!ext) return;
	DataProgramList();
	DataStageList();
	DataBottom();
}

void AiStageCtrl::DataList(ArrayCtrl& list, Vector<MetaNode*>& nodes, hash_t type_hash) {
	MetaNode& n = GetNode();
	nodes = n.FindTypeAllShallow(type_hash);
	for(int i = 0; i < nodes.GetCount(); i++) {
		const auto& it = *nodes[i];
		list.Set(i, 0, it.id);
		const AstValue* ast = FindRawValue<AstValue>(it.value);
		list.Set(i, 1, ast ? ast->type : "");
		list.Set(i,"IDX", i);
	}
	list.SetCount(nodes.GetCount());
	if (!list.IsCursor() && list.GetCount())
		list.SetCursor(0);
	else
		DataProgram();
}

void AiStageCtrl::DataProgramList() {
	DataList(proglist, programs, AsTypeHash<VfsProgram>());
}

void AiStageCtrl::DataProgram() {
	if (!proglist.IsCursor()) return;
	int idx = proglist.Get("IDX");
	MetaNode& n = *programs[idx];
	prog.SetData(n.value);
}

void AiStageCtrl::DataStageList() {
	DataList(stagelist, stages, AsTypeHash<VfsFarStage>());
}

void AiStageCtrl::DataStage() {
	if (!stagelist.IsCursor()) return;
	int idx = stagelist.Get("IDX");
	MetaNode& n = *stages[idx];
	stage.SetData(n.value);
}

void AiStageCtrl::DataBottom() {
	
}

bool AiStageCtrl::CompileStages() {
	bool succ = false;
	
	log.Clear();
	
	Agent* agent = ext->node.FindOwnerWith<Agent>();
	ASSERT(agent);
	
	for(int i = 0; i < stages.GetCount(); i++) {
		MetaNode& n = *stages[i];
		
		String esc = n.value;
		if (esc.IsEmpty()) continue;
		
		succ = agent->CompileStage(n, THISBACK(PrintLog));
	}
	return true;
}

bool AiStageCtrl::Compile() {
	if (!proglist.IsCursor()) return false;
	int idx = proglist.Get("IDX");
	MetaNode& n = *programs[idx];
	bool succ = false;
	
	String esc = n.value;
	if (esc.IsEmpty()) return false;
	
	log.Clear();
	
	Agent* agent = ext->node.FindOwnerWith<Agent>();
	ASSERT(agent);
	succ = agent->Compile(esc, THISBACK(PrintLog));
	
	return true;
}

bool AiStageCtrl::Run() {
	if (agent) {
		agent->Stop();
	}
	if (!Compile())
		return false;
	
	log.Clear();
	
	agent = ext->node.FindOwnerWith<Agent>();
	ASSERT(agent);
	
	agent->WhenPrint = THISBACK(Print);
	agent->WhenInput = THISBACK(Input);
	
	agent->Start(THISBACK(PrintLog), [this](bool succ) {
		if (!succ) {
			GuiLock __;
			log.Append("Running failed\n");
			log.SetCursor(log.GetLength());
		}
	});
	
	return true;
}

void AiStageCtrl::Print(EscEscape& e) {
	String s = e[0];
	
	GuiLock __;
	log.Append(s + "\n");
	log.SetCursor(log.GetLength());
}

void AiStageCtrl::Input(EscEscape& e) {
	String s;
	EditText(s, "Input", "Text");
	e = s;
}

void AiStageCtrl::PrintLog(Vector<ProcMsg>& msgs) {
	String s;
	for (ProcMsg& m : msgs) {
		s << m.ToString() << "\n";
	}
	GuiLock __;
	log.Append(s);
	log.SetCursor(log.GetLength());
}

void AiStageCtrl::ToolMenu(Bar& b) {
	b.Add("Compile Stages", [this]{CompileStages();}).Key(K_F8);
	b.Add("Compile Program", [this]{Compile();}).Key(K_F7);
	b.Add("Run", [this]{Run();}).Key(K_F5);
}

void AiStageCtrl::ProgramMenu(Bar& b) {
	b.Add("Add program", THISBACK(AddProgram));
	b.Add("Remove program", THISBACK(RemoveProgram));
	b.Add("Rename program", THISBACK(RenameProgram));
	b.Add("Duplicate program", THISBACK(DuplicateProgram));
}

void AiStageCtrl::AddProgram() {
	String name;
	if (!EditText(name, "Program's name", "Name"))
		return;
	
	for(int i = 0; i < programs.GetCount(); i++) {
		if (programs[i]->id == name) {
			PromptOK("Program with that name exists already");
			return;
		}
	}
	auto& ses = GetNode().Add<VfsProgram>(name);
	PostCallback(THISBACK(DataProgramList));
}

void AiStageCtrl::RemoveProgram() {
	if (!proglist.IsCursor())
		return;
	int id = proglist.Get("IDX");
	MetaNode* n = programs[id];
	GetNode().Remove(n);
	PostCallback(THISBACK(DataProgramList));
}

void AiStageCtrl::RenameProgram() {
	if (!proglist.IsCursor())
		return;
	int id = proglist.Get("IDX");
	auto& n = *programs[id];
	String name = n.id;
	if (!EditText(name, "Program's name", "Name"))
		return;
	n.id = name;
	PostCallback(THISBACK(DataProgramList));
}

void AiStageCtrl::DuplicateProgram() {
	if (!proglist.IsCursor())
		return;
	int id = proglist.Get("IDX");
	const auto& n0 = *programs[id];
	auto& m1 = GetNode().Add<VfsProgram>("Duplicate of " + n0.id);
	VisitCopy(n0, m1.node);
	PostCallback(THISBACK(DataProgramList));
}

void AiStageCtrl::StageMenu(Bar& b) {
	b.Add("Add stage", THISBACK(AddStage));
	b.Add("Remove stage", THISBACK(RemoveStage));
	b.Add("Rename stage", THISBACK(RenameStage));
	b.Add("Duplicate stage", THISBACK(DuplicateStage));
}

void AiStageCtrl::AddStage() {
	String name;
	if (!EditText(name, "Stage's name", "Name"))
		return;
	
	for(int i = 0; i < stages.GetCount(); i++) {
		if (stages[i]->id == name) {
			PromptOK("Stage with that name exists already");
			return;
		}
	}
	auto& ses = GetNode().Add<VfsFarStage>(name);
	PostCallback(THISBACK(DataStageList));
}

void AiStageCtrl::RemoveStage() {
	if (!stagelist.IsCursor())
		return;
	int id = stagelist.Get("IDX");
	MetaNode* n = stages[id];
	GetNode().Remove(n);
	PostCallback(THISBACK(DataStageList));
}

void AiStageCtrl::RenameStage() {
	if (!stagelist.IsCursor())
		return;
	int id = stagelist.Get("IDX");
	auto& n = *stages[id];
	String name = n.id;
	if (!EditText(name, "Stage's name", "Name"))
		return;
	n.id = name;
	PostCallback(THISBACK(DataStageList));
}

void AiStageCtrl::DuplicateStage() {
	if (!stagelist.IsCursor())
		return;
	int id = stagelist.Get("IDX");
	const auto& n0 = *stages[id];
	auto& m1 = GetNode().Add<VfsFarStage>("");
	VisitCopy(n0, m1.node);
	PostCallback(THISBACK(DataStageList));
}

MetaNode* AiStageCtrl::GetProgram() {
	if (!proglist.IsCursor())
		return 0;
	int ses_i = proglist.Get("IDX");
	return programs[ses_i];
}

MetaNode* AiStageCtrl::GetStage() {
	if (!stagelist.IsCursor())
		return 0;
	int ex_i = stagelist.Get("IDX");
	return stages[ex_i];
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
	sessions.WhenBar = THISBACK(SessionMenu);
	sessions.WhenCursor = THISBACK(DataSession);
	
	submit <<= THISBACK(Submit);
	clear <<= THISBACK(ClearSession);
	
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
	chat.Clear();
	
}

void ChatAiCtrl::AddSession() {
	ChatThread& t = GetChatThread();
	auto& session = t.sessions.Add();
	session.created = GetSysTime();
	session.name = "Unnamed";
}

void ChatAiCtrl::RemoveSession() {
	if (!sessions.IsCursor())
		return;
	int idx = sessions.Get("IDX");
	ChatThread& t = GetChatThread();
	t.sessions.Remove(idx);
	PostCallback(THISBACK(Data));
}

void ChatAiCtrl::ClearSession() {
	if (!sessions.IsCursor())
		return;
	int idx = sessions.Get("IDX");
	ChatThread& t = GetChatThread();
	auto& session = t.sessions[idx];
	session.items.Clear();
	session.changed = GetSysTime();
	PostCallback(THISBACK(DataSession));
}

void ChatAiCtrl::MainMenu(Bar& bar) {
	AiThreadCtrlBase::MainMenu(bar);
	bar.Separator();
	SessionMenu(bar);
}

void ChatAiCtrl::SessionMenu(Bar& bar) {
	if (sessions.IsCursor())
		bar.Add("Remove session", THISBACK(RemoveSession));
	bar.Add("Add session", THISBACK(AddSession));
}

void ChatAiCtrl::DataSession() {
	if (!sessions.IsCursor()) {
		ClearSessionCtrl();
		return;
	}
	
	int prev_session_i = session_i;
	session_i = sessions.Get("IDX");
	ChatThread& t = GetChatThread();
	const auto& session = t.sessions[session_i];
	
	if (session_i != prev_session_i ||
		chat.GetMessageCount() > session.items.GetCount())
		chat.Clear();
	
	int begin = chat.GetMessageCount();
	for(int i = begin; i < session.items.GetCount(); i++) {
		const auto& item = session.items[i];
		chat.AddMessage(
			GetMessageTypeString(item.type),
			item.content);
	}
	if (begin < session.items.GetCount())
		chat.Set(begin);
}

void ChatAiCtrl::Submit() {
	if (!HasThread()) return;
	if (this->model_name.GetCount() == 0) return;
	if (session_i < 0) return;
	
	ChatThread& t = GetChatThread();
	TaskMgr& m = AiTaskManager();
	
	String txt = this->prompt.GetData();
	txt.Replace("\r","");
	
	if (session_i < 0 || session_i >= t.sessions.GetCount())
		return;
	auto& session = t.sessions[session_i];
	{
		bool found = false;
		String c = this->system_instructions.GetData();
		for (int i = session.items.GetCount()-1; i >= 0; i--) {
			const auto& item = session.items[i];
			if (item.type == MSG_SYSTEM) {
				String a = item.content;
				found = a == c;
				break;
			}
		}
		if (!found) {
			auto& item = session.items.Add();
			item.type = MSG_SYSTEM;
			item.content = c;
			session.changed = item.created = GetSysTime();
		}
	}
	{
		bool found = false;
		if (session.items.GetCount()) {
			auto& item = session.items.Top();
			if (item.type == MSG_USER && item.content == txt)
				found = true;
		}
		if (!found) {
			auto& item = session.items.Add();
			item.type = MSG_USER;
			item.content = txt;
			item.username = "";
			session.changed = item.created = GetSysTime();
		}
		PostCallback([this]{
			this->prompt.Clear();
			DataSession();
		});
	}
	
	ChatArgs args;
	for (const auto& it : session.items) {
		auto& msg = args.messages.Add();
		msg.type = it.type;
		msg.content = it.content;
		msg.name = it.username;
	}
	
	args.model_name = models[this->model_name.GetIndex()].name;
	//args.response_format = this->response_format.GetData();
	args.temperature = this->temperature.GetData();
	args.stop_seq = this->stop_seq.GetData();
	args.top_prob = this->top_p.GetData();
	args.frequency_penalty = this->frequency_penalty.GetData();
	args.presence_penalty = this->presence_penalty.GetData();
	
	m.GetChat(args, [this](String res) {
		PostCallback([this,res]{
			ChatThread& t = GetChatThread();
			if (session_i < 0 || session_i >= t.sessions.GetCount())
				return;
			auto& session = t.sessions[session_i];
			auto& item = session.items.Add();
			item.type = MSG_ASSISTANT;
			item.content = res;
			item.created = GetSysTime();
			DataSession();
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
	tabs.Add(stage.SizePos(), "Function");
	tabs.Add(placeholder.SizePos(), "Action Planner");
	#if 0
	tabs.Add(placeholder.SizePos(), "User/System"); // https://sketch.dev/blog/agent-loop (https://news.ycombinator.com/item?id=43998472)
	tabs.Add(placeholder.SizePos(), "Action Planner");
	tabs.Add(placeholder.SizePos(), "Group of 3");
	tabs.Add(placeholder.SizePos(), "Tournament");
	tabs.Add(placeholder.SizePos(), "DM");
	tabs.Add(placeholder.SizePos(), "Team DM");
	tabs.Add(placeholder.SizePos(), "Game of toxic adoration");
	tabs.Add(placeholder.SizePos(), "Courtroom");
	tabs.Add(placeholder.SizePos(), "Multi-team cooperative parallel");
	tabs.Add(placeholder.SizePos(), "Multi-team cooperative hierarchical");
	tabs.Add(placeholder.SizePos(), "Animation");
	tabs.Add(placeholder.SizePos(), "Adventure");
	#endif
	tabs.Add(chain.SizePos(), "Filesystem-chat");
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
	stage.ext = 0;
	stage.type_hash = 0;
	chain.ext = 0;
	chain.type_hash = 0;
	StoreThis();
	omni.Clear();
}

void PlaygroundCtrl::CreateThread() {
	omni.Create();
	LoadThis();
	SetThread(*omni);
}

void PlaygroundCtrl::SetThread(OmniThread& t) {
	completion.SetThread(t);
	chat.SetThread(t);
	tts.SetThread(t);
	ass.SetThread(t);
	rt.SetThread(t);
	bias.SetThread(t);
}

void PlaygroundCtrl::TabMenu(Bar& b) {
	int tab = tabs.Get();
	
	switch (tab) {
		case 0: completion.MainMenu(b); break;
		case 1: chat.MainMenu(b); break;
		case 2: stage.ToolMenu(b); break;
		case 3: chain.ToolMenu(b); break;
		default: break;
	}
}

void PlaygroundCtrl::Data() {
	int tab = tabs.Get();
	
	switch (tab) {
		case 0: completion.Data(); break;
		case 1: chat.Data(); break;
		case 2: stage.Data(); break;
		case 3: chain.Data(); break;
		case 8: tasks.Data(); break;
		default: break;
	}
}

void PlaygroundCtrl::Visit(Vis& s) {
	int tab = tabs.Get();
	
	s.Ver(1)
	(1)	("tab",tab)
		("completion", (AiThreadCtrlBase&)completion, VISIT_NODE)
		("chat", (AiThreadCtrlBase&)chat, VISIT_NODE)
		("stage", (AiThreadCtrlBase&)stage, VISIT_NODE)
		;
	//s % tts;
	//s % ass;
	//s % rt;
	//s % bias;
	//s % edit_img;
	//s % img_aspect;
	//s % tasks;
	
	if (s.IsLoading()) {
		PostCallback([this,tab]{
			tabs.Set(tab);
		});
	}
}

void PlaygroundCtrl::StoreThis() {
	if (omni)
		VisitToJsonFile(*omni, ConfigFile("playground.json"));
	VisitToJsonFile(*this, ConfigFile("playground-gui.json"));
	if (node)
		VisitToJsonFile(*node, ConfigFile("playground-node.json"));
}

void PlaygroundCtrl::LoadThis() {
	if (omni)
		VisitFromJsonFile(*omni, ConfigFile("playground.json"));
	VisitFromJsonFile(*this, ConfigFile("playground-gui.json"));
	if (node) {
		VisitFromJsonFile(*node, ConfigFile("playground-node.json"));
		
		auto& stage_n = node->GetAdd<VfsFarStage>("stage-session");
		stage.ext = &stage_n;
		stage.type_hash = stage_n.GetTypeHash();
		
		auto& chain_n = node->GetAdd<ChainThread>("chain");
		chain.ext = &chain_n;
		chain.type_hash = chain_n.GetTypeHash();
		
		auto& agent = node->GetAdd<Agent>("agent");
	}
}

void PlaygroundCtrl::SetNode(MetaNode& n) {
	this->node = &n;
}



PlaygroundApp::PlaygroundApp() {
	Title("Playground").MaximizeBox().MinimizeBox();
	Sizeable().Maximize();
	
	Add(pg.SizePos());
	AddFrame(menu);
	PostCallback(THISBACK(UpdateMenu));
	
	omni_node.Create();
	VisitFromJsonFile(*omni_node, ConfigFile("playground-root.json"));
	pg.SetNode(*omni_node);
	
	pg.WhenTab << THISBACK(UpdateMenu);
	pg.CreateThread();
}

PlaygroundApp::~PlaygroundApp() {
	pg.StoreThis();
	VisitToJsonFile(*omni_node, ConfigFile("playground-root.json"));
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
