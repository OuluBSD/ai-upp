#include <ide/ide.h>

NAMESPACE_UPP

IdeAgent::IdeAgent() {
	tabs.Create();
	Add(hsplit.HSizePos(0,100).VSizePos());
	Add(tabs->RightPos(0,100).VSizePos());
	hsplit.Horz() << tasklist << placeholder;
	hsplit.SetPos(2000);
	
	placeholder.Add(edit.SizePos());
	placeholder.SetFrame(InsetFrame());
	
	tabs->Add(IdeImg::IconBuilding(), "Prompt", Black());
	tabs->Add(IdeImg::IconDebugging(), "Process", Black());
	tabs->Add(IdeImg::IconRunning(), "Result", Black());
	tabs->SetCursor(0);
	tabs->WhenAction << THISBACK(OnTab);
	OnTab();
	
	tasklist.AddColumn("Package");
	tasklist.AddColumn("Task name");
	
	tc.Set(-1000, THISBACK(Data));
}

void IdeAgent::Menu(Bar& bar) {
	
}

void IdeAgent::OnTab() {
	int i = tabs->GetCursor();
	
	edit.Show(i == 0);
	if (i == 0) edit.SetFocus();
	
	PostCallback(THISBACK(Data));
}

void IdeAgent::Data() {
	
	
	DataTasks();
}

void IdeAgent::DataTasks() {
	MetaEnvironment& env = MetaEnv();
	Ide& ide = *TheIde();
	WorkspaceWork& wspc = *dynamic_cast<Ide*>(TheIdeContext());
	
	//LOG(env.root.GetTreeString());
	int row = 0;
	for(int i = 0; i < wspc.package.GetCount(); i++) {
		Value name = wspc.package.Get(i);
		//LOG(name.ToString());
		
		VfsPath path;
		path.Add(name);
		path.Add("agent");
		
		VfsValue* agent = env.root.FindPath(path);
		if (!agent)
			continue;
		
		LOG(path.ToString());
		
		
		Vector<Ptr<VfsValue>> tasks = agent->FindAllWith<AgentTaskExt>();
		
		for (Ptr<VfsValue>& task : tasks) {
			if (!task) continue;
			tasklist.Set(0, 0, name);
			tasklist.Set(0, 1, task->id);
			row++;
		}
	}
	tasklist.SetCount(row);
	
}

END_UPP_NAMESPACE
