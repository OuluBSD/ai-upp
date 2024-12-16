#include "Ctrl.h"

NAMESPACE_UPP


NotepadCtrl::NotepadCtrl() {
	CtrlLayout(idea);
	
	Add(hsplit.SizePos());
	hsplit.Horz();
	hsplit << list << idea;
	hsplit.SetPos(2000);
	
	list.AddColumn("Title");
	//list.NoHeader();
	list.WhenBar << THISBACK(OnListMenu);
	list.WhenCursor << THISBACK(IdeaData);
	
	idea.title.WhenAction << THISBACK(OnValueChange);
	idea.target.WhenAction << THISBACK(OnValueChange);
	idea.ref.WhenAction << THISBACK(OnValueChange);
	idea.desc.WhenAction << THISBACK(OnValueChange);
}

void NotepadCtrl::Clear() {
	idea.title.Clear();
	idea.target.Clear();
	idea.ref.Clear();
	idea.desc.Clear();
}

void NotepadCtrl::Data() {
	DatasetPtrs p = GetDataset();
	if (!p.release) {
		Clear();
		return;
	}
	Release& release = *p.release;
	
	for(int i = 0; i < release.ideas.GetCount(); i++) {
		const ComponentIdea& idea = release.ideas[i];
		list.Set(i, 0, idea.title);
	}
	list.SetCount(release.ideas.GetCount());
	
	int idea_idx = 0;
	if (idea_idx >= 0 && idea_idx < list.GetCount())
		list.SetCursor(idea_idx);
	
	IdeaData();
}

void NotepadCtrl::IdeaData() {
	DatasetPtrs p = GetDataset();
	if (!p.release || !list.IsCursor()) {
		Clear();
		return;
	}
	Release& release = *p.release;
	int idea_idx = list.GetCursor();
	const ComponentIdea& obj = release.ideas[idea_idx];
	
	this->idea.title.SetData(obj.title);
	this->idea.target.SetData(obj.target_song);
	this->idea.ref.SetData(obj.reference_song);
	this->idea.desc.SetData(obj.description);
}

void NotepadCtrl::OnValueChange() {
	DatasetPtrs p = GetDataset();
	if (!p.release) return;
	Release& release = *p.release;
	
	if (!list.IsCursor()) return;
	int idea_idx = list.GetCursor();
	ComponentIdea& obj = release.ideas[idea_idx];
	
	obj.title = idea.title.GetData();
	obj.target_song = idea.target.GetData();
	obj.reference_song = idea.ref.GetData();
	obj.description = idea.desc.GetData();
	
	list.Set(0, obj.title);
}

void NotepadCtrl::OnListMenu(Bar& bar) {
	bar.Add(t_("Add Idea"), THISBACK(AddIdea));
	bar.Add(t_("Remove Idea"), THISBACK(RemoveIdea));
}

void NotepadCtrl::AddIdea() {
	DatasetPtrs p = GetDataset();
	if (!p.release) return;
	Release& release = *p.release;
	
	
	String title;
	bool b = EditTextNotNull(
		title,
		t_("Add Idea"),
		t_("Idea's title"),
		0
	);
	if (!b) return;
	
	int idea_i = -1;
	for(int i = 0; i < release.ideas.GetCount(); i++) {
		ComponentIdea& idea = release.ideas[i];
		if (idea.title == title) {
			idea_i = i;
			break;
		}
	}
	if (idea_i >= 0) {
		PromptOK(DeQtf(t_("Idea with that title exist already")));
		return;
	}
	
	ComponentIdea& idea = release.ideas.Add();
	idea.title = title;
	
	list.Add(title);
	list.SetCursor(list.GetCount()-1);
	this->idea.target.SetFocus();
}

void NotepadCtrl::RemoveIdea() {
	DatasetPtrs p = GetDataset();
	if (!p.release) return;
	Release& release = *p.release;
	
	if (!list.IsCursor()) return;
	int idx = list.GetCursor();
	release.ideas.Remove(idx);
	Data();
}


INITIALIZER_COMPONENT_CTRL(Notepad, NotepadCtrl)

END_UPP_NAMESPACE
