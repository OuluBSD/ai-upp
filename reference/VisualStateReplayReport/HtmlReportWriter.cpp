#include "HtmlReportWriter.h"

String VsmHtmlReportWriter::EscapeHtml(const String& text)
{
	String out;
	for(char c : text) {
		if(c == '<') out << "&lt;";
		else if(c == '>') out << "&gt;";
		else if(c == '&') out << "&amp;";
		else out << c;
	}
	return out;
}

bool VsmHtmlReportWriter::WriteHtml(const VsmSession& session, const String& out_dir)
{
	RealizeDirectory(out_dir);

	LogInfo(log_, "VsmHtmlReport", "Writing HTML report to: " + out_dir);

	String html;
	html << "<!DOCTYPE html>\n"
	     << "<html lang=\"en\">\n"
	     << "<head>\n"
	     << "  <meta charset=\"UTF-8\">\n"
	     << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
	     << "  <title>VisualStateModel Replay Report</title>\n"
	     << "</head>\n"
	     << "<body>\n";

	// Title
	html << "<h1>VisualStateModel Replay Report</h1>\n\n";

	// Session metadata
	html << "<h2>Session Information</h2>\n";
	html << "<p>\n";
	html << "  <strong>Session:</strong> <code>" << EscapeHtml(session.session_id) << "</code><br>\n";
	html << "  <strong>Source:</strong> " << EscapeHtml(session.source_type) << "<br>\n";
	html << "  <strong>Size:</strong> " << session.frame_width << "×" << session.frame_height << "<br>\n";
	html << "  <strong>Started:</strong> " << EscapeHtml(session.started_at) << "<br>\n";
	html << "  <strong>Ended:</strong> " << (session.ended_at.IsEmpty() ? String("—") : EscapeHtml(session.ended_at)) << "\n";
	html << "</p>\n\n";

	// Load warnings
	if(!session.load_warnings.IsEmpty()) {
		html << "<h2>Load Warnings</h2>\n";
		html << "<ul>\n";
		for(const String& w : session.load_warnings)
			html << "  <li>" << EscapeHtml(w) << "</li>\n";
		html << "</ul>\n\n";
	}

	// Session statistics
	html << "<h2>Session Statistics</h2>\n";
	html << "<table border=\"1\" cellpadding=\"5\" cellspacing=\"0\">\n";
	html << "  <tr>\n";
	html << "    <th>Item</th>\n";
	html << "    <th>Count</th>\n";
	html << "  </tr>\n";
	html << "  <tr><td>Frames</td><td>" << session.frames.GetCount() << "</td></tr>\n";
	html << "  <tr><td>Change events</td><td>" << session.changes.GetCount() << "</td></tr>\n";
	html << "  <tr><td>Regions</td><td>" << session.regions.GetCount() << "</td></tr>\n";
	html << "  <tr><td>OCR results</td><td>" << session.ocr_results.GetCount() << "</td></tr>\n";
	html << "  <tr><td>Template matches</td><td>" << session.template_results.GetCount() << "</td></tr>\n";
	html << "  <tr><td>State snapshots</td><td>" << session.state_snapshots.GetCount() << "</td></tr>\n";
	html << "  <tr><td>Divergences</td><td>" << session.divergences.GetCount() << "</td></tr>\n";
	html << "</table>\n\n";

	// Pipeline divergences from divergences.json (same as Markdown writer)
	{
		String div_json_path = AppendFileName(out_dir, "divergences.json");
		if(FileExists(div_json_path)) {
			String raw = LoadFile(div_json_path);
			Vector<VsmDivergence> pdivs;
			if(LoadFromJson(pdivs, raw) && !pdivs.IsEmpty()) {
				html << "<h2>Divergences</h2>\n";
				html << "<table border=\"1\" cellpadding=\"5\" cellspacing=\"0\">\n";
				html << "  <tr>\n";
				html << "    <th>Frame</th>\n";
				html << "    <th>Severity</th>\n";
				html << "    <th>Region</th>\n";
				html << "    <th>Message</th>\n";
				html << "  </tr>\n";
				for(const VsmDivergence& d : pdivs) {
					html << "  <tr>\n";
					html << "    <td>" << d.frame << "</td>\n";
					html << "    <td>" << EscapeHtml(d.severity) << "</td>\n";
					html << "    <td>" << (d.region_id.IsEmpty() ? String("—") : EscapeHtml(d.region_id)) << "</td>\n";
					html << "    <td>" << EscapeHtml(d.message) << "</td>\n";
					html << "  </tr>\n";
				}
				html << "</table>\n\n";
			}
		}
	}

	html << "</body>\n"
	     << "</html>\n";

	String path = AppendFileName(out_dir, "index.html");
	if(!SaveFile(path, html)) {
		LogError(log_, "VsmHtmlReport", "Cannot write index.html to: " + path);
		return false;
	}
	LogInfo(log_, "VsmHtmlReport", "Written: " + path);
	return true;
}
