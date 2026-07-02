#include "ReportWriter.h"

bool VsmReportWriter::Write(const VsmSession& session, const String& out_dir)
{
	RealizeDirectory(out_dir);
	String events_dir = AppendFileName(out_dir, "events");
	RealizeDirectory(events_dir);

	LogInfo(log_, "VsmReport", "Writing report to: " + out_dir);

	if(!WriteIndex(session, out_dir)) return false;

	// Write one page per change event
	int seq = 0;
	for(const VsmChangeEvent& ce : session.changes) {
		if(!WriteEventPage(ce, ++seq, events_dir)) return false;
	}

	// Write one page per divergence
	for(const VsmDivergence& div : session.divergences) {
		if(!WriteDivergencePage(div, ++seq, events_dir)) return false;
	}

	LogInfo(log_, "VsmReport", Format("Report complete: %d event pages written", seq));
	return true;
}

bool VsmReportWriter::WriteIndex(const VsmSession& session, const String& out_dir)
{
	String md;
	md << "# VisualStateModel Replay Report\n\n";
	md << "**Session:** `" << session.session_id << "`  \n";
	md << "**Source:** " << session.source_type << "  \n";
	md << "**Size:** " << session.frame_width << "×" << session.frame_height << "  \n";
	md << "**Started:** " << session.started_at << "  \n";
	md << "**Ended:** " << (session.ended_at.IsEmpty() ? "—" : session.ended_at) << "  \n\n";

	// Divergence summary — most prominent section
	if(!session.divergences.IsEmpty()) {
		md << "## ⚠ Divergences (" << session.divergences.GetCount() << ")\n\n";
		for(const VsmDivergence& div : session.divergences) {
			md << "- " << SeverityBadge(div.severity)
			   << " **Frame " << div.frame << ":** " << div.message << "  \n"
			   << "  Expected: `" << div.expected_json << "`  \n"
			   << "  Observed: `" << div.observed_json << "`\n\n";
		}
	} else {
		md << "## ✓ No divergences\n\n";
	}

	// Load warnings
	if(!session.load_warnings.IsEmpty()) {
		md << "## Load Warnings\n\n";
		for(const String& w : session.load_warnings)
			md << "- " << w << "\n";
		md << "\n";
	}

	// Stats
	md << "## Session Statistics\n\n";
	md << "| Item | Count |\n|------|-------|\n";
	md << "| Frames | " << session.frames.GetCount() << " |\n";
	md << "| Change events | " << session.changes.GetCount() << " |\n";
	md << "| Regions | " << session.regions.GetCount() << " |\n";
	md << "| OCR results | " << session.ocr_results.GetCount() << " |\n";
	md << "| Template matches | " << session.template_results.GetCount() << " |\n";
	md << "| State snapshots | " << session.state_snapshots.GetCount() << " |\n";
	md << "| Divergences | " << session.divergences.GetCount() << " |\n\n";

	// Pipeline divergences from divergences.json (auto-saved by pipeline runs)
	{
		String div_json_path = AppendFileName(out_dir, "divergences.json");
		if(FileExists(div_json_path)) {
			String raw = LoadFile(div_json_path);
			Vector<VsmDivergence> pdivs;
			if(LoadFromJson(pdivs, raw) && !pdivs.IsEmpty()) {
				md << "## Divergences (" << pdivs.GetCount() << ")\n\n";
				md << "| Frame | Severity | Region | Message |\n|---|---|---|---|\n";
				for(const VsmDivergence& d : pdivs)
					md << "| " << d.frame << " | " << d.severity << " | "
					   << (d.region_id.IsEmpty() ? String("—") : d.region_id)
					   << " | " << d.message << " |\n";
				md << "\n";
			}
		}
	}

	// Event index
	if(!session.changes.IsEmpty()) {
		md << "## Change Events\n\n";
		int seq = 0;
		for(const VsmChangeEvent& ce : session.changes) {
			seq++;
			String page = Format("events/%06d.md", seq);
			md << Format("- [Frame %d — %d region(s) changed](%s)\n",
			             ce.frame, ce.regions.GetCount(), page);
		}
		md << "\n";
	}

	// Divergence index
	int base_seq = session.changes.GetCount();
	if(!session.divergences.IsEmpty()) {
		md << "## Divergence Events\n\n";
		int seq = 0;
		for(const VsmDivergence& div : session.divergences) {
			seq++;
			String page = Format("events/%06d.md", base_seq + seq);
			md << Format("- [Frame %d — %s](%s): %s\n",
			             div.frame, div.severity, page, div.message);
		}
		md << "\n";
	}

	// Regions
	if(!session.regions.IsEmpty()) {
		md << "## Regions\n\n";
		md << "| ID | Frame | Action | Rect | Fingerprint |\n"
		      "|-----|-------|--------|------|-------------|\n";
		for(const VsmRegionNode& rn : session.regions) {
			String rect = Format("(%d,%d) ", rn.x, rn.y) + IntStr(rn.w) + "x" + IntStr(rn.h);
			String fp   = rn.fingerprint.hash.IsEmpty() ? "—" : rn.fingerprint.hash;
			md << Format("| `%s` | %d | %s | %s | `%s` |\n",
			             rn.id, rn.frame,
			             rn.action.IsEmpty() ? "—" : rn.action,
			             rect, fp);
		}
		md << "\n";
	}

	String path = AppendFileName(out_dir, "index.md");
	if(!SaveFile(path, md)) {
		LogError(log_, "VsmReport", "Cannot write index.md to: " + path);
		return false;
	}
	LogInfo(log_, "VsmReport", "Written: " + path);
	return true;
}

