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
	Add(hsplit.SizePos());
	
	hsplit.Horz() << msplit << structure << rsplit;
	hsplit.SetPos(1500,0).SetPos(5750,1);
	
	msplit.Vert() << session << examples;
	
	session.AddColumn("Session");
	session.AddColumn("Version");
	session.AddIndex("IDX");
	session.WhenBar = THISBACK(SessionMenu);
	session.WhenCursor = THISBACK(DataSession);
	
	examples.AddColumn("Example");
	examples.AddIndex("IDX");
	examples.WhenBar = THISBACK(ExampleMenu);
	examples.WhenCursor = THISBACK(DataExample);
	
	structure.WhenBar = THISBACK(StageMenu);
	structure.WhenCursor = THISBACK(DataItem);
	
}

void AiStageCtrl::Data() {
	auto& list = this->session;
	StageThread& t = GetStageThread();
	MetaNode& n = GetNode();
	sessions = n.FindAllShallow(METAKIND_ECS_COMPONENT_AI_STAGE_SESSION);
	for(int i = 0; i < sessions.GetCount(); i++) {
		const auto& it = *sessions[i];
		list.Set(i, 0, it.id);
		list.Set(i, 1, it.type);
		list.Set(i,"IDX", i);
	}
	list.SetCount(sessions.GetCount());
	if (!list.IsCursor() && list.GetCount())
		list.SetCursor(0);
	else
		DataSession();
}

MetaNode* AiStageCtrl::GetSession() {
	if (!session.IsCursor())
		return 0;
	int ses_i = session.Get("IDX");
	return sessions[ses_i];
}

MetaNode* AiStageCtrl::GetExample() {
	if (!examples.IsCursor())
		return 0;
	int ex_i = examples.Get("IDX");
	return example_nodes[ex_i];
}

void AiStageCtrl::SetExampleValue(Value key, Value val) {
	MetaNode* ex = GetExample();
	ASSERT(ex);
	if (!ex->ext)
		ex->ext = new AiStageExample(*ex);
	ASSERT(ex->ext);
	ValueComponentBase& comp = ex->GetExt<ValueComponentBase>();
	if (!comp.value.Is<ValueMap>())
		comp.value = ValueMap();
	ValueMap map = comp.value;
	map.Set(key, val);
	comp.value = map;
}

Value AiStageCtrl::GetExampleValue(Value key) {
	MetaNode* ex = GetExample();
	if (!ex)
		return Value();
	ASSERT(ex);
	if (!ex->ext)
		ex->ext = new AiStageExample(*ex);
	ASSERT(ex->ext);
	ValueComponentBase& comp = ex->GetExt<ValueComponentBase>();
	if (!comp.value.Is<ValueMap>())
		comp.value = ValueMap();
	ValueMap map = comp.value;
	int i = map.Find(key);
	if (i < 0)
		return Value();
	return map.GetValue(i);
}

void AiStageCtrl::DataSession() {
	if (!session.IsCursor()) {
		examples.Clear();
		return;
	}
	StageThread& t = GetStageThread();
	int ses_i = session.Get("IDX");
	MetaNode& ses = *sessions[ses_i];
	
	example_nodes = ses.FindAllShallow(METAKIND_ECS_COMPONENT_AI_STAGE_EXAMPLE);
	
	for(int i = 0; i < example_nodes.GetCount(); i++) {
		MetaNode& n = *example_nodes[i];
		examples.Set(i, 0, n.id);
		examples.Set(i, "IDX", i);
	}
	examples.SetCount(example_nodes.GetCount());
	if (!examples.IsCursor() && examples.GetCount())
		examples.SetCursor(0);
	
	DataExample();
}

void AiStageCtrl::DataExample() {
	MetaNode* ses = GetSession();
	if (!ses) {
		return;
	}
	
	structure_nodes.Clear();
	structure_nodes.Add(0, ses);
	structure.Clear();
	structure.SetRoot(MetaImgs::StageRoot(), "Session");
	structure_values.Clear();
	VisitNode(0, *ses, "");
	structure.OpenDeep(0);
	
	MetaNode* ex = GetExample();
	if (!ex)
		return;
	
}

