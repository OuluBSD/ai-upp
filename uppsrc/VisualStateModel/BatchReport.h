#ifndef _VisualStateModel_BatchReport_h_
#define _VisualStateModel_BatchReport_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Batch session entry — results for a single session directory

struct VsmBatchSessionEntry : Moveable<VsmBatchSessionEntry> {
	String session_dir;
	String session_id;
	int    divergence_count = 0;
	int    error_count      = 0;
	int    warning_count    = 0;
	bool   had_divergence_file = false;

	void Jsonize(JsonIO& json);
};

// ---------------------------------------------------------------------------
// Batch report result — aggregated summary across multiple sessions

struct VsmBatchReportResult : Moveable<VsmBatchReportResult> {
	int sessions_scanned    = 0;
	int sessions_with_data  = 0;
	int total_divergences   = 0;
	int total_errors        = 0;
	int total_warnings      = 0;
	Vector<VsmBatchSessionEntry> sessions;

	void Jsonize(JsonIO& json);
};

// ---------------------------------------------------------------------------
// Batch divergence report scanner

class VsmBatchDivergenceReport {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }

	// Scan directories for divergences.json and aggregate summary.
	// dirs: list of session root directories to scan.
	VsmBatchReportResult Run(const Vector<String>& dirs);

private:
	CoreLog log_;
};

} // namespace Upp

#endif
