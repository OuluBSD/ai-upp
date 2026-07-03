#include "VisualStateModel/VisualStateModel.h"

namespace Upp {

void VsmBatchSessionEntry::Jsonize(JsonIO& json)
{
	json("session_dir", session_dir)
	    ("session_id", session_id)
	    ("divergence_count", divergence_count)
	    ("error_count", error_count)
	    ("warning_count", warning_count)
	    ("had_divergence_file", had_divergence_file);
}

void VsmBatchReportResult::Jsonize(JsonIO& json)
{
	json("sessions_scanned", sessions_scanned)
	    ("sessions_with_data", sessions_with_data)
	    ("total_divergences", total_divergences)
	    ("total_errors", total_errors)
	    ("total_warnings", total_warnings)
	    ("sessions", sessions);
}

static String ExtractSessionId(const String& session_dir)
{
	String manifest_path = AppendFileName(session_dir, "manifest.json");
	if(!FileExists(manifest_path))
		return String();

	String json_content = LoadFile(manifest_path);
	if(json_content.IsEmpty())
		return String();

	try {
		VsmSessionManifest manifest;
		if(LoadFromJson(manifest, json_content)) {
			return manifest.session_id;
		}
	}
	catch(...) {
		// JSON parse error; ignore and return empty
	}

	return String();
}

VsmBatchReportResult VsmBatchDivergenceReport::Run(const Vector<String>& dirs)
{
	VsmBatchReportResult result;
	result.sessions_scanned = dirs.GetCount();

	for(const String& dir : dirs) {
		VsmBatchSessionEntry entry;
		entry.session_dir = dir;
		entry.session_id = ExtractSessionId(dir);

		// Look for divergences.json
		String divergence_path = AppendFileName(dir, "divergences.json");
		if(FileExists(divergence_path)) {
			entry.had_divergence_file = true;

			String json_content = LoadFile(divergence_path);
			if(!json_content.IsEmpty()) {
				try {
					Vector<VsmDivergence> divergences;
					if(LoadFromJson(divergences, json_content)) {
						entry.divergence_count = divergences.GetCount();

						// Count by severity
						for(const VsmDivergence& div : divergences) {
							if(div.severity == "error") {
								entry.error_count++;
							} else if(div.severity == "warning") {
								entry.warning_count++;
							}
						}

						result.sessions_with_data++;
					}
				}
				catch(...) {
					// JSON parse or load error; leave counts as zero
					LogWarn(log_, "BatchReport",
						   "Could not parse divergences.json at " + divergence_path);
				}
			}
		}

		// Accumulate into result
		result.total_divergences += entry.divergence_count;
		result.total_errors += entry.error_count;
		result.total_warnings += entry.warning_count;

		result.sessions.Add(entry);
	}

	return result;
}

} // namespace Upp