void AiStageCtrl::VisitNode(int tree_i, MetaNode& n, String path) {
	for(int i = 0; i < n.sub.GetCount(); i++) {
		auto& s = n.sub[i];
		if (s.kind == METAKIND_ECS_COMPONENT_AI_STAGE_EXAMPLE)
			continue;
		Image img;
		bool is_value_node = false;
		switch (s.kind) {
			case METAKIND_ECS_COMPONENT_AI_STAGE_PROMPT_NODE: img = MetaImgs::StageNode(); break;
			case METAKIND_ECS_COMPONENT_AI_STAGE_PROMPT_QUERY: img = MetaImgs::StageQuery(); break;
			case METAKIND_ECS_COMPONENT_AI_STAGE_PROMPT_RESPONSE: img = MetaImgs::StageResponse(); break;
			case METAKIND_ECS_COMPONENT_AI_STAGE_PROMPT_COMMENT: img = MetaImgs::StageComment(); break;
			case METAKIND_ECS_COMPONENT_AI_STAGE_PROMPT_EXAMPLE_VALUE: img = MetaImgs::StageExample(); is_value_node = true; break;
			case METAKIND_ECS_COMPONENT_AI_STAGE_PROMPT_INPUT_VALUE:   img = MetaImgs::StageValue();   is_value_node = true; break;
		}
		int cur = structure.Add(tree_i, img, s.id);
		structure_nodes.Add(cur, &s);
		String s_path = path + "." + s.id;
		if (is_value_node) {
			Value val = GetExampleValue(s_path);
			String str = val.ToString();
			int val_cur = structure.Add(cur, MetaImgs::StageDynamic(), str);
			structure_values.Add(val_cur, s_path);
		}
		VisitNode(cur, s, s_path);
	}
}

void AiStageCtrl::DataItem() {
	
}

void AiStageCtrl::ToolMenu(Bar& bar) {
	
}

void AiStageCtrl::SessionMenu(Bar& b) {
	b.Add("Add session", THISBACK(AddSession));
	b.Add("Remove session", THISBACK(RemoveSession));
	b.Add("Rename session", THISBACK(RenameSession));
	b.Add("Duplicate session", THISBACK(DuplicateSession));
	b.Add("Set session's version", THISBACK(SetSessionVersion));
}

void AiStageCtrl::AddSession() {
	String name;
	if (!EditText(name, "Session's name", "Name"))
		return;
	StageThread& t = GetStageThread();
	for(int i = 0; i < sessions.GetCount(); i++) {
		if (sessions[i]->id == name) {
			PromptOK("Session with that name exists already");
			return;
		}
	}
	auto& ses = GetNode().Add(METAKIND_ECS_COMPONENT_AI_STAGE_SESSION, name);
	ses.type = "1";
	PostCallback(THISBACK(Data));
}

void AiStageCtrl::RemoveSession() {
	if (!session.IsCursor())
		return;
	int ses_id = session.Get("IDX");
	MetaNode* ses = sessions[ses_id];
	StageThread& t = GetStageThread();
	GetNode().Remove(ses);
	PostCallback(THISBACK(Data));
}

void AiStageCtrl::RenameSession() {
	if (!session.IsCursor())
		return;
	int ses_id = session.Get("IDX");
	StageThread& t = GetStageThread();
	auto& ses = *sessions[ses_id];
	String name = ses.id;
	if (!EditText(name, "Session's name", "Name"))
		return;
	ses.id = name;
	PostCallback(THISBACK(Data));
}

void AiStageCtrl::DuplicateSession() {
	if (!session.IsCursor())
		return;
	int ses_id = session.Get("IDX");
	StageThread& t = GetStageThread();
	const auto& ses0 = *sessions[ses_id];
	auto& ses1 = GetNode().Add(METAKIND_ECS_COMPONENT_AI_STAGE_SESSION, "");
	VisitCopy(ses0, ses1);
	PostCallback(THISBACK(Data));
}

void AiStageCtrl::SetSessionVersion() {
	if (!session.IsCursor())
		return;
	int ses_id = session.Get("IDX");
	StageThread& t = GetStageThread();
	auto& ses = *sessions[ses_id];
	String s = ses.type;
	if (!EditText(s, "Session's version", "Version"))
		return;
	ses.type = s;
	PostCallback(THISBACK(Data));
}

void AiStageCtrl::ExampleMenu(Bar& b) {
	b.Add("Add session", THISBACK(AddExample));
	b.Add("Remove session", THISBACK(RemoveExample));
	b.Add("Rename session", THISBACK(RenameExample));
	b.Add("Duplicate session", THISBACK(DuplicateExample));
}

void AiStageCtrl::AddExample() {
	MetaNode* ses = GetSession();
	if (!ses)
		return;
	String name;
	if (!EditText(name, "Example's name", "Name"))
		return;
	StageThread& t = GetStageThread();
	for(int i = 0; i < example_nodes.GetCount(); i++) {
		if (example_nodes[i]->id == name) {
			PromptOK("An example with that name exists already");
			return;
		}
	}
	auto& ex = ses->Add(METAKIND_ECS_COMPONENT_AI_STAGE_EXAMPLE, name);
	PostCallback(THISBACK(DataSession));
}

