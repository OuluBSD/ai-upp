#ifndef _Overviewer_InsightEngine_h_
#define _Overviewer_InsightEngine_h_

#include <Core/Core.h>

using namespace Upp;

// Forward declaration
struct OverviewerProject;

struct Insight : Moveable<Insight> {
	String id;
	String type;
	String title;
	String description;
	int severity = 0; // 0: low, 1: medium, 2: high
	Vector<String> related_entries;
	Vector<String> supporting_evidence;
	Time timestamp_generated;
	bool dismissed = false;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("type", type)("title", title)("description", description)
		   ("severity", severity)("related_entries", related_entries)
		   ("supporting_evidence", supporting_evidence)
		   ("timestamp_generated", timestamp_generated)
		   ("dismissed", dismissed);
	}
	
	Insight() {}
	Insight(const Insight& s) {
		id = s.id; type = s.type; title = s.title; description = s.description;
		severity = s.severity;
		related_entries <<= s.related_entries;
		supporting_evidence <<= s.supporting_evidence;
		timestamp_generated = s.timestamp_generated;
		dismissed = s.dismissed;
	}
	Insight(const Insight& s, int) {
		id = s.id; type = s.type; title = s.title; description = s.description;
		severity = s.severity;
		related_entries <<= s.related_entries;
		supporting_evidence <<= s.supporting_evidence;
		timestamp_generated = s.timestamp_generated;
		dismissed = s.dismissed;
	}
};

class InsightEngine {
public:
	static Vector<Insight> Generate(const OverviewerProject& project);

private:
	static void CheckChurn(const OverviewerProject& p, Vector<Insight>& res);
	static void CheckStagnation(const OverviewerProject& p, Vector<Insight>& res);
	static void CheckDecisionDrift(const OverviewerProject& p, Vector<Insight>& res);
	static void CheckReviewAccumulation(const OverviewerProject& p, Vector<Insight>& res);
	static void CheckScenarioAbandonment(const OverviewerProject& p, Vector<Insight>& res);
	static void CheckCommentHotspots(const OverviewerProject& p, Vector<Insight>& res);
};

#endif
