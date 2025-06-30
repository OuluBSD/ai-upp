#include "AICtrl.h"


NAMESPACE_UPP


INITIALIZER_COMPONENT_CTRL(AiProgram, AiProgramCtrl)

AiProgramCtrl::AiProgramCtrl() {
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
		VfsValue& n = *programs[idx];
		n.value = prog.GetData();
	};
	stage.Highlight("cpp");
	stage.LineNumbers(true);
	stage.WhenAction = [this] {
		if (!stagelist.IsCursor()) return;
		int idx = stagelist.Get("IDX");
		VfsValue& n = *stages[idx];
		n.value = stage.GetData();
	};
}

void AiProgramCtrl::Data() {
	if (!ext) return;
	DataProgramList();
	DataStageList();
	DataBottom();
}

void AiProgramCtrl::DataList(ArrayCtrl& list, Vector<VfsValue*>& nodes, hash_t type_hash) {
	Entity* eng = GetValue().FindOwner<Entity>();
	ASSERT(eng);
	if (!eng) return;
	
	nodes = eng->val.FindTypeAllShallow(type_hash);
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

void AiProgramCtrl::DataProgramList() {
	DataList(proglist, programs, AsTypeHash<VfsProgram>());
}

void AiProgramCtrl::DataProgram() {
	if (!proglist.IsCursor()) return;
	int idx = proglist.Get("IDX");
	VfsValue& n = *programs[idx];
	prog.SetData(n.value);
}

void AiProgramCtrl::DataStageList() {
	DataList(stagelist, stages, AsTypeHash<VfsFarStage>());
}

void AiProgramCtrl::DataStage() {
	if (!stagelist.IsCursor()) return;
	int idx = stagelist.Get("IDX");
	VfsValue& n = *stages[idx];
	const AstValue* a = n;
	if (a) {
		n.value = String();
	}
	stage.SetData(n.value);
}

void AiProgramCtrl::DataBottom() {
	
}

bool AiProgramCtrl::CompileStages(bool force) {
	bool succ = false;
	
	log.Clear();
	
	Agent* agent = ext->val.FindOwnerWith<Agent>();
	ASSERT(agent);
	
	for(int i = 0; i < stages.GetCount(); i++) {
		VfsValue& n = *stages[i];
		
		String esc = n.value;
		if (esc.IsEmpty()) continue;
		
		succ = agent->CompileStage(n, force, THISBACK(PrintLog));
	}
	return true;
}

bool AiProgramCtrl::Compile(bool force) {
	if (!proglist.IsCursor()) return false;
	int idx = proglist.Get("IDX");
	VfsValue& n = *programs[idx];
	bool succ = false;
	
	String esc = n.value;
	if (esc.IsEmpty()) return false;
	
	log.Clear();
	
	Agent* agent = ext->val.FindOwnerWith<Agent>();
	ASSERT(agent);
	succ = agent->Compile(esc, force, THISBACK(PrintLog));
	
	return true;
}

bool AiProgramCtrl::Run() {
	if (agent) {
		agent->Stop();
	}
	if (!Compile(false))
		return false;
	
	log.Clear();
	
	agent = ext->val.FindOwnerWith<Agent>();
	ASSERT(agent);
	
	agent->WhenPrint = THISBACK(Print);
	agent->WhenInput = THISBACK(Input);
	
	//agent->SetSeparateThread();
	agent->Start(THISBACK(PrintLog), [this](bool succ) {
		if (!succ) {
			PostCallback([this]{
				log.Append("Running failed\n");
				log.SetCursor(log.GetLength());
			});
		}
	});
	
	return true;
}

void AiProgramCtrl::Print(EscEscape& e) {
	String s = e[0];
	
	PostCallback([this,s]{
		log.Append(s + "\n");
		log.SetCursor(log.GetLength());
	});
}

void AiProgramCtrl::Input(EscEscape& e) {
	String s;
	EditText(s, "Input", "Text");
	e = s;
}

void AiProgramCtrl::PrintLog(Vector<ProcMsg>& msgs) {
	String s;
	for (ProcMsg& m : msgs) {
		s << m.ToString() << "\n";
	}
	PostCallback([this,s]{
		log.Append(s);
		log.SetCursor(log.GetLength());
	});
}

void AiProgramCtrl::ToolMenu(Bar& b) {
	b.Add("Compile Stages", [this]{CompileStages(true);}).Key(K_F8);
	b.Add("Compile Program", [this]{Compile(true);}).Key(K_F7);
	b.Add("Run", [this]{Run();}).Key(K_F5);
}

void AiProgramCtrl::ProgramMenu(Bar& b) {
	b.Add("Add program", THISBACK(AddProgram));
	b.Add("Remove program", THISBACK(RemoveProgram));
	b.Add("Rename program", THISBACK(RenameProgram));
	b.Add("Duplicate program", THISBACK(DuplicateProgram));
}

void AiProgramCtrl::AddProgram() {
	String name;
	if (!EditText(name, "Program's name", "Name"))
		return;
	
	for(int i = 0; i < programs.GetCount(); i++) {
		if (programs[i]->id == name) {
			PromptOK("Program with that name exists already");
			return;
		}
	}
	Entity* eng = GetValue().FindOwner<Entity>();
	ASSERT(eng);
	if (!eng) return;
	auto& ses = eng->val.Add<VfsProgram>(name);
	PostCallback(THISBACK(DataProgramList));
}

void AiProgramCtrl::RemoveProgram() {
	if (!proglist.IsCursor())
		return;
	int id = proglist.Get("IDX");
	VfsValue* n = programs[id];
	GetValue().Remove(n);
	PostCallback(THISBACK(DataProgramList));
}

void AiProgramCtrl::RenameProgram() {
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

void AiProgramCtrl::DuplicateProgram() {
	if (!proglist.IsCursor())
		return;
	int id = proglist.Get("IDX");
	const auto& n0 = *programs[id];
	Entity* eng = GetValue().FindOwner<Entity>();
	ASSERT(eng);
	if (!eng) return;
	auto& m1 = eng->val.Add<VfsProgram>("Duplicate of " + n0.id);
	VisitCopy(n0, m1.val);
	PostCallback(THISBACK(DataProgramList));
}

void AiProgramCtrl::StageMenu(Bar& b) {
	b.Add("Add stage", THISBACK(AddStage));
	b.Add("Remove stage", THISBACK(RemoveStage));
	b.Add("Rename stage", THISBACK(RenameStage));
	b.Add("Duplicate stage", THISBACK(DuplicateStage));
}

void AiProgramCtrl::AddStage() {
	String name;
	if (!EditText(name, "Stage's name", "Name"))
		return;
	
	for(int i = 0; i < stages.GetCount(); i++) {
		if (stages[i]->id == name) {
			PromptOK("Stage with that name exists already");
			return;
		}
	}
	auto& ses = GetValue().Add<VfsFarStage>(name);
	PostCallback(THISBACK(DataStageList));
}

void AiProgramCtrl::RemoveStage() {
	if (!stagelist.IsCursor())
		return;
	int id = stagelist.Get("IDX");
	VfsValue* n = stages[id];
	GetValue().Remove(n);
	PostCallback(THISBACK(DataStageList));
}

void AiProgramCtrl::RenameStage() {
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

void AiProgramCtrl::DuplicateStage() {
	if (!stagelist.IsCursor())
		return;
	int id = stagelist.Get("IDX");
	const auto& n0 = *stages[id];
	auto& m1 = GetValue().Add<VfsFarStage>("");
	VisitCopy(n0, m1.val);
	PostCallback(THISBACK(DataStageList));
}

VfsValue* AiProgramCtrl::GetProgram() {
	if (!proglist.IsCursor())
		return 0;
	int ses_i = proglist.Get("IDX");
	return programs[ses_i];
}

VfsValue* AiProgramCtrl::GetStage() {
	if (!stagelist.IsCursor())
		return 0;
	int ex_i = stagelist.Get("IDX");
	return stages[ex_i];
}


END_UPP_NAMESPACE