void AiStageCtrl::RemoveExample() {
	MetaNode* ses = GetSession();
	if (!ses)
		return;
	int ex_id = examples.Get("IDX");
	MetaNode* ex = example_nodes[ex_id];
	ses->Remove(ex);
	PostCallback(THISBACK(DataSession));
}

void AiStageCtrl::RenameExample() {
	MetaNode* ses = GetSession();
	if (!ses)
		return;
	int ex_id = examples.Get("IDX");
	MetaNode& ex = *example_nodes[ex_id];
	String name = ex.id;
	if (!EditText(name, "Example's name", "Name"))
		return;
	ex.id = name;
	PostCallback(THISBACK(DataSession));
}

void AiStageCtrl::DuplicateExample() {
	MetaNode* ses = GetSession();
	if (!ses)
		return;
	int ex_id = examples.Get("IDX");
	MetaNode& ex0 = *example_nodes[ex_id];
	auto& ex1 = ses->Add(METAKIND_ECS_COMPONENT_AI_STAGE_EXAMPLE, "");
	VisitCopy(ex0, ex1);
	PostCallback(THISBACK(DataSession));
}

void AiStageCtrl::EditExampleValue(bool string) {
	MetaNode* ses = GetSession();
	MetaNode* ex = GetExample();
	if (!ses || !ex || !structure.IsCursor())
		return;
	int cur = structure.GetCursor();
	String path = structure_values.Get(cur, "");
	if (path.IsEmpty())
		return;
	Value val = GetExampleValue(path);
	if (string) {
		String str = val.ToString();
		if (!EditText(str, "Value JSON", "Value JSON"))
			return;
		SetExampleValue(path, str);
	}
	else {
		String json = AsJSON(val);
		while (1) {
			try {
				if (!EditText(json, "Value JSON", "Value JSON"))
					return;
				SetExampleValue(path, ParseJSON(json));
				break;
			}
			catch (Exc e) {
				if (!PromptYesNo("JSON parse failed: " + e + ".\nWant to keep editing?"))
					return;
			}
		}
	}
	PostCallback(THISBACK(DataSession));
}

void AiStageCtrl::StageMenu(Bar& b) {
	if (structure.IsCursor()) {
		int cur = structure.GetCursor();
		int val_i = structure_values.Find(cur);
		if (val_i >= 0) {
			b.Add("Set String", THISBACK1(EditExampleValue, true));
			b.Add("Set JSON", THISBACK1(EditExampleValue, false));
			return;
		}
		b.Add("Add comment", THISBACK1(AddStageNode, METAKIND_ECS_COMPONENT_AI_STAGE_PROMPT_COMMENT));
		if (!cur) {
			b.Add("Add query", THISBACK1(AddStageNode, METAKIND_ECS_COMPONENT_AI_STAGE_PROMPT_QUERY));
			b.Add("Add response", THISBACK1(AddStageNode, METAKIND_ECS_COMPONENT_AI_STAGE_PROMPT_RESPONSE));
		}
		else {
			b.Add("Add", THISBACK1(AddStageNode, METAKIND_ECS_COMPONENT_AI_STAGE_PROMPT_NODE));
			b.Add("Add example value", THISBACK1(AddStageNode, METAKIND_ECS_COMPONENT_AI_STAGE_PROMPT_EXAMPLE_VALUE));
			b.Add("Add input value", THISBACK1(AddStageNode, METAKIND_ECS_COMPONENT_AI_STAGE_PROMPT_INPUT_VALUE));
		}
		b.Add("Remove", THISBACK(RemoveStageNode));
		b.Add("Rename", THISBACK(RenameStageNode));
		if (!cur) {
			b.Separator();
			b.Add("Make variation", Callback());
			b.Sub("Templates", [this](Bar& b) {
				if (structure_nodes.GetCount() > 1) {
					b.Add("Save template", THISBACK(SaveTemplate));
					b.Separator();
				}
				b.Sub("Remove", [this](Bar& b) {
					auto tmpls = GetNode().FindAllShallow(METAKIND_ECS_COMPONENT_AI_STAGE_SESSION_TEMPLATE);
					for(int i = 0; i < tmpls.GetCount(); i++) {
						MetaNode& n = *tmpls[i];
						b.Add(n.id, THISBACK1(RemoveTemplate, &n));
					}
				});
				auto tmpls = GetNode().FindAllShallow(METAKIND_ECS_COMPONENT_AI_STAGE_SESSION_TEMPLATE);
				for(int i = 0; i < tmpls.GetCount(); i++) {
					MetaNode& n = *tmpls[i];
					b.Add(n.id, THISBACK1(LoadTemplate, &n));
				}
			});
		}
	}
}

