#include "MaestroHub.h"

NAMESPACE_UPP

EvidencePane::EvidencePane() {
	CtrlLayout(*this);
	
	toolbar.Set(THISBACK(OnToolbar));
	
	evidence_list.AddColumn("ID", 100);
	evidence_list.AddColumn("Time", 120);
	evidence_list.AddColumn("Items", 80);
	evidence_list.WhenCursor = THISBACK(OnEvidenceCursor);
	
	main_split.Vert(evidence_list, detail_view);
	main_split.SetPos(4000);
}

void EvidencePane::OnToolbar(Bar& bar) {
	bar.Add("Collect", CtrlImg::plus(), THISBACK(OnCollect)).Tip("Collect Current Context Evidence");
	bar.Separator();
	bar.Add("Export PDF...", []{ PromptOK("Exporting to PDF (Stub)"); });
}

void EvidencePane::Load(const String& maestro_root) {
	root = maestro_root;
	evidence_list.Clear();
	
	String evidence_dir = AppendFileName(root, ".maestro/evidence");
	FindFile ff(AppendFileName(evidence_dir, "*.json"));
	while(ff) {
		if(ff.IsFile()) {
			String json = LoadFile(ff.GetPath());
			EvidencePack ep;
			if(LoadFromJson(ep, json)) {
				evidence_list.Add(ep.meta.pack_id, ep.meta.created_at, ep.meta.evidence_count, json);
			}
		}
		ff.Next();
	}
	
	if(evidence_list.GetCount() > 0) evidence_list.SetCursor(0);
}

void EvidencePane::OnEvidenceCursor() {
	detail_view.SetQTF("");
	if(!evidence_list.IsCursor()) return;
	
	String json = evidence_list.Get(3);
	detail_view.SetQTF("[C1 " + DeQtf(json) + "]");
}

void EvidencePane::OnCollect() {
	if(root.IsEmpty()) return;
	
	EvidenceCollector ec(root);
	EvidencePack ep = ec.CollectAll();
	
	String evidence_dir = AppendFileName(root, ".maestro/evidence");
	RealizeDirectory(evidence_dir);
	
	String filename = "evidence_" + ep.meta.pack_id + ".json";
	String path = AppendFileName(evidence_dir, filename);
	
	if(SaveFile(path, StoreAsJson(ep))) {
		Load(root);
		PromptOK("Evidence collected successfully: " + filename);
	}
}

END_UPP_NAMESPACE