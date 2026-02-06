#include "MaestroHub.h"

NAMESPACE_UPP

PlaybookVisualLogic::PlaybookVisualLogic() {
	Add(split.SizePos());
	split.Horz(puml_editor, graph_view);
	split.SetPos(4000);
	
	puml_editor.WhenAction = [=] { UpdatePreview(); };
}

void PlaybookVisualLogic::Load(const String& puml) {
	puml_editor.SetData(puml);
	UpdatePreview();
}

void PlaybookVisualLogic::UpdatePreview() {
	// Future: Integration with Maestro PUML parser to update graph_view
}

String PlaybookVisualLogic::Get() const {
	return puml_editor.GetData();
}

PlaybookPane::PlaybookPane() {
	CtrlLayout(*this);
	
	toolbar.Set(THISBACK(OnToolbar));
	
	left_pane.Add(playbook_list.SizePos());
	playbook_list.AddColumn("ID", 80);
	playbook_list.AddColumn("Title", 200);
	playbook_list.WhenCursor = THISBACK(OnPlaybookCursor);
	
	general_pane.Add(playbook_id.TopPos(8, 19).LeftPos(96, 200));
	general_pane.Add(title.TopPos(32, 19).LeftPos(96, 400));
	general_pane.Add(intent.TopPos(80, 60).HSizePos(8, 8));
	general_pane.Add(principles.VSizePos(168, 8).HSizePos(8, 8));
	
	principles.AddColumn("Principle / Rule");
	
	detail_tabs.Add(general_pane.SizePos(), "General Info");
	detail_tabs.Add(visual_logic.SizePos(), "Visual Logic");
	detail_tabs.Add("Glossary & Constraints (Stub)");
	
	split.Horz(left_pane, detail_tabs);
	split.SetPos(2500);
}

void PlaybookPane::OnToolbar(Bar& bar) {
	bar.Add("New", CtrlImg::plus(), THISBACK(OnNew)).Tip("Create New Strategy");
	bar.Add("Save", CtrlImg::save(), THISBACK(OnSave)).Tip("Save Current Strategy");
	bar.Separator();
	bar.Add("Validate", CtrlImg::reporticon(), THISBACK(OnValidate)).Tip("Validate Strategy Integrity");
}

void PlaybookPane::Load(const String& maestro_root) {
	root = maestro_root;
	PlaybookManager pm(root);
	playbooks = pick(pm.ListPlaybooks());
	
	playbook_list.Clear();
	for(const auto& pb : playbooks)
		playbook_list.Add(pb.id, pb.title);
		
	if(playbook_list.GetCount() > 0) playbook_list.SetCursor(0);
}

void PlaybookPane::OnPlaybookCursor() {
	if(!playbook_list.IsCursor()) return;
	
	const Playbook& pb = playbooks[playbook_list.GetCursor()];
	playbook_id.SetData(pb.id);
	title.SetData(pb.title);
	intent.SetData(pb.intent);
	
	principles.Clear();
	for(const auto& p : pb.principles)
		principles.Add(p);
		
	// Load PUML logic file if it exists
	String puml_path = AppendFileName(AppendFileName(root, "docs/playbooks"), pb.id + ".puml");
	if(FileExists(puml_path))
		visual_logic.Load(LoadFile(puml_path));
	else
		visual_logic.Load("@startuml\n\n@enduml");
}

void PlaybookPane::OnNew() {
	playbook_id.SetData("new_strategy");
	title.SetData("New Expert Strategy");
	intent.SetData("");
	principles.Clear();
}

void PlaybookPane::OnSave() {
	if(root.IsEmpty()) return;
	
	Playbook pb;
	pb.id = playbook_id.GetData();
	pb.title = title.GetData();
	pb.intent = intent.GetData();
	
	for(int i = 0; i < principles.GetCount(); i++)
		pb.principles.Add(principles.Get(i, 0));
		
	PlaybookManager pm(root);
	if(pm.SavePlaybook(pb)) {
		PromptOK("Playbook saved.");
		Load(root);
	}
}

void PlaybookPane::OnValidate() {
	if(!playbook_list.IsCursor()) return;
	
	const Playbook& pb = playbooks[playbook_list.GetCursor()];
	if(PlaybookManager::Validate(pb))
		PromptOK("Playbook validation successful.");
	else
		Exclamation("Playbook validation failed: Missing required metadata.");
}

PlaybookSelectDialog::PlaybookSelectDialog() {
	CtrlLayoutOKCancel(*this, "Select Expert Strategy");
	Sizeable().Zoomable();
	
	list.AddColumn("ID", 80);
	list.AddColumn("Title", 200);
}

void PlaybookSelectDialog::Load(const String& root) {
	PlaybookManager pm(root);
	Array<Playbook> playbooks = pick(pm.ListPlaybooks());
	
	list.Clear();
	for(const auto& pb : playbooks)
		list.Add(pb.id, pb.title);
		
	if(list.GetCount() > 0) list.SetCursor(0);
}

END_UPP_NAMESPACE