void AiStageCtrl::SaveTemplate() {
	if (!session.IsCursor())
		return;
	int ses_id = session.Get("IDX");
	StageThread& t = GetStageThread();
	auto& ses = *sessions[ses_id];
	auto& dest = GetNode().Add(METAKIND_ECS_COMPONENT_AI_STAGE_SESSION_TEMPLATE, ses.id);
	VisitCopy(ses, dest);
	dest.kind = METAKIND_ECS_COMPONENT_AI_STAGE_SESSION_TEMPLATE;
}

void AiStageCtrl::RemoveTemplate(MetaNode* n) {
	GetNode().Remove(n);
}

void AiStageCtrl::LoadTemplate(MetaNode* n) {
	if (!session.IsCursor())
		return;
	int ses_id = session.Get("IDX");
	StageThread& t = GetStageThread();
	auto& ses = *sessions[ses_id];
	VisitCopy(*n, ses);
	ses.kind = METAKIND_ECS_COMPONENT_AI_STAGE_SESSION;
	PostCallback(THISBACK(Data));
}

void AiStageCtrl::AddStageNode(int kind) {
	if (!structure.IsCursor()) return;
	int cur = structure.GetCursor();
	int i = structure_nodes.Find(cur);
	if (i < 0) {PromptOK("Internal error"); return;}
	MetaNode& n = *structure_nodes[i];
	String id;
	if (!EditText(id, "Node's id", "id"))
		return;
	MetaNode& s = n.Add(kind, id);
	PostCallback(THISBACK(Data));
}

void AiStageCtrl::RenameStageNode() {
	if (!structure.IsCursor()) return;
	int cur = structure.GetCursor();
	int i = structure_nodes.Find(cur);
	if (i < 0) {PromptOK("Internal error"); return;}
	MetaNode& n = *structure_nodes[i];
	String id = n.id;
	if (!EditText(id, "Node's id", "id"))
		return;
	n.id = id;
	structure.Set(cur, id);
}

void AiStageCtrl::RemoveStageNode() {
	if (!structure.IsCursor()) return;
	int cur = structure.GetCursor();
	if (!cur) return; // can't remove root
	int i = structure_nodes.Find(cur);
	if (i < 0) {PromptOK("Internal error"); return;}
	MetaNode& n = *structure_nodes[i];
	int owner = structure.GetParent(cur);
	i = structure_nodes.Find(owner);
	if (i < 0) {PromptOK("Internal error"); return;}
	MetaNode& o = *structure_nodes[i];
	o.Remove(&n);
	PostCallback(THISBACK(Data));
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
	tabs.Add(stage.SizePos(), "Stage");
	tabs.Add(placeholder.SizePos(), "Action Planner");
	tabs.Add(placeholder.SizePos(), "DM");
	tabs.Add(placeholder.SizePos(), "Team DM");
	tabs.Add(chain.SizePos(), "Filesystem-chat");
	tabs.Add(placeholder.SizePos(), "Animation");
	tabs.Add(placeholder.SizePos(), "Adventure");
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
	chain.ext = 0;
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
		("completion", (AiThreadCtrlBase&)completion)
		("chat", (AiThreadCtrlBase&)chat)
		//("stage", (AiThreadCtrlBase&)stage)
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
}

void PlaygroundCtrl::LoadThis() {
	if (omni)
		VisitFromJsonFile(*omni, ConfigFile("playground.json"));
	VisitFromJsonFile(*this, ConfigFile("playground-gui.json"));
}

void PlaygroundCtrl::SetNode(MetaNode& n) {
	auto& stage_n = n.GetAdd("stage", "", METAKIND_ECS_COMPONENT_AI_STAGE);
	ASSERT(stage_n.ext);
	stage.ext = &*stage_n.ext;
	
	auto& chain_n = n.GetAdd("chain", "", METAKIND_ECS_COMPONENT_AI_CHAIN);
	ASSERT(chain_n.ext);
	chain.ext = &*chain_n.ext;
}



PlaygroundApp::PlaygroundApp() {
	Title("Playground").MaximizeBox().MinimizeBox();
	Sizeable().Maximize();
	
	Add(pg.SizePos());
	AddFrame(menu);
	PostCallback(THISBACK(UpdateMenu));
	
	pg.WhenTab << THISBACK(UpdateMenu);
	pg.CreateThread();
	
	omni_node.Create();
	VisitFromJsonFile(*omni_node, ConfigFile("playground-root.json"));
	pg.SetNode(*omni_node);
}

PlaygroundApp::~PlaygroundApp() {
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
