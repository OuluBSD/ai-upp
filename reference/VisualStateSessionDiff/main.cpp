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

	const Vector<String>& args = CommandLine();
	String session_a_dir, session_b_dir;
	Vector<String> positional_args;

	// Parse command-line arguments
	for(const String& arg : args) {
		if(arg == "--help") {
			Cout() << "Usage: VisualStateSessionDiff [<session_a> <session_b>]\n";
			SetExitCode(0);
			return;
		} else {
			positional_args.Add(arg);
		}
	}

	// If positional arguments were provided, use real data
	if(positional_args.GetCount() > 0) {
		if(positional_args.GetCount() != 2) {
			Cout() << "ERROR: expected exactly 2 session directories, got " << positional_args.GetCount() << "\n";
			SetExitCode(1);
			return;
		}

		session_a_dir = positional_args[0];
		session_b_dir = positional_args[1];

		if(!DirectoryExists(session_a_dir)) {
			Cout() << "ERROR: session_a directory not found: " << session_a_dir << "\n";
			SetExitCode(1);
			return;
		}

		if(!DirectoryExists(session_b_dir)) {
			Cout() << "ERROR: session_b directory not found: " << session_b_dir << "\n";
			SetExitCode(1);
			return;
		}

		AppLog log;
		log.SetForwardToUppLog(false);

		Cout() << "=== VisualStateModel Session Diff ===\n\n";
		Cout() << "Comparing:\n";
		Cout() << "  Session A: " << session_a_dir << "\n";
		Cout() << "  Session B: " << session_b_dir << "\n\n";

		VsmSessionDiff session_diff;
		session_diff.SetLog(&log);
		VsmSessionDiffResult result = session_diff.Compare(session_a_dir, session_b_dir);

		Cout() << "--- Diff Result Summary ---\n";
		Cout() << "Only in A:  " << result.only_in_a << "\n";
		Cout() << "Only in B:  " << result.only_in_b << "\n";
		Cout() << "In both:    " << result.in_both << "\n";
		Cout() << "Total entries: " << result.entries.GetCount() << "\n";

		Cout() << "\nDetailed Entries:\n";
		for(const VsmSessionDiffEntry& entry : result.entries) {
			Cout() << "  [" << entry.status << "] Frame " << entry.frame
			       << " (" << entry.severity << "): " << entry.message << "\n";
		}

		return;
	}

	// --- Synthetic self-check (no arguments provided) ---
	Cout() << "=== VisualStateModel Session Diff Demo ===\n\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// --- Step 1: Create 2 synthetic sessions ---
	session_a_dir = AppendFileName(GetTempPath(), "vsm_session_diff_a");
	session_b_dir = AppendFileName(GetTempPath(), "vsm_session_diff_b");

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
