#include "ReportWriter.h"
#include "HtmlReportWriter.h"

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	const Vector<String>& args = CommandLine();
	bool write_html = false;
	String out_dir = AppendFileName(GetTempPath(), "vsm_report");

	// Parse command-line arguments
	for(const String& arg : args) {
		if(arg == "--help") {
			Cout() << "Usage: VisualStateReplayReport [--html] [<output_dir>]\n";
			SetExitCode(0);
			return;
		} else if(arg == "--html") {
			write_html = true;
		} else {
			out_dir = arg;
		}
	}

	// Set up structured logging
	AppLog app_log;
	app_log.SetForwardToUppLog(true);

	// Load sample session
	VsmGroundTruthLoader loader;
	loader.SetLog(&app_log);

	VsmSession session;
	String sample_json = VsmMakeSampleJson();
	String tmp = AppendFileName(GetTempPath(), "vsm_report_input.json");
	SaveFile(tmp, sample_json);

	Cout() << "Loading sample session from: " << tmp << "\n";
	if(!loader.Load(tmp, session)) {
		Cout() << "ERROR: failed to load sample session\n";
		SetExitCode(1);
		FileDelete(tmp);
		return;
	}
	FileDelete(tmp);

	Cout() << "Session: '" << session.session_id << "'\n";
	Cout() << "Events: frames=" << session.frames.GetCount()
	       << " changes=" << session.changes.GetCount()
	       << " regions=" << session.regions.GetCount()
	       << " divergences=" << session.divergences.GetCount() << "\n\n";

	// Also run replay to collect all events
	VsmReplaySession replay;
	replay.SetLog(&app_log);
	String tmp2 = AppendFileName(GetTempPath(), "vsm_report_replay.json");
	SaveFile(tmp2, sample_json);
	replay.Load(tmp2);
	FileDelete(tmp2);
	replay.RunAll();

	Cout() << "Replay complete: " << replay.GetTotalEvents() << " events stepped\n";
	if(!replay.GetDivergences().IsEmpty())
		Cout() << "Divergences found: " << replay.GetDivergences().GetCount() << "\n";

	// Save replay divergences as divergences.json so WriteIndex picks them up
	if(!replay.GetDivergences().IsEmpty()) {
		RealizeDirectory(out_dir);
		SaveFile(AppendFileName(out_dir, "divergences.json"),
		         StoreAsJson(replay.GetDivergences(), true));
		Cout() << "Divergences saved: " << replay.GetDivergences().GetCount()
		       << " record(s) → divergences.json\n";
	}

	// Write report
	VsmReportWriter writer;
	writer.SetLog(&app_log);

	Cout() << "\nWriting report to: " << out_dir << "\n";
	if(!writer.Write(session, out_dir)) {
		Cout() << "ERROR: report write failed\n";
		SetExitCode(1);
		return;
	}

	// Write HTML report if requested
	if(write_html) {
		VsmHtmlReportWriter html_writer;
		html_writer.SetLog(&app_log);
		if(!html_writer.WriteHtml(session, out_dir)) {
			Cout() << "ERROR: HTML report write failed\n";
			SetExitCode(1);
			return;
		}
	}

	// Print log summary
	Cout() << "\n=== AppLog records ===\n";
	for(const AppLogRecord& r : app_log.GetRecords()) {
		static const char* kLevel[] = {"TRACE","DEBUG","INFO ","WARN ","ERROR"};
		int lv = clamp(r.level, 0, 4);
		Cout() << Format("[%s][%s] %s\n", kLevel[lv], r.channel, r.message);
	}

	Cout() << "\nReport written to: " << out_dir << "\n";
	Cout() << "  index.md — main entry point\n";
	if(write_html)
		Cout() << "  index.html — HTML version\n";
	Cout() << "  events/  — per-event pages\n";
}
