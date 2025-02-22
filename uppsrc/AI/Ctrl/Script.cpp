#include "Ctrl.h"

NAMESPACE_UPP



ScriptTextCtrl::SubTab::SubTab(ScriptTextCtrl& o, const VirtualNode& vnode) : owner(o), VNodeComponentCtrl(o, vnode) {
	Add(tabs.SizePos());
	dbproc.WhenView = THISBACK(PageView);
}

void ScriptTextCtrl::SubTab::PageView(int page) {
	int i = 0;
	#define CREATE(x, txt) if (i++ == page) x->Data();
	CREATE(srcdata, "Source data")
	CREATE(tk, "Analyzed")
	CREATE(awp, "Tokens")
	CREATE(vp, "Ambiguous Word Pairs")
	CREATE(vpp, "Virtual Phrases")
	CREATE(vps, "Virtual Phrase Structs")
	CREATE(vpa, "Phrase Part Analysis 1")
	CREATE(vpa2, "Phrase Part Analysis 2")
	CREATE(aap, "Action Attrs Page")
	CREATE(att, "Attributes")
	CREATE(diag, "Text Data Diagnostics")
	#undef CREATE
}

void ScriptTextCtrl::SubTab::Data() {
	Vector<int> pages = dbproc.GetPagesOnSight();
	for (int page : pages)
		PageView(page);
}

void ScriptTextCtrl::SubTab::EditPos(JsonIO& json) {
	int tab_i = tabs.Get();
	json("tab", tab_i);
	if (json.IsLoading() && tab_i >= 0 && tab_i < tabs.GetCount())
		tabs.Set(tab_i);
}

void ScriptTextCtrl::SubTab::AddRootTabs() {
	part_view.Create();
	editor.Create();
	tabs.Add(part_view->SizePos(), "Part view");
	tabs.Add(editor->SizePos(), "Editor");
}


void ScriptTextCtrl::SubTab::AddLineOwnerTabs() {
	RealizeEntityVfsObject(vnode, METAKIND_ECS_VIRTUAL_VALUE_SRCTEXT);
	
	tabs.Add(dbproc.SizePos(), "Line process");
	#define CREATE(x, txt, height) \
		{x.Create(*this); \
		auto& page = dbproc.Add(*x, txt); \
		if (height > 0) page.Height(height); }
	CREATE(srcdata, "Source data", 200)
	CREATE(tk, "Analyzed", 0)
	CREATE(awp, "Tokens", 0)
	CREATE(vp, "Ambiguous Word Pairs", 0)
	CREATE(vpp, "Virtual Phrases", 0)
	CREATE(vps, "Virtual Phrase Structs", 0)
	CREATE(vpa, "Phrase Part Analysis 1", 0)
	CREATE(vpa2, "Phrase Part Analysis 2", 0)
	CREATE(aap, "Action Attrs Page", 0)
	CREATE(att, "Attributes", 0)
	CREATE(diag, "Text Data Diagnostics", 0)
	#undef CREATE
	
	srcdata->SetMixed();
}


ScriptTextCtrl::LineTab::LineTab(ScriptTextCtrl& o, const VirtualNode& vnode) : owner(o), db(o), VNodeComponentCtrl(o, vnode) {
	Add(tabs.SizePos());
	tabs.Add(db.SizePos(), "DB");
}

void ScriptTextCtrl::LineTab::Data() {
	
}



ScriptTextCtrl::PartTab::PartTab(ScriptTextCtrl& o, const VirtualNode& vnode) : owner(o), VNodeComponentCtrl(o, vnode) {
	
}

void ScriptTextCtrl::PartTab::Data() {
	
}



ScriptTextCtrl::GenerateTab::GenerateTab(ScriptTextCtrl& o, const VirtualNode& vnode) : owner(o), VNodeComponentCtrl(o, vnode) {
	Add(vsplit.SizePos());
	vsplit.Vert() << params << output;
	params.AddColumn("Key");
	params.AddColumn("Value");
}

void ScriptTextCtrl::GenerateTab::Data() {
	
}



ScriptTextCtrl::ScriptTextCtrl() {
	
}

bool ScriptTextCtrl::TreeItemString(const VirtualNode& n, const Value& key, String& qtf_value) {
	if (n.IsValue()) {
		ValueMap value = n.GetValue();
		String text = value("text");
		if (text.GetCount()) {
			qtf_value =
				"[$(28.255.150)1 Color][1  ][$(255.220.200)1 Element][1  ][$(154.213.147)1 Attr Group][1  ][$(110.220.98)1 Attr Value][1  ][$(238.162.211)1 Action Group][1  ][$(238.172.230)1 Action Value&][ " +
				DeQtf(text);
			return true;
		}
	}
	return false;
}

