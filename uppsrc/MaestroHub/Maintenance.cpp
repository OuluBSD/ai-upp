#include "MaestroHub.h"

NAMESPACE_UPP

MaintenancePane::MaintenancePane() {
	CtrlLayout(*this);
	
	toolbar.MaxIconSize(Size(20, 20));
	toolbar.Set([=](Bar& bar) {
		bar.Add("Purge AI Cache", CtrlImg::remove(), THISBACK(OnPurgeCache));
		bar.Add("Purge Track Cache", CtrlImg::remove(), THISBACK(OnPurgeTracks));
		bar.Separator();
		bar.Add("Sync Core", CtrlImg::Plus(), THISBACK(OnSyncCore));
	});

	cache_list.AddColumn("Key");
	cache_list.AddColumn("Type");
	cache_list.AddColumn("Size");
	cache_list.WhenCursor = THISBACK(OnCacheCursor);

	split.Horz(cache_list, cache_detail);
}

void MaintenancePane::Load(const String& maestro_root) {
	root = maestro_root;
	
	cache_list.Clear();
	// Stub data for now
	cache_list.Add("gpt-4o:prompt_123...", "AI Response", "12 KB");
	cache_list.Add("track:conversion_task_4...", "Task State", "4 KB");
	cache_list.Add("meta:project_index", "Search Index", "2 MB");
}

void MaintenancePane::OnPurgeCache() {
	if(PromptYesNo("Purge all cached AI responses?")) {
		// Real implementation would delete files in .maestro/cache
		Load(root);
	}
}

void MaintenancePane::OnPurgeTracks() {
	if(PromptYesNo("Purge all persistent track state? This will reset progress.")) {
		Load(root);
	}
}

void MaintenancePane::OnSyncCore() {
	// Stub for synchronization logic
	PromptOK("Core sync finished successfully.");
}

void MaintenancePane::OnCacheCursor() {
	if(cache_list.IsCursor()) {
		String key = cache_list.Get(0);
		cache_detail.SetQTF("[&@6 [* Cache Entry Detail]]&[C1 Key: " + DeQtf(key) + "]&[C1 This view will show JSON or binary content of the cache entry.]");
	}
}

END_UPP_NAMESPACE
