#include "RunbookEditor.h"
#include "StepWizard.h"

NAMESPACE_UPP

RunbookEditor::RunbookEditor() {
	CtrlLayout(*this, "Runbook Visual Editor");
	
	scope.Add("product");
	scope.Add("user");
	scope.Add("manager");
	scope.Add("ui");
	scope.Add("code");
	scope.Add("reverse_engineering");
	
	status.Add("proposed");
	status.Add("approved");
	status.Add("deprecated");
	
	steps.AddColumn("N");
	steps.AddColumn("Actor");
	steps.AddColumn("Action");
	steps.AddColumn("Command");
	
	btn_add_step << THISBACK(AddStep);
	btn_edit_step << THISBACK(EditStep);
	btn_remove_step << THISBACK(RemoveStep);
	btn_save << THISBACK(OnSave);
	btn_cancel << [=] { Close(); };
	btn_resolve << THISBACK(OnResolve);
	
title.WhenAction = THISBACK(UpdatePreview);
	goal.WhenAction = THISBACK(UpdatePreview);
}

void RunbookEditor::New(const String& maestro_root) {
	root = maestro_root;
	rbm.Create(root);
	work_rb.Create();
	work_rb->id = "rb-" + FormatIntHex(Random(), 8);
	work_rb->title = "New Runbook";
	UpdateUI();
}

void RunbookEditor::Load(const String& maestro_root, const String& id) {
	root = maestro_root;
	rbm.Create(root);
	work_rb.Create();
	*work_rb = rbm->LoadRunbook(id);
	UpdateUI();
}

void RunbookEditor::UpdateUI() {
	if(!work_rb) return;
	title.SetData(work_rb->title);
	goal.SetData(work_rb->goal);
	
	steps.Clear();
	for(const auto& s : work_rb->steps)
		steps.Add(s.n, s.actor, s.action, s.command);
		
	UpdatePreview();
}

void RunbookEditor::UpdatePreview() {
	if(!work_rb) return;
	String qtf;
	qtf << "[*@3 " << DeQtf(title.GetData().ToString()) << "]&";
	qtf << "[* Goal:] " << DeQtf(goal.GetData().ToString()) << "&";
	qtf << "[* Steps: ] " << work_rb->steps.GetCount();
	preview.SetQTF(qtf);
}

void RunbookEditor::AddStep() {
	if(!work_rb) return;
	
	StepWizard dlg;
	RunbookStep s;
	s.n = work_rb->steps.GetCount() + 1;
	s.actor = "dev";
	
	dlg.SetStep(s);
	if(dlg.Run() == IDOK) {
		work_rb->steps.Add(dlg.GetStep());
		UpdateUI();
	}
}

void RunbookEditor::EditStep() {
	if(!work_rb || !steps.IsCursor()) return;
	
	int idx = steps.GetCursor();
	StepWizard dlg;
	dlg.SetStep(work_rb->steps[idx]);
	
	if(dlg.Run() == IDOK) {
		work_rb->steps[idx] = dlg.GetStep();
		UpdateUI();
	}
}

void RunbookEditor::RemoveStep() {
	if(steps.IsCursor() && work_rb) {
		work_rb->steps.Remove(steps.GetCursor());
		// Renumber
		for(int i = 0; i < work_rb->steps.GetCount(); i++)
			work_rb->steps[i].n = i + 1;
		UpdateUI();
	}
}

void RunbookEditor::OnSave() {
	if(!work_rb) return;
	work_rb->title = title.GetData();
	work_rb->goal = goal.GetData();
	if(rbm->SaveRunbook(*work_rb)) {
		PromptOK("Runbook saved successfully.");
		Break(IDOK);
	} else {
		Exclamation("Failed to save runbook.");
	}
}

void RunbookEditor::OnResolve() {
	String text;
	if(EditText(text, "AI Resolve", "Enter freeform text to generate runbook steps:")) {
		Cout() << "Calling AI Resolve...\n";
		if(rbm && work_rb) {
			*work_rb = rbm->Resolve(text);
			UpdateUI();
		}
	}
}

END_UPP_NAMESPACE