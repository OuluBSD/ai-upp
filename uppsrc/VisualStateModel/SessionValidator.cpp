#include "VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// VsmValidationIssue

void VsmValidationIssue::Jsonize(JsonIO& json)
{
	json("severity", severity)
	    ("message",  message);
}

// ---------------------------------------------------------------------------
// VsmValidationResult

void VsmValidationResult::Jsonize(JsonIO& json)
{
	json("ok",              ok)
	    ("frames_checked",  frames_checked)
	    ("crops_checked",   crops_checked)
	    ("issues",          issues);
}

// ---------------------------------------------------------------------------
// VsmSessionValidator

VsmValidationResult VsmSessionValidator::Validate(const String& session_dir)
{
	VsmValidationResult result;
	result.ok = false;
	result.frames_checked = 0;
	result.crops_checked = 0;

	// --- Check 1: Session directory exists ---
	if(!DirectoryExists(session_dir)) {
		VsmValidationIssue& issue = result.issues.Add();
		issue.severity = "error";
		issue.message = Format("Session directory does not exist: %s", session_dir);
		LogError(log_, "VsmSessionValidator", issue.message);
		return result;
	}

	// --- Check 2: manifest.json exists and parses ---
	String manifest_path = AppendFileName(session_dir, "manifest.json");
	if(!FileExists(manifest_path)) {
		VsmValidationIssue& issue = result.issues.Add();
		issue.severity = "error";
		issue.message = Format("manifest.json not found at: %s", manifest_path);
		LogError(log_, "VsmSessionValidator", issue.message);
		return result;
	}

	String json_content = LoadFile(manifest_path);
	if(json_content.IsEmpty()) {
		VsmValidationIssue& issue = result.issues.Add();
		issue.severity = "error";
		issue.message = Format("Cannot read manifest.json: %s", manifest_path);
		LogError(log_, "VsmSessionValidator", issue.message);
		return result;
	}

	VsmSessionManifest manifest;
	if(!LoadFromJson(manifest, json_content)) {
		VsmValidationIssue& issue = result.issues.Add();
		issue.severity = "error";
		issue.message = Format("Cannot parse manifest.json: %s", manifest_path);
		LogError(log_, "VsmSessionValidator", issue.message);
		return result;
	}

	// --- Check 3: session_id is non-empty ---
	if(manifest.session_id.IsEmpty()) {
		VsmValidationIssue& issue = result.issues.Add();
		issue.severity = "warning";
		issue.message = "session_id is empty in manifest";
		LogWarn(log_, "VsmSessionValidator", issue.message);
	}

	// --- Check 4: Frame asset files exist and frame_indexes are unique ---
	Index<int> seen_frame_indexes;
	for(const VsmFrameAsset& frame : manifest.frames) {
		// Check for duplicate frame_index
		if(seen_frame_indexes.Find(frame.frame_index) >= 0) {
			VsmValidationIssue& issue = result.issues.Add();
			issue.severity = "warning";
			issue.message = Format("Duplicate frame_index: %d", frame.frame_index);
			LogWarn(log_, "VsmSessionValidator", issue.message);
		} else {
			seen_frame_indexes.Add(frame.frame_index);
		}

		// Check if asset file exists
		String asset_path = AppendFileName(session_dir, frame.relative_path);
		if(!FileExists(asset_path)) {
			VsmValidationIssue& issue = result.issues.Add();
			issue.severity = "error";
			issue.message = Format("Frame asset file not found: %s (referenced as frame_index=%d)",
			                       frame.relative_path, frame.frame_index);
			LogError(log_, "VsmSessionValidator", issue.message);
		}
		result.frames_checked++;
	}

	// --- Check 5: Crop asset files exist ---
	for(const VsmCropAsset& crop : manifest.crops) {
		String asset_path = AppendFileName(session_dir, crop.relative_path);
		if(!FileExists(asset_path)) {
			VsmValidationIssue& issue = result.issues.Add();
			issue.severity = "error";
			issue.message = Format("Crop asset file not found: %s (region_id=%s)",
			                       crop.relative_path, crop.region_id);
			LogError(log_, "VsmSessionValidator", issue.message);
		}
		result.crops_checked++;
	}

	// --- Determine overall success ---
	bool has_error = false;
	for(const VsmValidationIssue& issue : result.issues) {
		if(issue.severity == "error") {
			has_error = true;
			break;
		}
	}
	result.ok = !has_error;

	if(result.ok) {
		LogInfo(log_, "VsmSessionValidator",
		        Format("Validation passed: %d frames, %d crops",
		               result.frames_checked, result.crops_checked));
	} else {
		LogError(log_, "VsmSessionValidator",
		         Format("Validation failed with %d errors",
		                (int)result.issues.GetCount()));
	}

	return result;
}

} // namespace Upp
