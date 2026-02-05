#include "StateEditor.h"

NAMESPACE_UPP

StateEditor::StateEditor() {
	CtrlLayout(*this, "Workflow State Machine Editor");
	
split.Horz(puml_editor, graph_view);
	
toolbar.Set(THISBACK(OnToolbar));
	
puml_editor.WhenAction = [=] {
		SetTimeCallback(500, THISBACK(UpdatePreview), 1);
	};
}

void StateEditor::OnToolbar(Bar& bar) {
	bar.Add("Save", THISBACK(Save));
	bar.Separator();
	bar.Add("New State", THISBACK(NewState));
	bar.Add("New Transition", THISBACK(NewTransition));
}

void StateEditor::Load(const String& maestro_root, const String& id) {
	root = maestro_root;
	current_id = id;
	wfm.Create(root);
	
	String path = AppendFileName(AppendFileName(root, "docs/workflows"), id + ".puml");
	puml_editor.SetData(LoadFile(path));
	UpdatePreview();
}

void StateEditor::UpdatePreview() {
	String puml = puml_editor.GetData();
	Cout() << "Updating PUML Preview...\n";
}

void StateEditor::NewState() {
	puml_editor.Insert(puml_editor.GetCursor(), "state NewState {\n}\n");
}

void StateEditor::NewTransition() {
	puml_editor.Insert(puml_editor.GetCursor(), "[*] --> InitialState\n");
}

void StateEditor::Save() {
	String path = AppendFileName(AppendFileName(root, "docs/workflows"), current_id + ".puml");
	if(SaveFile(path, puml_editor.GetData())) {
		PromptOK("Workflow saved.");
	}
}

END_UPP_NAMESPACE