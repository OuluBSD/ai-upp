#include "VisualStateModel/VisualStateModel.h"

namespace Upp {

void VsmSessionDiffEntry::Jsonize(JsonIO& json)
{
	json("status", status)
	    ("frame", frame)
	    ("severity", severity)
	    ("message", message);
}

void VsmSessionDiffResult::Jsonize(JsonIO& json)
{
	json("only_in_a", only_in_a)
	    ("only_in_b", only_in_b)
	    ("in_both", in_both)
	    ("entries", entries);
}

VsmSessionDiffResult VsmSessionDiff::Compare(const String& session_dir_a,
                                             const String& session_dir_b)
{
	VsmSessionDiffResult result;

	// Load divergences from session A
	Vector<VsmDivergence> divs_a;
	String div_path_a = AppendFileName(session_dir_a, "divergences.json");
	if(FileExists(div_path_a)) {
		String json_content = LoadFile(div_path_a);
		if(!json_content.IsEmpty()) {
			try {
				if(!LoadFromJson(divs_a, json_content)) {
					LogWarn(log_, "SessionDiff",
					        "Could not deserialize divergences.json from " + session_dir_a);
				}
			}
			catch(...) {
				LogWarn(log_, "SessionDiff",
				        "Could not parse divergences.json from " + session_dir_a);
			}
		}
	}

	// Load divergences from session B
	Vector<VsmDivergence> divs_b;
	String div_path_b = AppendFileName(session_dir_b, "divergences.json");
	if(FileExists(div_path_b)) {
		String json_content = LoadFile(div_path_b);
		if(!json_content.IsEmpty()) {
			try {
				if(!LoadFromJson(divs_b, json_content)) {
					LogWarn(log_, "SessionDiff",
					        "Could not deserialize divergences.json from " + session_dir_b);
				}
			}
			catch(...) {
				LogWarn(log_, "SessionDiff",
				        "Could not parse divergences.json from " + session_dir_b);
			}
		}
	}

	// Track which B divergences have been matched
	Vector<bool> matched_b(divs_b.GetCount(), false);

	// Process all divergences from A
	for(const VsmDivergence& div_a : divs_a) {
		bool found_in_b = false;

		// Look for matching divergence in B (same frame and message)
		for(int i = 0; i < divs_b.GetCount(); i++) {
			const VsmDivergence& div_b = divs_b[i];
			if(div_a.frame == div_b.frame && div_a.message == div_b.message) {
				// Match found
				found_in_b = true;
				matched_b[i] = true;

				// Add to result as in_both
				VsmSessionDiffEntry& entry = result.entries.Add();
				entry.status = "in_both";
				entry.frame = div_a.frame;
				entry.severity = div_a.severity;
				entry.message = div_a.message;
				result.in_both++;

				break;
			}
		}

		if(!found_in_b) {
			// Entry is only in A
			VsmSessionDiffEntry& entry = result.entries.Add();
			entry.status = "only_in_a";
			entry.frame = div_a.frame;
			entry.severity = div_a.severity;
			entry.message = div_a.message;
			result.only_in_a++;
		}
	}

	// Process unmatched divergences from B
	for(int i = 0; i < divs_b.GetCount(); i++) {
		if(!matched_b[i]) {
			const VsmDivergence& div_b = divs_b[i];
			VsmSessionDiffEntry& entry = result.entries.Add();
			entry.status = "only_in_b";
			entry.frame = div_b.frame;
			entry.severity = div_b.severity;
			entry.message = div_b.message;
			result.only_in_b++;
		}
	}

	return result;
}

} // namespace Upp
