#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

static bool Fail(const char* label)
{
	Cout() << "FAIL: " << label << "\n";
	SetExitCode(1);
	return false;
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	Cout() << "=== VisualStateModel Session Diff Demo ===\n\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// --- Step 1: Create 2 synthetic sessions ---
	String session_a_dir = AppendFileName(GetTempPath(), "vsm_session_diff_a");
	String session_b_dir = AppendFileName(GetTempPath(), "vsm_session_diff_b");

	if(DirectoryExists(session_a_dir)) DeleteFolderDeep(session_a_dir);
	if(DirectoryExists(session_b_dir)) DeleteFolderDeep(session_b_dir);

	Cout() << "Creating session A: " << session_a_dir << "\n";
	{
		VsmSessionStore store;
		store.SetLog(&log);
		if(!store.Create(session_a_dir, "session-diff-a", 48, 48))
			{ Fail("Cannot create session A"); return; }
		store.SaveFrameImage(0, VsmImageBuffer::MakeSolid(48, 48, 100, 1));
		store.SaveManifest();
	}

	Cout() << "Creating session B: " << session_b_dir << "\n";
	{
		VsmSessionStore store;
		store.SetLog(&log);
		if(!store.Create(session_b_dir, "session-diff-b", 48, 48))
			{ Fail("Cannot create session B"); return; }
		store.SaveFrameImage(0, VsmImageBuffer::MakeSolid(48, 48, 150, 1));
		store.SaveManifest();
	}

	// --- Step 2: Seed with divergences ---
	Cout() << "\nSeeding divergences.json files...\n";

	// Session A: one shared divergence + one unique to A
	Vector<VsmDivergence> divs_a;
	{
		VsmDivergence& d1 = divs_a.Add();
		d1.frame = 0;
		d1.ts = "2026-07-03T12:00:00Z";
		d1.severity = "warning";
		d1.message = "Shared divergence between sessions";
		d1.region_id = "region-shared";

		VsmDivergence& d2 = divs_a.Add();
		d2.frame = 0;
		d2.ts = "2026-07-03T12:00:01Z";
		d2.severity = "error";
		d2.message = "Unique divergence only in session A";
		d2.region_id = "region-only-a";
	}

	String div_path_a = AppendFileName(session_a_dir, "divergences.json");
	if(!SaveFile(div_path_a, StoreAsJson(divs_a, true)))
		{ Fail("Cannot save divergences.json for session A"); return; }
	Cout() << "  Saved " << divs_a.GetCount() << " divergences to session A\n";

	// Session B: one shared divergence + one unique to B
	Vector<VsmDivergence> divs_b;
	{
		VsmDivergence& d1 = divs_b.Add();
		d1.frame = 0;
		d1.ts = "2026-07-03T12:00:02Z";
		d1.severity = "warning";
		d1.message = "Shared divergence between sessions";
		d1.region_id = "region-shared-2";

		VsmDivergence& d2 = divs_b.Add();
		d2.frame = 1;
		d2.ts = "2026-07-03T12:00:03Z";
		d2.severity = "warning";
		d2.message = "Unique divergence only in session B";
		d2.region_id = "region-only-b";
	}

	String div_path_b = AppendFileName(session_b_dir, "divergences.json");
	if(!SaveFile(div_path_b, StoreAsJson(divs_b, true)))
		{ Fail("Cannot save divergences.json for session B"); return; }
	Cout() << "  Saved " << divs_b.GetCount() << " divergences to session B\n";

	// --- Step 3: Run the session diff ---
	Cout() << "\n--- Running Session Diff ---\n";

	VsmSessionDiff session_diff;
	session_diff.SetLog(&log);
	VsmSessionDiffResult result = session_diff.Compare(session_a_dir, session_b_dir);

	// --- Step 4: Print the result ---
	Cout() << "\n--- Diff Result Summary ---\n";
	Cout() << "Only in A:  " << result.only_in_a << "\n";
	Cout() << "Only in B:  " << result.only_in_b << "\n";
	Cout() << "In both:    " << result.in_both << "\n";
	Cout() << "Total entries: " << result.entries.GetCount() << "\n";

	Cout() << "\nDetailed Entries:\n";
	for(const VsmSessionDiffEntry& entry : result.entries) {
		Cout() << "  [" << entry.status << "] Frame " << entry.frame
		       << " (" << entry.severity << "): " << entry.message << "\n";
	}

	// --- Step 5: Acceptance checks ---
	Cout() << "\n--- Acceptance Checks ---\n";

	if(result.only_in_a != 1)
		{ Fail(Format("Expected only_in_a=1, got %d", result.only_in_a)); return; }
	Cout() << "OK: only_in_a == 1\n";

	if(result.only_in_b != 1)
		{ Fail(Format("Expected only_in_b=1, got %d", result.only_in_b)); return; }
	Cout() << "OK: only_in_b == 1\n";

	if(result.in_both != 1)
		{ Fail(Format("Expected in_both=1, got %d", result.in_both)); return; }
	Cout() << "OK: in_both == 1\n";

	if(result.entries.GetCount() != 3)
		{ Fail(Format("Expected 3 total entries, got %d", result.entries.GetCount())); return; }
	Cout() << "OK: 3 total entries\n";

	Cout() << "\nAll session diff checks passed.\n";

	// Cleanup
	DeleteFolderDeep(session_a_dir);
	DeleteFolderDeep(session_b_dir);
}
