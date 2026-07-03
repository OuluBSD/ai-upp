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

	Cout() << "=== VisualStateModel Session Validator Demo ===\n\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// --- Step 1: Create a valid session with 3 frames ---
	String valid_session_dir = AppendFileName(GetTempPath(), "vsm_validate_valid");
	String broken_session_dir = AppendFileName(GetTempPath(), "vsm_validate_broken");

	if(DirectoryExists(valid_session_dir)) DeleteFolderDeep(valid_session_dir);
	if(DirectoryExists(broken_session_dir)) DeleteFolderDeep(broken_session_dir);

	Cout() << "Creating valid session: " << valid_session_dir << "\n";
	{
		VsmSessionStore store;
		store.SetLog(&log);
		if(!store.Create(valid_session_dir, "validate-valid-001", 64, 64))
			{ Fail("Cannot create valid session"); return; }

		// Add 3 frames
		store.AllocateFrame(0);
		store.AllocateFrame(1);
		store.AllocateFrame(2);

		// Add 1 crop
		store.AllocateCrop("region-001");

		if(!store.SaveManifest())
			{ Fail("Cannot save manifest for valid session"); return; }
	}

	Cout() << "Creating broken session: " << broken_session_dir << "\n";
	{
		VsmSessionStore store;
		store.SetLog(&log);
		if(!store.Create(broken_session_dir, "validate-broken-001", 64, 64))
			{ Fail("Cannot create broken session"); return; }

		// Add 3 frames
		store.AllocateFrame(0);
		store.AllocateFrame(1);
		store.AllocateFrame(2);

		// Add 1 crop
		store.AllocateCrop("region-001");

		if(!store.SaveManifest())
			{ Fail("Cannot save manifest for broken session"); return; }
	}

	// --- Step 2: Break the broken session by deleting a frame asset file ---
	String frame_to_delete = AppendFileName(broken_session_dir, "frames/00000001.placeholder");
	if(!DeleteFile(frame_to_delete))
		{ Fail("Cannot delete frame file to break session"); return; }

	Cout() << "\n--- Validation Check 1: Valid Session ---\n";

	VsmSessionValidator validator;
	validator.SetLog(&log);
	VsmValidationResult valid_result = validator.Validate(valid_session_dir);

	Cout() << "Result: " << (valid_result.ok ? "PASS" : "FAIL") << "\n";
	Cout() << "Frames checked: " << valid_result.frames_checked << "\n";
	Cout() << "Crops checked: " << valid_result.crops_checked << "\n";
	Cout() << "Issues: " << valid_result.issues.GetCount() << "\n";

	for(const VsmValidationIssue& issue : valid_result.issues) {
		Cout() << "  [" << issue.severity << "] " << issue.message << "\n";
	}

	if(!valid_result.ok)
		{ Fail("Valid session validation should have passed"); return; }
	Cout() << "OK: Valid session passes validation\n";

	Cout() << "\n--- Validation Check 2: Broken Session ---\n";

	VsmValidationResult broken_result = validator.Validate(broken_session_dir);

	Cout() << "Result: " << (broken_result.ok ? "PASS" : "FAIL") << "\n";
	Cout() << "Frames checked: " << broken_result.frames_checked << "\n";
	Cout() << "Crops checked: " << broken_result.crops_checked << "\n";
	Cout() << "Issues: " << broken_result.issues.GetCount() << "\n";

	for(const VsmValidationIssue& issue : broken_result.issues) {
		Cout() << "  [" << issue.severity << "] " << issue.message << "\n";
	}

	if(broken_result.ok)
		{ Fail("Broken session validation should have failed"); return; }

	// Check that we detected the missing frame file
	bool found_missing_frame_error = false;
	for(const VsmValidationIssue& issue : broken_result.issues) {
		if(issue.severity == "error" && issue.message.Find("Frame asset file not found") >= 0) {
			found_missing_frame_error = true;
			break;
		}
	}

	if(!found_missing_frame_error)
		{ Fail("Broken session should report missing frame asset error"); return; }
	Cout() << "OK: Broken session correctly reports missing frame asset error\n";

	Cout() << "\n--- All validation checks passed ---\n";

	// Cleanup
	DeleteFolderDeep(valid_session_dir);
	DeleteFolderDeep(broken_session_dir);
}
