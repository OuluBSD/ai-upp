#include "AuditTrail.h"

NAMESPACE_UPP

AuditTrailCorrelator::AuditTrailCorrelator() {
	CtrlLayout(*this);
	
event_list.AddColumn("Time", 100);
	event_list.AddColumn("Type", 100);
	event_list.AddColumn("Summary", 400);
	event_list.WhenCursor = THISBACK(OnEventCursor);
	
vsplit.Vert(event_list, detail_view);
	vsplit.SetPos(6000); // 60% list, 40% detail
}

void AuditTrailCorrelator::Load(const String& maestro_root) {
	event_list.Clear();
	
	// Correlate from LogManager
	LogManager lm(maestro_root);
	for(const auto& s : lm.ListScans())
		event_list.Add(s.timestamp, "Log Scan", "Scan findings count: " + IntStr(s.finding_count), StoreAsJson(s));
		
	// Correlate from WorkSessionManager
	for(const auto& ws : WorkSessionManager::ListSessions(maestro_root))
		event_list.Add(ws.created, "AI Session", "[" + ws.session_type + "] " + ws.purpose, StoreAsJson(ws));
		
	// Correlate from Git (Stub - would use RepoScanner or direct git calls)
	event_list.Add(GetSysTime(), "Git Commit", "Refactored Core/Renamer.cpp", "{\"commit\": \"abc1234\", \"author\": \"maestro-ai\"}");
	
	event_list.Sort(0); // Sort by time (column 0)
	if(event_list.GetCount() > 0) event_list.SetCursor(0);
}

void AuditTrailCorrelator::OnEventCursor() {
	detail_view.SetQTF("");
	if(!event_list.IsCursor()) return;
	
	String json = event_list.Get(3);
	detail_view.SetQTF("[C1 " + DeQtf(json) + "]");
}

END_UPP_NAMESPACE
