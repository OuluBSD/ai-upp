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
	String sessions_root_dir;

	// Parse command-line arguments
	for(const String& arg : args) {
		if(arg == "--help") {
			Cout() << "Usage: VisualStateBatchReport [<sessions_root_dir>]\n";
			SetExitCode(0);
			return;
		} else {
			sessions_root_dir = arg;
		}
	}

	// If a sessions root dir was provided, use real data
	if(!sessions_root_dir.IsEmpty()) {
		if(!DirectoryExists(sessions_root_dir)) {
			Cout() << "ERROR: sessions root directory not found: " << sessions_root_dir << "\n";
			SetExitCode(1);
			return;
		}

		AppLog log;
		log.SetForwardToUppLog(false);

		// Enumerate subdirectories of the root
		Vector<String> session_dirs;
		FindFile ff(AppendFileName(sessions_root_dir, "*"));
		while(ff) {
			if(ff.IsFolder()) {
				session_dirs.Add(AppendFileName(sessions_root_dir, ff.GetName()));
			}
			ff.Next();
		}

		if(session_dirs.IsEmpty()) {
			Cout() << "ERROR: no subdirectories found in: " << sessions_root_dir << "\n";
			SetExitCode(1);
			return;
		}

		Cout() << "=== VisualStateModel Batch Divergence Report ===\n\n";
		Cout() << "Processing " << session_dirs.GetCount() << " session(s) from: " << sessions_root_dir << "\n\n";

		VsmBatchDivergenceReport batch_report;
		batch_report.SetLog(&log);
		VsmBatchReportResult result = batch_report.Run(session_dirs);

		Cout() << "\n--- Batch Report Summary ---\n";
		Cout() << "Sessions scanned:     " << result.sessions_scanned << "\n";
		Cout() << "Sessions with data:   " << result.sessions_with_data << "\n";
		Cout() << "Total divergences:    " << result.total_divergences << "\n";
		Cout() << "Total errors:         " << result.total_errors << "\n";
		Cout() << "Total warnings:       " << result.total_warnings << "\n";

		Cout() << "\nPer-Session Details:\n";
		for(const VsmBatchSessionEntry& entry : result.sessions) {
			Cout() << "  Session: " << entry.session_id << "\n";
			Cout() << "    Dir: " << entry.session_dir << "\n";
			Cout() << "    Had divergence file: " << (entry.had_divergence_file ? "yes" : "no") << "\n";
			Cout() << "    Divergence count: " << entry.divergence_count << "\n";
			Cout() << "    Errors: " << entry.error_count << "\n";
			Cout() << "    Warnings: " << entry.warning_count << "\n";
		}

		return;
	}

	// --- Synthetic self-check (no arguments provided) ---
	Cout() << "=== VisualStateModel Batch Divergence Report Demo ===\n\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// --- Step 1: Create 3 synthetic sessions ---
	String session_1_dir = AppendFileName(GetTempPath(), "vsm_batch_session_1");
	String session_2_dir = AppendFileName(GetTempPath(), "vsm_batch_session_2");
	String session_3_dir = AppendFileName(GetTempPath(), "vsm_batch_session_3");

	if(DirectoryExists(session_1_dir)) DeleteFolderDeep(session_1_dir);
	if(DirectoryExists(session_2_dir)) DeleteFolderDeep(session_2_dir);
	if(DirectoryExists(session_3_dir)) DeleteFolderDeep(session_3_dir);

	Cout() << "Creating session 1: " << session_1_dir << "\n";
	{
		VsmSessionStore store;
		VsmSyntheticSessionOptions opts;
		opts.output_dir = session_1_dir;
		opts.session_id = "batch-session-001";
		opts.frame_count = 1;
		opts.width = 48;
		opts.height = 48;
		opts.solid_value = 100;
		if(!VsmBuildSyntheticSession(opts, store))
			{ Fail("Cannot create session 1"); return; }
	}

	Cout() << "Creating session 2: " << session_2_dir << "\n";
	{
		VsmSessionStore store;
		VsmSyntheticSessionOptions opts;
		opts.output_dir = session_2_dir;
		opts.session_id = "batch-session-002";
		opts.frame_count = 1;
		opts.width = 48;
		opts.height = 48;
		opts.solid_value = 150;
		if(!VsmBuildSyntheticSession(opts, store))
			{ Fail("Cannot create session 2"); return; }
	}

	Cout() << "Creating session 3: " << session_3_dir << "\n";
	{
		VsmSessionStore store;
		VsmSyntheticSessionOptions opts;
		opts.output_dir = session_3_dir;
		opts.session_id = "batch-session-003";
		opts.frame_count = 1;
		opts.width = 48;
		opts.height = 48;
		opts.solid_value = 200;
		if(!VsmBuildSyntheticSession(opts, store))
			{ Fail("Cannot create session 3"); return; }
	}

	// --- Step 2: Seed session 1 and 2 with divergences.json ---
	Cout() << "\nSeeding divergences.json files...\n";

	Vector<VsmDivergence> session_1_divs;
	{
		VsmDivergence& d1 = session_1_divs.Add();
		d1.frame = 0;
		d1.ts = "2026-07-03T12:00:00Z";
		d1.severity = "warning";
		d1.message = "Test warning divergence 1";
		d1.region_id = "region-001";

		VsmDivergence& d2 = session_1_divs.Add();
		d2.frame = 0;
		d2.ts = "2026-07-03T12:00:01Z";
		d2.severity = "error";
		d2.message = "Test error divergence 1";
		d2.region_id = "region-002";
	}

	String div_path_1 = AppendFileName(session_1_dir, "divergences.json");
	if(!SaveFile(div_path_1, StoreAsJson(session_1_divs, true)))
		{ Fail("Cannot save divergences.json for session 1"); return; }
	Cout() << "  Saved " << session_1_divs.GetCount() << " divergences to session 1\n";

	Vector<VsmDivergence> session_2_divs;
	{
		VsmDivergence& d1 = session_2_divs.Add();
		d1.frame = 0;
		d1.ts = "2026-07-03T12:00:02Z";
		d1.severity = "warning";
		d1.message = "Test warning divergence 2";
		d1.region_id = "region-003";

		VsmDivergence& d2 = session_2_divs.Add();
		d2.frame = 0;
		d2.ts = "2026-07-03T12:00:03Z";
		d2.severity = "warning";
		d2.message = "Test warning divergence 2b";
		d2.region_id = "region-004";

		VsmDivergence& d3 = session_2_divs.Add();
		d3.frame = 1;
		d3.ts = "2026-07-03T12:00:04Z";
		d3.severity = "error";
		d3.message = "Test error divergence 2";
		d3.region_id = "region-005";
	}

	String div_path_2 = AppendFileName(session_2_dir, "divergences.json");
	if(!SaveFile(div_path_2, StoreAsJson(session_2_divs, true)))
		{ Fail("Cannot save divergences.json for session 2"); return; }
	Cout() << "  Saved " << session_2_divs.GetCount() << " divergences to session 2\n";

	// Session 3 has no divergences.json (this is not an error)
	Cout() << "  Session 3 has no divergences.json (expected scenario)\n";

	// --- Step 3: Run the batch report ---
	Cout() << "\n--- Running Batch Divergence Report ---\n";

	Vector<String> session_dirs;
	session_dirs.Add(session_1_dir);
	session_dirs.Add(session_2_dir);
	session_dirs.Add(session_3_dir);

	VsmBatchDivergenceReport batch_report;
	batch_report.SetLog(&log);
	VsmBatchReportResult result = batch_report.Run(session_dirs);

	// --- Step 4: Print the summary ---
	Cout() << "\n--- Batch Report Summary ---\n";
	Cout() << "Sessions scanned:     " << result.sessions_scanned << "\n";
	Cout() << "Sessions with data:   " << result.sessions_with_data << "\n";
	Cout() << "Total divergences:    " << result.total_divergences << "\n";
	Cout() << "Total errors:         " << result.total_errors << "\n";
	Cout() << "Total warnings:       " << result.total_warnings << "\n";

	Cout() << "\nPer-Session Details:\n";
	for(const VsmBatchSessionEntry& entry : result.sessions) {
		Cout() << "  Session: " << entry.session_id << "\n";
		Cout() << "    Dir: " << entry.session_dir << "\n";
		Cout() << "    Had divergence file: " << (entry.had_divergence_file ? "yes" : "no") << "\n";
		Cout() << "    Divergence count: " << entry.divergence_count << "\n";
		Cout() << "    Errors: " << entry.error_count << "\n";
		Cout() << "    Warnings: " << entry.warning_count << "\n";
	}

	// --- Step 5: Acceptance checks ---
	Cout() << "\n--- Acceptance Checks ---\n";

	if(result.sessions_scanned != 3)
		{ Fail("Expected 3 sessions scanned"); return; }
	Cout() << "OK: 3 sessions scanned\n";

	if(result.sessions_with_data != 2)
		{ Fail("Expected 2 sessions with divergence data"); return; }
	Cout() << "OK: 2 sessions with divergence data\n";

	// Total should be 2 (session 1) + 3 (session 2) = 5 divergences
	int expected_total_divs = session_1_divs.GetCount() + session_2_divs.GetCount();
	if(result.total_divergences != expected_total_divs)
		{ Fail(Format("Expected %d total divergences, got %d", expected_total_divs, result.total_divergences)); return; }
	Cout() << "OK: " << expected_total_divs << " total divergences\n";

	// Session 1 has 1 error + 1 warning, session 2 has 1 error + 2 warnings
	if(result.total_errors != 2)
		{ Fail("Expected 2 total errors"); return; }
	Cout() << "OK: 2 total errors\n";

	if(result.total_warnings != 3)
		{ Fail("Expected 3 total warnings"); return; }
	Cout() << "OK: 3 total warnings\n";

	Cout() << "\nAll batch report checks passed.\n";

	// Cleanup
	DeleteFolderDeep(session_1_dir);
	DeleteFolderDeep(session_2_dir);
	DeleteFolderDeep(session_3_dir);
}
