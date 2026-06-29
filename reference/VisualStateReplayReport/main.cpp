#include "ReportWriter.h"

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	const Vector<String>& args = CommandLine();
	String out_dir = args.GetCount() > 0 ? args[0]
	                                       : AppendFileName(GetTempPath(), "vsm_report");

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

	// Write report
	VsmReportWriter writer;
	writer.SetLog(&app_log);

	Cout() << "\nWriting report to: " << out_dir << "\n";
	if(!writer.Write(session, out_dir)) {
		Cout() << "ERROR: report write failed\n";
		SetExitCode(1);
		return;
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
	Cout() << "  events/  — per-event pages\n";
}
