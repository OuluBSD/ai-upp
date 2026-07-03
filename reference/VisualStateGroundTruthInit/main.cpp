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
	String session_dir, output_file;
	Vector<String> positional_args;

	// Parse command-line arguments
	for(const String& arg : args) {
		if(arg == "--help") {
			Cout() << "Usage: VisualStateGroundTruthInit [<session_dir> <output_template_path>]\n";
			SetExitCode(0);
			return;
		} else {
			positional_args.Add(arg);
		}
	}

	// If positional arguments were provided, use real data
	if(positional_args.GetCount() > 0) {
		if(positional_args.GetCount() != 2) {
			Cout() << "ERROR: expected exactly 2 arguments (session_dir and output_template_path), got " << positional_args.GetCount() << "\n";
			SetExitCode(1);
			return;
		}

		session_dir = positional_args[0];
		output_file = positional_args[1];

		if(!DirectoryExists(session_dir)) {
			Cout() << "ERROR: session directory not found: " << session_dir << "\n";
			SetExitCode(1);
			return;
		}

		AppLog log;
		log.SetForwardToUppLog(false);

		Cout() << "=== VisualStateModel Ground Truth Template Generator ===\n\n";
		Cout() << "Generating template from session: " << session_dir << "\n";
		Cout() << "Output file: " << output_file << "\n\n";

		VsmGroundTruthTemplateGenerator generator;
		generator.SetLog(&log);

		VsmGroundTruthTemplateOptions opts;
		opts.session_dir = session_dir;
		opts.output_path = output_file;

		VsmGroundTruthTemplateResult result = generator.Generate(opts);

		if(!result.success) {
			Cout() << "ERROR: generation failed\n";
			SetExitCode(1);
			return;
		}

		Cout() << "Success: " << result.session_id << "\n";
		Cout() << "Frame count: " << result.frame_count << "\n";
		Cout() << "Output path: " << result.output_path << "\n";

		return;
	}

	// --- Synthetic self-check (no arguments provided) ---
	Cout() << "=== VisualStateModel Ground Truth Template Generator Demo ===\n\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// --- Step 1: Create a synthetic 3-frame session ---
	session_dir = AppendFileName(GetTempPath(), "vsm_ground_truth_init");
	output_file = AppendFileName(GetTempPath(), "ground_truth_template.json");

	if(DirectoryExists(session_dir)) DeleteFolderDeep(session_dir);
	if(FileExists(output_file)) DeleteFile(output_file);

	Cout() << "Creating synthetic 3-frame session: " << session_dir << "\n";
	{
		VsmSessionStore store;
		store.SetLog(&log);
		if(!store.Create(session_dir, "gt-template-demo", 48, 48))
			{ Fail("Cannot create session"); return; }

		// Create 3 frame entries
		store.SaveFrameImage(0, VsmImageBuffer::MakeSolid(48, 48, 100, 1));
		store.SaveFrameImage(1, VsmImageBuffer::MakeSolid(48, 48, 110, 1));
		store.SaveFrameImage(2, VsmImageBuffer::MakeSolid(48, 48, 120, 1));

		if(!store.SaveManifest())
			{ Fail("Cannot save manifest"); return; }
		Cout() << "  Created session with 3 frames\n";
	}

	// --- Step 2: Run the template generator ---
	Cout() << "\nGenerating template from session...\n";

	VsmGroundTruthTemplateGenerator generator;
	generator.SetLog(&log);

	VsmGroundTruthTemplateOptions opts;
	opts.session_dir = session_dir;
	opts.output_path = output_file;

	VsmGroundTruthTemplateResult result = generator.Generate(opts);

	if(!result.success)
		{ Fail("Generate failed"); return; }

	Cout() << "  Success: " << result.session_id << "\n";
	Cout() << "  Frame count: " << result.frame_count << "\n";
	Cout() << "  Output path: " << result.output_path << "\n";

	// --- Step 3: Load and verify the generated file ---
	Cout() << "\nLoading and verifying generated template...\n";

	VsmGroundTruthSession loaded;
	VsmGroundTruthLoader loader;
	loader.SetLog(&log);
	if(!loader.Load(output_file, loaded))
		{ Fail("Cannot parse generated JSON"); return; }

	// --- Step 4: Verify frame count ---
	if(loaded.frames.GetCount() != 3)
		{ Fail(Format("Expected 3 frames, got %d", loaded.frames.GetCount())); return; }
	Cout() << "OK: frame_count == 3\n";

	// --- Step 5: Verify exactly one example divergence entry ---
	if(loaded.divergences.GetCount() != 1)
		{ Fail(Format("Expected 1 example divergence, got %d", loaded.divergences.GetCount())); return; }
	Cout() << "OK: exactly 1 example divergence\n";

	const VsmDivergence& example = loaded.divergences[0];
	if(example.message != "EXAMPLE - replace or remove")
		{ Fail(Format("Expected example message, got: %s", example.message)); return; }
	Cout() << "OK: example divergence message is correct\n";

	// --- Step 6: Print the loaded template ---
	Cout() << "\n--- Generated Template ---\n";
	Cout() << "Session ID: " << loaded.session_id << "\n";
	Cout() << "Frame count: " << loaded.frames.GetCount() << "\n";
	Cout() << "Divergences: " << loaded.divergences.GetCount() << "\n";
	for(int i = 0; i < loaded.divergences.GetCount(); i++) {
		const VsmDivergence& div = loaded.divergences[i];
		Cout() << "  [" << i << "] Frame " << div.frame << " (" << div.severity
		       << "): " << div.message << "\n";
	}

	Cout() << "\n--- Acceptance Checks ---\n";

	if(loaded.session_id != "gt-template-demo")
		{ Fail("Session ID mismatch"); return; }
	Cout() << "OK: session_id == \"gt-template-demo\"\n";

	if(loaded.frame_width != 48 || loaded.frame_height != 48)
		{ Fail("Frame dimensions mismatch"); return; }
	Cout() << "OK: frame dimensions 48x48\n";

	Cout() << "\nAll ground truth template checks passed.\n";

	// Cleanup
	DeleteFolderDeep(session_dir);
	DeleteFile(output_file);
}
