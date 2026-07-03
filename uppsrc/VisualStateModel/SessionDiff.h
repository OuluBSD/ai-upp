#ifndef _VisualStateModel_SessionDiff_h_
#define _VisualStateModel_SessionDiff_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Session diff entry — a single divergence in the comparison result

struct VsmSessionDiffEntry : Moveable<VsmSessionDiffEntry> {
	String status;    // "only_in_a", "only_in_b", "in_both"
	int    frame = -1;
	String severity;
	String message;

	void Jsonize(JsonIO& json);
};

// ---------------------------------------------------------------------------
// Session diff result — comparison of two sessions' divergences

struct VsmSessionDiffResult : Moveable<VsmSessionDiffResult> {
	int only_in_a = 0;
	int only_in_b = 0;
	int in_both   = 0;
	Vector<VsmSessionDiffEntry> entries;

	void Jsonize(JsonIO& json);
};

// ---------------------------------------------------------------------------
// Session diff comparator

class VsmSessionDiff {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }

	// Loads divergences.json from each session directory and compares.
	// Matching key: same frame and same message (exact string match).
	// Missing divergences.json is treated as zero divergences (not an error).
	VsmSessionDiffResult Compare(const String& session_dir_a,
	                              const String& session_dir_b);

private:
	CoreLog log_;
};

} // namespace Upp

#endif
