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

	Cout() << "=== VisualStateModel Image Sequence Importer ===\n\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// --- Step 1: Generate a tiny synthetic sequence in a temp source dir ---
	String src_dir = AppendFileName(GetTempPath(), "vsm_import_src");
	String out_dir = AppendFileName(GetTempPath(), "vsm_import_out");

	if(DirectoryExists(src_dir)) DeleteFolderDeep(src_dir);
	if(DirectoryExists(out_dir)) DeleteFolderDeep(out_dir);
	RealizeDirectory(src_dir);

	int frame_w = 64, frame_h = 64;
	int num_frames = 4;
	Cout() << "Generating " << num_frames << " synthetic frames in: " << src_dir << "\n";

	for(int i = 0; i < num_frames; i++) {
		VsmImageBuffer img;
		if(i % 2 == 0)
			img = VsmImageBuffer::MakeSolid(frame_w, frame_h, (byte)(80 + i * 20), 1);
		else
			img = VsmImageBuffer::MakeCheckerboard(frame_w, frame_h, 8);

		String path = AppendFileName(src_dir, Format("%08d.vsm", i));
		if(!img.Save(path)) {
			Fail("Cannot write synthetic frame");
			return;
		}
	}
	Cout() << "Source frames written OK\n\n";

	// --- Step 2: Import the sequence ---
	VsmImageSequenceImportOptions opts;
	opts.source_dir = src_dir;
	opts.output_dir = out_dir;
	opts.session_id = "import-test-001";
	opts.fps        = 30;

	VsmImageSequenceImporter importer;
	importer.SetLog(&log);

	Cout() << "Importing sequence...\n";
	VsmImageSequenceImportResult res = importer.Import(opts);

	Cout() << "--- Import Summary ---\n";
	Cout() << "Success:           " << (res.success ? "yes" : "no") << "\n";
	Cout() << "Frames scanned:    " << res.frames_scanned  << "\n";
	Cout() << "Frames imported:   " << res.frames_imported << "\n";
	Cout() << "Frames skipped:    " << res.frames_skipped  << "\n";
	Cout() << "Session ID:        " << res.session_id  << "\n";
	Cout() << "Output dir:        " << res.output_dir  << "\n";
	if(res.HasWarnings()) {
		Cout() << "Warnings:\n";
		for(const VsmImportWarning& w : res.warnings)
			Cout() << "  " << w.filename << ": " << w.message << "\n";
	}
	Cout() << "\n";

	// --- Acceptance checks ---
	Cout() << "--- Acceptance Checks ---\n";

	if(!res.success)
		{ Fail("Import did not succeed"); return; }
	Cout() << "OK:   Import success\n";

	if(res.frames_imported != num_frames)
		{ Fail("Expected " + IntStr(num_frames) + " frames imported"); return; }
	Cout() << "OK:   All " << num_frames << " frames imported\n";

	if(res.frames_skipped != 0)
		{ Fail("Unexpected skipped frames"); return; }
	Cout() << "OK:   No frames skipped\n";

	// --- Step 3: Verify output session can be opened by VsmSessionStoreSource ---
	VsmSessionStoreSource src_reader;
	src_reader.SetLog(&log);
	if(!src_reader.Open(out_dir))
		{ Fail("VsmSessionStoreSource cannot open imported session"); return; }
	if(src_reader.GetWidth() != frame_w || src_reader.GetHeight() != frame_h)
		{ Fail("Imported session dimensions mismatch"); return; }
	Cout() << "OK:   Imported session opens via VsmSessionStoreSource\n";
	Cout() << "      " << src_reader.GetSourceInfo() << "\n";

	int frames_read = 0;
	VsmImageBuffer out_frame; int64 ts_ms = 0;
	while(src_reader.ReadFrame(out_frame, ts_ms)) frames_read++;
	if(frames_read != num_frames)
		{ Fail("ReadFrame count mismatch: expected " + IntStr(num_frames) +
		       " got " + IntStr(frames_read)); return; }
	Cout() << "OK:   All " << frames_read << " frames readable from imported session\n";

	Cout() << "\nAll import checks passed.\n";

	// Cleanup
	DeleteFolderDeep(src_dir);
	DeleteFolderDeep(out_dir);
}
