#include "ide.h"

#ifdef flagAI

INITBLOCK {
	RegisterGlobalConfig("AITaskDlg2");
}

struct AITaskDlg : TopWindow {
	typedef AITaskDlg CLASSNAME;
	
	virtual bool Key(dword key, int count);
	
	void Serialize(Stream& s);
	
	// Tasks tab
	Splitter hsplit, vsplit;
	ArrayCtrl task_list;
	DocEdit input, output;
	
	Ide* theide = 0;
	
	AITaskDlg();
	
	void Update();
};

AITaskDlg::AITaskDlg() {
	Title("AI");
	Add(hsplit.SizePos());
	
	hsplit.Horz(task_list, vsplit);
	vsplit.Vert(input, output);
	
	Maximize();
	MaximizeBox();
	MinimizeBox();
	
	
	
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
	AITaskDlg dlg;
	LoadFromGlobal(dlg, "AITaskDlg");
	dlg.theide = this;
	dlg.Update();
	dlg.Execute();
	StoreToGlobal(dlg, "AITaskDlg");
}

#endif
