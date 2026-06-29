#ifndef _VisualStateReplayReport_ReportWriter_h_
#define _VisualStateReplayReport_ReportWriter_h_

#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

// ---------------------------------------------------------------------------
// Writes a replay debug report to a directory.
//
//   report/
//     index.md
//     events/
//       000001.md  (per-event pages)

class VsmReportWriter {
public:
	VsmReportWriter() {}

	void SetLog(AppLog* sink) { log_.SetSink(sink); }
	CoreLog& GetLog()         { return log_; }

	// Write a full report for the given session to out_dir.
	// Creates the directory if needed.
	// Returns false on write failure.
	bool Write(const VsmSession& session, const String& out_dir);

private:
	CoreLog log_;

	bool WriteIndex(const VsmSession& session, const String& out_dir);
	bool WriteEventPage(const VsmChangeEvent& ce, int seq, const String& events_dir);
	bool WriteDivergencePage(const VsmDivergence& div, int seq, const String& events_dir);

	static String SeverityBadge(const String& sev);
	static String RegionTable(const Vector<VsmChangedRect>& regions);
};

#endif
