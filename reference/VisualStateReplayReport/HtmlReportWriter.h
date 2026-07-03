#ifndef _VisualStateReplayReport_HtmlReportWriter_h_
#define _VisualStateReplayReport_HtmlReportWriter_h_

#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

// ---------------------------------------------------------------------------
// Writes a replay report as HTML (semantic markup only, no CSS or JS).
//
// Renders the same information as ReportWriter (session statistics and
// divergences table), with proper escaping of HTML-unsafe characters
// in content from OCR/model sources.

class VsmHtmlReportWriter {
public:
	VsmHtmlReportWriter() {}

	void SetLog(AppLog* sink) { log_.SetSink(sink); }
	CoreLog& GetLog()         { return log_; }

	// Write an HTML report for the given session to out_dir.
	// Creates the directory if needed.
	// Writes index.html alongside existing index.md.
	// Returns false on write failure.
	bool WriteHtml(const VsmSession& session, const String& out_dir);

private:
	CoreLog log_;

	// Helper: escape HTML-unsafe characters in user/OCR content
	static String EscapeHtml(const String& text);
};

#endif
