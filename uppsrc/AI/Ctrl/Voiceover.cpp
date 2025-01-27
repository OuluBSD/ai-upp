#include "Ctrl.h"

NAMESPACE_UPP

VoiceoverTextCtrl::AnalyzeTab::AnalyzeTab() {
	Add(vsplit.SizePos());
	vsplit.Vert() << input << hsplit;
	hsplit.Horz() << cat << values;
	hsplit.SetPos(3333);
	cat.AddColumn("Category");
	values.AddColumn("Key");
	values.AddColumn("Value");
	values.ColumnWidths("1 2");
}

VoiceoverTextCtrl::GenerateTab::GenerateTab() {
	Add(vsplit.SizePos());
	vsplit.Vert() << params << output;
	params.AddColumn("Key");
	params.AddColumn("Value");
}

VoiceoverTextCtrl::VoiceoverTextCtrl() {
	
	
}

void VoiceoverTextCtrl::ToolMenu(Bar& bar) {
	
}

void VoiceoverTextCtrl::RealizeData() {
	VirtualNode& root = this->Root();
	if (!root.GetKind()) {
		root.SetKind(METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER);
	}
}

void VoiceoverTextCtrl::Init() {
	RealizeData();
}

void VoiceoverTextCtrl::VirtualData() {
	
}

String VoiceoverTextCtrl::GetTitle() const {
	return "Input/Output";
}


INITIALIZER_COMPONENT_CTRL(VoiceoverText, VoiceoverTextCtrl)

END_UPP_NAMESPACE
