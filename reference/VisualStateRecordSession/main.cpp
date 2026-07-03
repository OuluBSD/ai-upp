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
	String output_dir;

	// Parse command-line arguments
	for(const String& arg : args) {
		if(arg == "--help") {
			Cout() << "Usage: VisualStateRecordSession.exe [--help] [<output_dir>]\n";
			SetExitCode(0);
			return;
		} else {
			output_dir = arg;
		}
	}

	Cout() << "=== VisualStateModel Capture Sink Demo ===\n\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// If output_dir was provided, validate it
	bool is_real_output = !output_dir.IsEmpty();
	if(is_real_output && DirectoryExists(output_dir)) {
		Cout() << "ERROR: Output directory already exists: " << output_dir << "\n";
		SetExitCode(1);
		return;
	}

	// --- Step 1: Create a source session with 3 real frames ---
	String src_session_dir = AppendFileName(GetTempPath(), "vsm_record_src");
	String rec_session_dir = is_real_output ? output_dir : AppendFileName(GetTempPath(), "vsm_record_out");

	if(DirectoryExists(src_session_dir)) DeleteFolderDeep(src_session_dir);
	if(DirectoryExists(rec_session_dir)) DeleteFolderDeep(rec_session_dir);

	Cout() << "Creating source session: " << src_session_dir << "\n";
	{
		VsmSessionStore src_store;
		src_store.SetLog(&log);
		if(!src_store.Create(src_session_dir, "record-src-001", 48, 48))
			{ Fail("Cannot create source session"); return; }

		src_store.SaveFrameImage(0, VsmImageBuffer::MakeSolid(48, 48, 80, 1));
		src_store.SaveFrameImage(1, VsmImageBuffer::MakeGradient(48, 48));
		src_store.SaveFrameImage(2, VsmImageBuffer::MakeCheckerboard(48, 48, 6));
		src_store.SaveManifest();
	}

	// --- Step 2: Open source via VsmSessionStoreSource ---
	VsmSessionStoreSource src;
	src.SetLog(&log);
	if(!src.Open(src_session_dir))
		{ Fail("Cannot open source session"); return; }
	Cout() << "Source: " << src.GetSourceInfo() << "\n\n";

	// --- Step 3: Record into new session via VsmCaptureSink ---
	VsmCaptureSinkOptions opts;
	opts.output_dir = rec_session_dir;
	opts.session_id = "record-out-001";

	VsmCaptureSink sink;
	sink.SetLog(&log);

	Cout() << "Recording...\n";
	VsmCaptureSummary summary = sink.Record(src, opts);

	Cout() << "--- Record Summary ---\n";
	Cout() << "Success:          " << (summary.success ? "yes" : "no") << "\n";
	Cout() << "Frames recorded:  " << summary.frames_recorded << "\n";
	Cout() << "Frames dropped:   " << summary.frames_dropped  << "\n";
	Cout() << "Errors:           " << summary.error_count     << "\n";
	Cout() << "Session ID:       " << summary.session_id      << "\n";
	Cout() << "Output dir:       " << summary.output_dir      << "\n";
	Cout() << "Source info:      " << summary.source_info     << "\n\n";

	// --- Acceptance checks ---
	Cout() << "--- Acceptance Checks ---\n";

	if(!summary.success)
		{ Fail("Record did not succeed"); return; }
	Cout() << "OK:   Record success\n";

	if(summary.frames_recorded != 3)
		{ Fail("Expected 3 frames recorded"); return; }
	Cout() << "OK:   3 frames recorded\n";

	if(summary.frames_dropped != 0)
		{ Fail("Unexpected dropped frames"); return; }
	Cout() << "OK:   No frames dropped\n";

	// --- Step 4: Verify the recorded session can be replayed ---
	VsmSessionStoreSource replay;
	replay.SetLog(&log);
	if(!replay.Open(rec_session_dir))
		{ Fail("Cannot open recorded session"); return; }
	if(replay.GetWidth() != 48 || replay.GetHeight() != 48)
		{ Fail("Recorded session dimensions mismatch"); return; }
	Cout() << "OK:   Recorded session opens via VsmSessionStoreSource\n";
	Cout() << "      " << replay.GetSourceInfo() << "\n";

	int replayed = 0;
	VsmImageBuffer out; int64 ts_ms = 0;
	while(replay.ReadFrame(out, ts_ms)) replayed++;
	if(replayed != 3)
		{ Fail("Replay frame count mismatch"); return; }
	Cout() << "OK:   All " << replayed << " frames replay correctly\n";

	// --- Step 5: Run observation pipeline on recorded session ---
	replay.Close();
	replay.Open(rec_session_dir);

	VsmFakeOcrEngine ocr("RecordTest", 0.85);
	Vector<VsmOcrRule> ocr_rules;
	VsmOcrRule& r = ocr_rules.Add();
	r.rule_id = "ocr-rec"; r.annotation_id = String();
	r.expectation.mode = VSM_EXPECT_EXACT;
	r.expectation.expected_text = "RecordTest";
	r.confidence_threshold = 0.5;

	VsmAnnotationLayer ann;
	ann.session_id = "record-out-001";
	VsmRegionAnnotation& a = ann.annotations.Add();
	a.id = "ann-rec"; a.name = "FullFrame"; a.x = 0; a.y = 0; a.w = 48; a.h = 48;
	r.annotation_id = "ann-rec";

	VsmObservationPipeline pipe;
	pipe.SetLog(&log);
	pipe.SetAnnotationLayer(&ann);
	pipe.SetOcrRules(&ocr_rules);
	pipe.SetOcrEngine(&ocr);
	VsmSessionStore& out_store = sink.GetStore();
	pipe.SetSessionStore(&out_store);

	VsmPipelineRunSummary run = pipe.RunFromSource(replay);
	if(!run.success)
		{ Fail("Pipeline run on recorded session failed"); return; }
	if(run.frames_processed != 3)
		{ Fail("Pipeline processed wrong frame count"); return; }
	Cout() << "OK:   Pipeline ran on recorded session — " << run.frames_processed
	       << " frames, " << run.observations_made << " observations\n";

	Cout() << "\nAll record/replay checks passed.\n";

	// Cleanup (only for synthetic temp directories)
	DeleteFolderDeep(src_session_dir);
	if(!is_real_output)
		DeleteFolderDeep(rec_session_dir);
	else
		Cout() << "\nRecorded session saved to: " << rec_session_dir << "\n";
}
