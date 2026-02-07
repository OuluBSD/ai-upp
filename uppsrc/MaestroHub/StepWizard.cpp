#include "MaestroHub.h"

NAMESPACE_UPP

StepWizard::StepWizard() {
	CtrlLayout(*this, "Step Wizard");
	
	Add(ai_assist.SetLabel("AI Assist").RightPos(10, 80).TopPos(5, 24));
	ai_assist << [=] {
		String prompt;
		prompt << "I am authoring a Runbook step.\n";
		prompt << "Action: " << (String)action.GetData() << "\n";
		prompt << "Command: " << (String)command.GetData() << "\n";
		prompt << "Expected Result: " << (String)result.GetData() << "\n";
		prompt << "Please suggest improvements, error handling, or specific command variants for Linux/Windows.";
		
		if(WhenAssist) WhenAssist(prompt);
	};
	
	variants.AddColumn("OS / Key");
	variants.AddColumn("Command");
	
	btn_add_variant << THISBACK(AddVariant);
	btn_remove_variant << THISBACK(RemoveVariant);
	
	ok << [=] { Break(IDOK); };
	cancel << [=] { Break(IDCANCEL); };
}

void StepWizard::SetStep(const RunbookStep& s) {
	step = s;
	actor.SetData(s.actor);
	action.SetData(s.action);
	command.SetData(s.command);
	result.SetData(s.expected);
	
	variants.Clear();
	for(int i = 0; i < s.variants.GetCount(); i++) {
		variants.Add(s.variants.GetKey(i), s.variants.GetValue(i));
	}
}

RunbookStep StepWizard::GetStep() {
	step.actor = actor.GetData();
	step.action = action.GetData();
	step.command = command.GetData();
	step.expected = result.GetData();
	
	step.variants.Clear();
	for(int i = 0; i < variants.GetCount(); i++) {
		step.variants.Add(variants.Get(i, 0), variants.Get(i, 1));
	}
	return step;
}

void StepWizard::AddVariant() {
	String os, cmd;
	if(EditText(os, "New Variant", "OS/Key:") && EditText(cmd, "New Variant", "Command:")) {
		variants.Add(os, cmd);
	}
}

void StepWizard::RemoveVariant() {
	if(variants.IsCursor()) {
		variants.Remove(variants.GetCursor());
	}
}

END_UPP_NAMESPACE