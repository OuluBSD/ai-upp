#include "MaestroHub.h"

NAMESPACE_UPP

LogAnalyzer::LogAnalyzer() {
	CtrlLayout(*this);
	
	scan_list.AddColumn("Time");
	scan_list.AddColumn("Source");
	scan_list.AddColumn("Count");
	scan_list.WhenCursor = THISBACK(OnScanCursor);
	
	finding_list.AddColumn("Kind");
	finding_list.AddColumn("File");
	finding_list.AddColumn("Message");
	finding_list.WhenCursor = THISBACK(OnFindingCursor);
	
	finding_list.WhenBar = [=](Bar& bar) {
		if(finding_list.IsCursor()) {
			bar.Add("Create Issue", THISBACK(OnCreateIssue));
			
			ValueMap f;
			if(LoadFromJson(f, (String)finding_list.Get(3))) {
				if(sm.Match(f["message"]) >= 0)
					bar.Add("AI Remediate...", THISBACK(OnRemediate));
			}
			
			bar.Separator();
			bar.Add("Manage Solutions...", [=] {
				SolutionsHub dlg;
				dlg.Load(root);
				dlg.Run();
				sm.Load(root);
			});
		}
	};
	
	hsplit.Horz(finding_list, detail_view);
	vsplit.Vert(scan_list, hsplit);
	
	tail.WhenLine = THISBACK(OnTailLine);
}

void LogAnalyzer::Load(const Upp::String& maestro_root) {
	root = maestro_root;
	lm.Create(root);
	sm.Load(root);
	UpdateScans();
	
	Upp::String test_log = AppendFileName(root, "test.log");
	if(Upp::FileExists(test_log)) {
		tail.Open(test_log);
		SetTimeCallback(-500, [=] { tail.Poll(); }, 0);
	}
}

void LogAnalyzer::OnTailLine(Upp::String line) {
	if(WhenLog) WhenLog("LOG: " + line);
	
	Upp::Array<LogFinding> findings = pick(lm->ExtractFindings(line, "any"));
	for(const auto& f : findings) {
		finding_list.Insert(0);
		finding_list.Set(0, 0, f.kind);
		finding_list.Set(0, 1, f.file);
		finding_list.Set(0, 2, f.message);
		finding_list.Set(0, 3, StoreAsJson(f));
		if(WhenLog) WhenLog("LIVE ISSUE DETECTED: " + f.message);
	}
}

void LogAnalyzer::UpdateScans() {
	scan_list.Clear();
	Array<LogScanMeta> scans = pick(lm->ListScans());
	for(const auto& s : scans)
		scan_list.Add(s.timestamp, s.source_path, s.finding_count, s.scan_id);
	
	if(scan_list.GetCount() > 0) scan_list.SetCursor(0);
}

void LogAnalyzer::OnScanCursor() {
	finding_list.Clear();
	detail_view.SetQTF("");
	
	if(!scan_list.IsCursor()) return;
	String scan_id = scan_list.Get(3);
	
	LogScan s = lm->LoadScan(scan_id);
	for(const auto& f : s.findings)
		finding_list.Add(f.kind, f.file, f.message, StoreAsJson(f));
		
	if(finding_list.GetCount() > 0) finding_list.SetCursor(0);
}

void LogAnalyzer::OnFindingCursor() {
	detail_view.SetQTF("");
	if(!finding_list.IsCursor()) return;
	
	ValueMap f;
	if(LoadFromJson(f, (String)finding_list.Get(3))) {
		String qtf;
		qtf << "[*@3 " << f["kind"] << ": " << DeQtf((String)f["message"]) << "]&";
		qtf << "[* File:] " << DeQtf((String)f["file"]) << " (line " << f["line"] << ")&";
		qtf << "[* Tool:] " << DeQtf((String)f["tool"]) << "&";
		
		int idx = sm.Match(f["message"]);
		if(idx >= 0) {
			qtf << "&[= [* Found Solution Pattern: " << DeQtf(sm.patterns[idx].name) << "]]&";
		}
		
		qtf << "------------------------------------------&";
		qtf << DeQtf((String)f["raw_line"]);
		detail_view.SetQTF(qtf);
	}
}

void LogAnalyzer::OnCreateIssue() {
	if(!finding_list.IsCursor()) return;
	
	ValueMap f;
	if(LoadFromJson(f, (String)finding_list.Get(3))) {
		String scan_id = scan_list.Get(3);
		IssueManager ism(root);
		String issue_id = ism.CreateFromLogFinding(f, scan_id);
		if(!issue_id.IsEmpty())
			PromptOK("Issue " + issue_id + " created successfully.");
	}
}

void LogAnalyzer::OnRemediate() {
	if(!finding_list.IsCursor()) return;
	ValueMap f;
	if(LoadFromJson(f, (String)finding_list.Get(3))) {
		int idx = sm.Match(f["message"]);
		if(idx >= 0) {
			const auto& p = sm.patterns[idx];
			String prompt = p.prompt_template;
			prompt.Replace("%FILEPATH%", f["file"]);
			prompt.Replace("%FILELINE%", AsString(f["line"]));
			prompt.Replace("%ERRORMESSAGE%", f["message"]);
			
			if(WhenRemediate) WhenRemediate(prompt);
		}
	}
}

END_UPP_NAMESPACE