bool VsmReportWriter::WriteEventPage(const VsmChangeEvent& ce, int seq,
                                      const String& events_dir)
{
	String md;
	md << "# Change Event — Frame " << ce.frame << "\n\n";
	md << "**Timestamp:** " << ce.ts << "  \n";
	md << "**Changed regions:** " << ce.regions.GetCount() << "\n\n";
	md << RegionTable(ce.regions);

	String path = AppendFileName(events_dir, Format("%06d.md", seq));
	if(!SaveFile(path, md)) {
		LogError(log_, "VsmReport", "Cannot write event page: " + path);
		return false;
	}
	return true;
}

bool VsmReportWriter::WriteDivergencePage(const VsmDivergence& div, int seq,
                                           const String& events_dir)
{
	String md;
	md << "# Divergence — Frame " << div.frame << "\n\n";
	md << "**Severity:** " << SeverityBadge(div.severity) << "  \n";
	md << "**Timestamp:** " << div.ts << "  \n";
	md << "**Region:** " << (div.region_id.IsEmpty() ? "—" : div.region_id) << "  \n\n";
	md << "## Message\n\n" << div.message << "\n\n";
	md << "## Expected State\n\n```json\n" << div.expected_json << "\n```\n\n";
	md << "## Observed State\n\n```json\n" << div.observed_json << "\n```\n\n";
	md << "> **Image crops:** not yet available (image pipeline placeholder)\n";

	String path = AppendFileName(events_dir, Format("%06d.md", seq));
	if(!SaveFile(path, md)) {
		LogError(log_, "VsmReport", "Cannot write divergence page: " + path);
		return false;
	}
	return true;
}

String VsmReportWriter::SeverityBadge(const String& sev)
{
	if(sev == "fatal")   return "🔴 FATAL";
	if(sev == "error")   return "🔴 ERROR";
	if(sev == "warning") return "🟡 WARNING";
	return sev;
}

String VsmReportWriter::RegionTable(const Vector<VsmChangedRect>& regions)
{
	if(regions.IsEmpty()) return "_No changed regions._\n\n";
	String md;
	md << "| # | X | Y | W | H | Score |\n"
	      "|---|---|---|---|---|-------|\n";
	for(int i = 0; i < regions.GetCount(); i++) {
		const VsmChangedRect& r = regions[i];
		md << Format("| %d | %d | %d | %d | %d | %.1f%% |\n",
		             i + 1, r.x, r.y, r.w, r.h, r.score * 100.0);
	}
	md << "\n> Image crops: external file placeholder — not yet implemented.\n\n";
	return md;
}