void ScriptTextCtrl::EditPos(JsonIO& json) {
	json("process_automatically", process_automatically);
	VirtualFSComponentCtrl::EditPos(json);
	
	if (json.IsLoading() && process_automatically)
		PostCallback(THISBACK1(StartProcess, false));
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
	
	bar.Add("Refresh", [this]{
		this->Data();
	}).Key(K_CTRL_Q);
	bar.Separator();
	bar.Add("Process automatically", [this]{process_automatically = !process_automatically;}).Check(process_automatically);
	if (!active_process)
		bar.Add("Start process", THISBACK1(StartProcess, true)).Key(K_F5);
	else
		bar.Add("Stop process", THISBACK(StopProcess)).Key(K_F5);
}

void ScriptTextCtrl::StartProcess(bool user_started) {
	auto* c = GetVNodeComponentCtrl();
	if (!c) return;
	RefreshParams();
	DatasetPtrs p = c->GetDataset();
	if (!p.src) {
		if (user_started) {
			PromptOK("Database context is needed. Set one in entity");
		}
		PostOnStop();
		return;
	}
	if (p.srctxt) {
		active_process = &ScriptTextProcess::Get(
			p,
			this->GetNode().GetPath(),
			params,
			*p.srctxt,
			THISBACK(PostOnStop));
		active_process->WhenError << [this](String err) {PostOnStop();};
		active_process->Start();
	}
}

void ScriptTextCtrl::StopProcess() {
	active_process->Stop();
}

void ScriptTextCtrl::PostOnStop() {
	Ptr<ScriptTextCtrl> p = this;
	PostCallback([p]{
		if (p)
			p->OnStop();
	});
}

void ScriptTextCtrl::OnStop() {
	active_process = nullptr;
	this->Data();
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
	ValueMap selected = proofread.value("selected");
	Index<int> selected_indices;
	for(int i = 0; i < selected.GetCount(); i++)
		selected_indices.FindAdd(selected.GetKey(i));
	SortIndex(selected_indices, StdLess<int>());
	String text = proofread.value("proofread");
	TranscriptResponse r;
	LoadFromJson(r,text);
	new_node.RemoveSubNodes();
	VirtualNode section = new_node.Add("0", METAKIND_ECS_VIRTUAL_IO_SCRIPT_PART_SUB);
	for(int i = 0; i < selected_indices.GetCount(); i++) {
		int seg_i = selected_indices[i];
		if (seg_i >= 0 && seg_i < r.segments.GetCount()) {
			const auto& seg = r.segments[seg_i];
			VirtualNode line = section.Add(i, METAKIND_ECS_VIRTUAL_IO_SCRIPT_PART_LINE);
			ASSERT(line.IsValue());
			ValueMap lineval = line.GetValue();
			lineval("text", seg.text);
			lineval("begin", seg.start);
			lineval("end", seg.end);
			line.WriteValue(lineval);
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
	params = ValueMap();
	
	auto* c = GetVNodeComponentCtrl();
	if (!c)
		return;
	VirtualNode vnode = c->GetVnode();
	
	ValueArray input_text;
	ASSERT(vnode.IsValue());
	auto sub = vnode.FindAll(METAKIND_ECS_VIRTUAL_IO_SCRIPT_PART_LINE);
	for(int i = 0; i < sub.GetCount(); i++) {
		VirtualNode s = sub[i];
		ASSERT(s.IsValue());
		Value val = s.GetValue();
		Value text = val("text");
		input_text.Add(text);
	}
	params("input_text") = input_text;
	params("genres") = ValueArray();
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
	if (kind == METAKIND_ECS_VIRTUAL_IO_SCRIPT) {
		SubTab* o = new SubTab(*this, vnode);
		o->AddRootTabs();
		o->AddLineOwnerTabs();
		return o;
	}
	else if (kind == METAKIND_ECS_VIRTUAL_IO_SCRIPT_PART_SUB) {
		SubTab* o = new SubTab(*this, vnode);
		o->AddLineOwnerTabs();
		return o;
	}
	else if (kind == METAKIND_ECS_VIRTUAL_IO_SCRIPT_PART_PROOFREAD ||
			 kind == METAKIND_ECS_VIRTUAL_IO_SCRIPT_PART_LINE)
		return new LineTab(*this, vnode);
	//else if (kind == METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER_GENERATE)
	//	return new GenerateTab(*this);
	return 0;
}


INITIALIZER_COMPONENT_CTRL(ScriptText, ScriptTextCtrl)



END_UPP_NAMESPACE
