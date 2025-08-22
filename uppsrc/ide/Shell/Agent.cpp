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
}

void IdeAgent::Menu(Bar& bar) {
	
}

void IdeAgent::OnTab() {
	int i = tabs->GetCursor();
	
	edit.Show(i == 0);
	if (i == 0) edit.SetFocus();
	
}

END_UPP_NAMESPACE
