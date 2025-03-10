#include "ide.h"

#ifdef flagAI
#include <AI/AI.h>


INITBLOCK {
	RegisterGlobalConfig("AITaskDlg2");
}

struct AITaskDlg : TopWindow {
	typedef AITaskDlg CLASSNAME;
	
	virtual bool Key(dword key, int count);
	
	void Serialize(Stream& s);
	
	TabCtrl tabs;
	MetaEnvTree menv;
	TaskCtrl tasks;
	PlaygroundCtrl playground;
	
	Ide* theide = 0;
	
	AITaskDlg();
	
	void Update();
};

AITaskDlg::AITaskDlg() {
	Maximize();
	MaximizeBox();
	MinimizeBox();
	Title("AI");
	
	Add(tabs.SizePos());
	tabs.Add(tasks.SizePos(), "Tasks");
	tabs.Add(playground.SizePos(), "Playground");
	tabs.Add(menv.SizePos(), "Meta Env. Tree");
	
	
	
	
}

bool AITaskDlg::Key(dword key, int count) {
	if (key == K_ESCAPE) {
		Close();
		return true;
	}
	return false;
}

void AITaskDlg::Update() {
	
}

void AITaskDlg::Serialize(Stream& s) {
	
}

void Ide::OpenAITaskDlg()
{
	TaskMgr::Setup(this);
	AITaskDlg dlg;
	LoadFromGlobal(dlg, "AITaskDlg");
	dlg.theide = this;
	dlg.Update();
	dlg.Execute();
	StoreToGlobal(dlg, "AITaskDlg");
}

#endif
