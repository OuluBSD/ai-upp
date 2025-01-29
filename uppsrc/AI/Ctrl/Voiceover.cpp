#include "Ctrl.h"

NAMESPACE_UPP

VoiceoverTextCtrl::InputTab::InputTab(VoiceoverTextCtrl& o) : owner(o) {
	CtrlLayout(*this);
	this->scene.WhenAction = [this](){this->owner.Set("scene", this->scene.GetData());};
	this->people.WhenAction = [this](){this->owner.Set("people", this->people.GetData());};
	this->transcription.WhenAction = [this](){this->owner.Set("transcription", this->transcription.GetData());};
	this->lang.WhenAction = [this](){this->owner.Set("language", this->lang.GetData());};
}

void VoiceoverTextCtrl::InputTab::Data() {
	this->scene.SetData(this->owner.Get("scene"));
	this->people.SetData(this->owner.Get("people"));
	this->transcription.SetData(this->owner.Get("transcription"));
	this->lang.SetData(this->owner.Get("language"));
}

VoiceoverTextCtrl::PartTab::PartTab(VoiceoverTextCtrl& o) : owner(o) {
	
}

void VoiceoverTextCtrl::PartTab::Data() {
	
}

VoiceoverTextCtrl::GenerateTab::GenerateTab(VoiceoverTextCtrl& o) : owner(o) {
	Add(vsplit.SizePos());
	vsplit.Vert() << params << output;
	params.AddColumn("Key");
	params.AddColumn("Value");
}

void VoiceoverTextCtrl::GenerateTab::Data() {
	
}

VoiceoverTextCtrl::VoiceoverTextCtrl() {
	
	
}

void VoiceoverTextCtrl::ToolMenu(Bar& bar) {
	RefreshParams();
	if (!active_process)
		bar.Add("Start process", [this]{active_process = &VoiceoverProcess::Get(this->GetNode().GetPath(), params); active_process->Start();}).Key(K_F5);
	else
		bar.Add("Stop process", [this]{active_process->Stop();}).Key(K_F5);
}

void VoiceoverTextCtrl::RefreshParams() {
	ValueMap map;
	map.Add("scene", Get("scene"));
	map.Add("people", Get("people"));
	map.Add("transcription", Get("transcription"));
	map.Add("language", Get("language"));
	params = map;
}

void VoiceoverTextCtrl::RealizeData() {
	VirtualNode root = this->Root();
	int kind = root.GetKind();
	if (!root.GetKind()) {
		root.SetKind(METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER);
	}
	ASSERT(root.GetKind() == METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER);
	
	if (1) {
		root.Add("test 1", METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER_PART);
		root.Add("test 2", METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER_PART);
		root.Add("test 3", METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER_PART);
	}
}

void VoiceoverTextCtrl::Init() {
	RealizeData();
}

String VoiceoverTextCtrl::GetTitle() const {
	return "Content";
}

VNodeComponentCtrl* VoiceoverTextCtrl::CreateCtrl(const VirtualNode& vnode) {
	int kind = vnode.GetKind();
	if (kind == METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER)
		return new InputTab(*this);
	else if (kind == METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER_PART)
		return new PartTab(*this);
	else if (kind == METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER_GENERATE)
		return new GenerateTab(*this);
	return 0;
}


INITIALIZER_COMPONENT_CTRL(VoiceoverText, VoiceoverTextCtrl)

END_UPP_NAMESPACE
