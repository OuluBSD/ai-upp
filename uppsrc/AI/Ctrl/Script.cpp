#include "Ctrl.h"

NAMESPACE_UPP

ScriptTextCtrl::InputTab::InputTab(ScriptTextCtrl& o) : owner(o) {
	CtrlLayout(*this);
	
}

void ScriptTextCtrl::InputTab::Data() {
	
}

ScriptTextCtrl::PartTab::PartTab(ScriptTextCtrl& o) : owner(o) {
	
}

void ScriptTextCtrl::PartTab::Data() {
	
}

ScriptTextCtrl::GenerateTab::GenerateTab(ScriptTextCtrl& o) : owner(o) {
	Add(vsplit.SizePos());
	vsplit.Vert() << params << output;
	params.AddColumn("Key");
	params.AddColumn("Value");
}

void ScriptTextCtrl::GenerateTab::Data() {
	
}



ScriptTextCtrl::ScriptTextCtrl() {
	
}

void ScriptTextCtrl::DataTree(TreeCtrl& tree) {
	VirtualFSComponentCtrl::DataTree(tree);
	tree.WhenBar = [this,&tree](Bar& b) {
		int cur = tree.GetCursor();
		if (cur == 0) {
			b.Add("Add part", THISBACK(AddPart));
		}
		else {
			b.Add("Remove part", THISBACK(RemovePart));
		}
	};
}

void ScriptTextCtrl::ToolMenu(Bar& bar) {
	RefreshParams();
	if (!active_process)
		bar.Add("Start process", [this]{active_process = &ScriptTextProcess::Get(this->GetNode().GetPath(), params); active_process->Start();}).Key(K_F5);
	else
		bar.Add("Stop process", [this]{active_process->Stop();}).Key(K_F5);
}

void ScriptTextCtrl::AddPart() {
	RealizeData();
	EntityEditorCtrl* ee = dynamic_cast<EntityEditorCtrl*>(&*owner);
	if (!ee) {
		PromptOK("Unexpected context");
		return;
	}
	MetaNode* n = ee->SelectTreeNode("Select node to be used as part");
	if (n) {
		TranscriptProofread* proofread = 0;
		if (n->kind == METAKIND_ECS_ENTITY) {
			proofread = n->Find<TranscriptProofread>();
		}
		else if (n->kind == METAKIND_ECS_COMPONENT_TRANSCRIPT_PROOFREAD) {
			proofread = &n->GetExt<TranscriptProofread>();
			n = n->owner;
		}
		if (proofread) {
			VirtualNode root = this->Root();
			String id = n->id;
			if (id.IsEmpty()) id = "part";
			VirtualNode new_node = root.Add(id, METAKIND_ECS_VIRTUAL_IO_SCRIPT_PART_PROOFREAD);
			ImportProofread(new_node, *proofread);
			WhenEditorChange();
		}
	}
}

void ScriptTextCtrl::ImportProofread(VirtualNode new_node, TranscriptProofread& proofread) {
	ValueArray selected = proofread.value("selected");
	String text = proofread.value("proofread");
	TranscriptResponse r;
	LoadFromJson(r,text);
	new_node.RemoveSubNodes();
	VirtualNode section = new_node.Add("0", METAKIND_ECS_VIRTUAL_IO_SCRIPT_PART_SUB);
	for(int i = 0; i < selected.GetCount(); i++) {
		int seg_i = selected[i];
		if (seg_i >= 0 && seg_i < r.segments.GetCount()) {
			const auto& seg = r.segments[i];
			section.Add(IntStr(i), METAKIND_ECS_VIRTUAL_IO_SCRIPT_PART_LINE);
		}
	}
}

void ScriptTextCtrl::RemovePart() {
	VirtualNode root = this->Root();
	VfsPath path = GetCursorRelativePath();
	if (path.IsEmpty())
		return;
	String name = path.TopPart();
	path.RemoveLast();
	VirtualNode sub = this->Find(path);
	if (sub) {
		sub.Remove(name);
		WhenEditorChange();
	}
}

void ScriptTextCtrl::RefreshParams() {
	ValueMap map;
	
	
	
	params = map;
}

void ScriptTextCtrl::RealizeData() {
	VirtualNode root = this->Root();
	int kind = root.GetKind();
	if (!root.GetKind()) {
		root.SetKind(METAKIND_ECS_VIRTUAL_IO_SCRIPT);
	}
	ASSERT(root.GetKind() == METAKIND_ECS_VIRTUAL_IO_SCRIPT);
}

void ScriptTextCtrl::Init() {
	RealizeData();
}

String ScriptTextCtrl::GetTitle() const {
	return "Content";
}

VNodeComponentCtrl* ScriptTextCtrl::CreateCtrl(const VirtualNode& vnode) {
	int kind = vnode.GetKind();
	if (kind == METAKIND_ECS_VIRTUAL_IO_SCRIPT)
		return new InputTab(*this);
	else if (kind == METAKIND_ECS_VIRTUAL_IO_SCRIPT_PART_PROOFREAD)
		return new PartTab(*this);
	//else if (kind == METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER_GENERATE)
	//	return new GenerateTab(*this);
	return 0;
}


INITIALIZER_COMPONENT_CTRL(ScriptText, ScriptTextCtrl)

END_UPP_NAMESPACE
