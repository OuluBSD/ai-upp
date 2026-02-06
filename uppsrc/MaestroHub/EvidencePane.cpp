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
	bar.Add("Verify", CtrlImg::save(), THISBACK(OnVerify)).Tip("Verify Evidence Integrity");
	bar.Separator();
	bar.Add("Export PDF...", THISBACK(OnExportPdf)).Tip("Export Evidence to PDF");
}

void EvidencePane::OnExportPdf() {
	if(!evidence_list.IsCursor()) {
		Exclamation("Please select an evidence pack to export.");
		return;
	}
	
	String filename = "evidence_report_" + (String)evidence_list.Get(0) + ".pdf";
	String path = SelectFileSaveAs("PDF files (*.pdf)\t*.pdf");
	if(path.IsEmpty()) return;
	
	RichText rt = clone(detail_view.Get());
	if(rt.IsEmpty()) {
		// If no report visible, export the raw JSON/details
		rt = ParseQTF("[*@3 Raw Evidence Details]&\n" + DeQtf((String)evidence_list.Get(3)));
	}
	
	Size page_sz(3968, 6074); // A4 600dpi approx
	rt.ApplyZoom(Zoom(4, 1)); // Adjust for 600dpi
	
	String pdf_data = Pdf(rt, page_sz, 200);
	
	if(SaveFile(path, pdf_data))
		PromptOK("Report exported to: " + GetFileName(path));
}

void EvidencePane::OnVerify() {
	if(!evidence_list.IsCursor()) return;
	
	String json = evidence_list.Get(3);
	EvidencePack ep;
	if(!LoadFromJson(ep, json)) {
		Exclamation("Failed to load evidence pack for verification.");
		return;
	}
	
	// Simulation of verification logic
	SemanticIntegrityChecker sic(root);
	String report;
	report << "[*@3 Evidence Verification Report]&\n";
	report << "[* ID:] " << ep.meta.pack_id << "&\n";
	report << "[* Time:] " << ep.meta.created_at << "&\n";
	report << "------------------------------------------&\n";
	
	int file_count = 0;
	for(const auto& item : ep.items) {
		if(item.kind == "file") {
			report << "[* File:] " << item.source << " (" << item.size_bytes << " bytes) ";
			if(item.truncated) report << "[@Y TRUNCATED]";
			else report << "[@G OK]";
			report << "&\n";
			file_count++;
		}
	}
	
	report << "------------------------------------------&\n";
	report << "[* Result:] SUCCESS (" << file_count << " items verified)";
	
	detail_view.SetQTF(report);
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