#include "AICtrl.h"


NAMESPACE_UPP


INITIALIZER_COMPONENT_CTRL(VfsProgram, VfsProgramCtrl)

VfsProgramCtrl::VfsProgramCtrl() {
	Add(vsplit.SizePos());
	
	vsplit.Vert() << hsplit << btabs;
	vsplit.SetPos(8000);
	
	btabs.Add(log.SizePos(), "Log");
	
	hsplit.Horz() << prjsplit << prj << rtabs;
	hsplit.SetPos(1000,0).SetPos(5000,1);
	
	rtabs.Add(memoryctrl.SizePos(), "Memory");
	rtabs.Add(stagectrl.SizePos(), "Stage");
	
	stagectrl.Horz() << stagesplit << stage;
	stagectrl.SetPos(2000);
	memoryctrl.Vert() << memtree;
	
	prjsplit.Vert() << prjlist << sessionlist << iterlist;
	
	prjlist.AddColumn("Project");
	prjlist.AddIndex("IDX");
	prjlist.WhenBar = THISBACK(ProjectMenu);
	prjlist.WhenCursor = THISBACK(DataProject);
	
	sessionlist.AddColumn("Session");
	sessionlist.AddIndex("IDX");
	sessionlist.WhenBar = THISBACK(SessionMenu);
	sessionlist.WhenCursor = THISBACK(DataSession);
	
	iterlist.AddColumn("Iteration");
	iterlist.AddIndex("IDX");
	iterlist.WhenBar = THISBACK(IterationMenu);
	iterlist.WhenCursor = THISBACK(DataIteration);
	
	stagesplit.Vert() << stagelist << querylist;
	
	stagelist.AddColumn("Stage");
	stagelist.AddIndex("IDX");
	stagelist.WhenBar = THISBACK(StageMenu);
	stagelist.WhenCursor = THISBACK(DataStage);
	
	querylist.AddColumn("Query");
	querylist.AddIndex("IDX");
	querylist.WhenBar = THISBACK(QueryMenu);
	querylist.WhenCursor = THISBACK(DataQuery);
	
	prj.Highlight("cpp");
	prj.LineNumbers(true);
	prj.WhenAction = [this] {
		if (!prjlist.IsCursor()) return;
		int idx = prjlist.Get("IDX");
		VfsValue& n = *projects[idx];
		n.value = prj.GetData();
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

void VfsProgramCtrl::Data() {
	if (!ext) return;
	DataProjectList();
	DataStageList();
	DataBottom();
}

void VfsProgramCtrl::DataList(ArrayCtrl& list, VfsValue& parent, Vector<VfsValue*>& nodes, hash_t type_hash, Event<> WhenData) {
	nodes = parent.FindTypeAllShallow(type_hash);
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
		WhenData();
}

void VfsProgramCtrl::DataProjectList() {
	VfsProgram& prog = GetExt<VfsProgram>();
	DataList(prjlist, prog.val, projects, AsTypeHash<VfsProgramProject>(), THISBACK(DataProject));
}

void VfsProgramCtrl::DataProject() {
	if (!prjlist.IsCursor()) {cur_project = 0; return;}
	int idx = prjlist.Get("IDX");
	VfsValue& n = *projects[idx];
	cur_project = &n;
	
	if (n.value.Is<String>())
		prj.SetData(n.value);
	else
		prj.SetData(n.value.ToString());
	
	DataList(sessionlist, n, sessions, AsTypeHash<VfsProgramSession>(), THISBACK(DataSession));
}

void VfsProgramCtrl::DataSession() {
	if (!prjlist.IsCursor() || !sessionlist.IsCursor()) {cur_session = 0; return;}
	int ses_idx = sessionlist.Get("IDX");
	VfsValue& n = *sessions[ses_idx];
	cur_session = &n;
	
	DataList(iterlist, n, iterations, AsTypeHash<VfsProgramIteration>(), THISBACK(DataIteration));
}

void VfsProgramCtrl::DataIteration() {
	
	// cur_iter = 
	
}

void VfsProgramCtrl::DataStageList() {
	VfsProgram& prog = this->GetExt<VfsProgram>();
	DataList(stagelist, prog.val, stages, AsTypeHash<VfsFarStage>(), THISBACK(DataStage));
}

void VfsProgramCtrl::DataStage() {
	if (!stagelist.IsCursor()) return;
	int idx = stagelist.Get("IDX");
	VfsValue& n = *stages[idx];
	const AstValue* a = n;
	if (a) {
		n.value = String();
	}
	stage.SetData(n.value);
}

void VfsProgramCtrl::DataQuery() {
	
}

void VfsProgramCtrl::DataBottom() {
	
}

bool VfsProgramCtrl::CompileStages(bool force) {
	bool succ = false;
	
	log.Clear();
	
	Agent* agent = ext->val.FindOwnerWith<Agent>();
	if (!agent) {
		PrintString("error: could not find Agent in MetaEnv");
		return false;
	}
	
	for(int i = 0; i < stages.GetCount(); i++) {
		VfsValue& n = *stages[i];
		
		String esc = n.value;
		if (esc.IsEmpty()) continue;
		
		succ = agent->CompileStage(n, force, THISBACK(PrintLog));
	}
	return true;
}

bool VfsProgramCtrl::Compile(bool force) {
	if (!prjlist.IsCursor()) return false;
	int idx = prjlist.Get("IDX");
	VfsValue& n = *projects[idx];
	bool succ = false;
	
	String esc = n.value;
	if (esc.IsEmpty()) return false;
	
	log.Clear();
	
	Agent* agent = ext->val.FindOwnerWith<Agent>();
	if (!agent) {
		PrintString("error: could not find Agent in MetaEnv");
		return false;
	}
	
	succ = agent->Compile(esc, force, THISBACK(PrintLog));
	
	return true;
}

bool VfsProgramCtrl::Run() {
	if (agent) {
		agent->Stop();
	}
	if (!Compile(false))
		return false;
	
	log.Clear();
	
	agent = ext->val.FindOwnerWith<Agent>();
	if (!agent) {
		PrintString("error: could not find Agent in MetaEnv");
		return false;
	}
	
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

void VfsProgramCtrl::Print(EscEscape& e) {
	PrintString(e[0]);
}

void VfsProgramCtrl::PrintString(String s) {
	PostCallback([this,s]{
		log.Append(s + "\n");
		log.SetCursor(log.GetLength());
	});
}

void VfsProgramCtrl::Input(EscEscape& e) {
	String s;
	EditText(s, "Input", "Text");
	e = s;
}

void VfsProgramCtrl::PrintLog(Vector<ProcMsg>& msgs) {
	String s;
	for (ProcMsg& m : msgs) {
		s << m.ToString() << "\n";
	}
	PostCallback([this,s]{
		log.Append(s);
		log.SetCursor(log.GetLength());
	});
}

void VfsProgramCtrl::ToolMenu(Bar& b) {
	b.Add("Compile Stages", [this]{CompileStages(true);}).Key(K_F8);
	b.Add("Compile Program", [this]{Compile(true);}).Key(K_F7);
	b.Add("Run", [this]{Run();}).Key(K_F5);
}

void VfsProgramCtrl::ProjectMenu(Bar& b) {
	b.Add("Add project", THISBACK(AddProject));
	b.Add("Remove project", THISBACK(RemoveProject));
	b.Add("Rename project", THISBACK(RenameProject));
	b.Add("Duplicate project", THISBACK(DuplicateProject));
}

void VfsProgramCtrl::SessionMenu(Bar& b) {
	b.Add("Add session", THISBACK(AddSession));
	b.Add("Remove session", THISBACK(RemoveSession));
	b.Add("Rename session", THISBACK(RenameSession));
	b.Add("Duplicate session", THISBACK(DuplicateSession));
}

void VfsProgramCtrl::IterationMenu(Bar& b) {
	b.Add("Rename iteration", THISBACK(RenameIteration));
}

void VfsProgramCtrl::QueryMenu(Bar& b) {
	
}

void VfsProgramCtrl::AddProject() {
	String name;
	if (!EditText(name, "Project's name", "Name"))
		return;
	
	for(int i = 0; i < projects.GetCount(); i++) {
		if (projects[i]->id == name) {
			PromptOK("Project with that name exists already");
			return;
		}
	}
	VfsProgram& prog = this->GetExt<VfsProgram>();
	auto& ses = prog.val.Add<VfsProgramProject>(name);
	PostCallback(THISBACK(DataProjectList));
}

void VfsProgramCtrl::RemoveProject() {
	if (!prjlist.IsCursor())
		return;
	int id = prjlist.Get("IDX");
	VfsValue* n = projects[id];
	GetValue().Remove(n);
	PostCallback(THISBACK(DataProjectList));
}

void VfsProgramCtrl::RenameProject() {
	if (!prjlist.IsCursor())
		return;
	int id = prjlist.Get("IDX");
	auto& n = *projects[id];
	String name = n.id;
	if (!EditText(name, "Project's name", "Name"))
		return;
	n.id = name;
	PostCallback(THISBACK(DataProjectList));
}

void VfsProgramCtrl::DuplicateProject() {
	if (!prjlist.IsCursor())
		return;
	int id = prjlist.Get("IDX");
	const auto& n0 = *projects[id];
	VfsProgram& prog = this->GetExt<VfsProgram>();
	auto& m1 = prog.val.Add<VfsProgramProject>("Duplicate of " + n0.id);
	VisitCopy(n0, m1.val);
	PostCallback(THISBACK(DataProjectList));
}

void VfsProgramCtrl::AddSession() {
	String name;
	if (!cur_project) return;
	if (!EditText(name, "Session's name", "Name"))
		return;
	
	for(int i = 0; i < sessions.GetCount(); i++) {
		if (sessions[i]->id == name) {
			PromptOK("Session with that name exists already");
			return;
		}
	}
	int id = prjlist.Get("IDX");
	auto& parent = *projects[id];
	auto& ses = parent.Add<VfsProgramSession>(name);
	PostCallback(THISBACK(DataProject));
}

void VfsProgramCtrl::RemoveSession() {
	if (!sessionlist.IsCursor() || !cur_project)
		return;
	int id = sessionlist.Get("IDX");
	VfsValue* n = sessions[id];
	if (cur_project)
		cur_project->Remove(n);
	PostCallback(THISBACK(DataProject));
}

void VfsProgramCtrl::RenameSession() {
	if (!sessionlist.IsCursor())
		return;
	int id = sessionlist.Get("IDX");
	auto& n = *sessions[id];
	String name = n.id;
	if (!EditText(name, "Session's name", "Name"))
		return;
	n.id = name;
	PostCallback(THISBACK(DataProject));
}

void VfsProgramCtrl::DuplicateSession() {
	if (!sessionlist.IsCursor() || !cur_project)
		return;
	int parent_id = prjlist.Get("IDX");
	auto& parent = *projects[parent_id];
	int id = sessionlist.Get("IDX");
	const auto& n0 = *sessions[id];
	auto& m1 = parent.Add<VfsProgramSession>("Duplicate of " + n0.id);
	VisitCopy(n0, m1.val);
	PostCallback(THISBACK(DataProject));
}

void VfsProgramCtrl::RenameIteration() {
	if (!iterlist.IsCursor())
		return;
	int id = iterlist.Get("IDX");
	auto& n = *iterations[id];
	String name = n.id;
	if (!EditText(name, "Iteration's name", "Name"))
		return;
	n.id = name;
	PostCallback(THISBACK(DataSession));
}

void VfsProgramCtrl::StageMenu(Bar& b) {
	b.Add("Add stage", THISBACK(AddStage));
	b.Add("Remove stage", THISBACK(RemoveStage));
	b.Add("Rename stage", THISBACK(RenameStage));
	b.Add("Duplicate stage", THISBACK(DuplicateStage));
}

void VfsProgramCtrl::AddStage() {
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

void VfsProgramCtrl::RemoveStage() {
	if (!stagelist.IsCursor())
		return;
	int id = stagelist.Get("IDX");
	VfsValue* n = stages[id];
	GetValue().Remove(n);
	PostCallback(THISBACK(DataStageList));
}

void VfsProgramCtrl::RenameStage() {
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

void VfsProgramCtrl::DuplicateStage() {
	if (!stagelist.IsCursor())
		return;
	int id = stagelist.Get("IDX");
	const auto& n0 = *stages[id];
	auto& m1 = GetValue().Add<VfsFarStage>("");
	VisitCopy(n0, m1.val);
	PostCallback(THISBACK(DataStageList));
}

VfsValue* VfsProgramCtrl::GetProgram() {
	if (!prjlist.IsCursor())
		return 0;
	int ses_i = prjlist.Get("IDX");
	return projects[ses_i];
}

VfsValue* VfsProgramCtrl::GetStage() {
	if (!stagelist.IsCursor())
		return 0;
	int ex_i = stagelist.Get("IDX");
	return stages[ex_i];
}


END_UPP_NAMESPACE
