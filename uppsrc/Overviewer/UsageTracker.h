#ifndef _Overviewer_UsageTracker_h_
#define _Overviewer_UsageTracker_h_

#include <Core/Core.h>

using namespace Upp;

struct UsageEvent : Moveable<UsageEvent> {
	Time timestamp;
	String actor_id;
	String actor_type;
	String session_id;
	String action_type;
	String target;
	int duration_ms = 0;
	bool success = true;

	void Jsonize(JsonIO& jio) {
		jio("timestamp", timestamp)("actor_id", actor_id)("actor_type", actor_type)
		   ("session_id", session_id)("action_type", action_type)("target", target)
		   ("duration_ms", duration_ms)("success", success);
	}
	
	UsageEvent() {}
	UsageEvent(const UsageEvent& s, int) {
		timestamp = s.timestamp; actor_id = s.actor_id; actor_type = s.actor_type;
		session_id = s.session_id; action_type = s.action_type; target = s.target;
		duration_ms = s.duration_ms; success = s.success;
	}
};

struct FrictionSignal : Moveable<FrictionSignal> {
	String type;
	String description;
	int severity = 0; // 0: low, 1: medium, 2: high
	Vector<String> related_targets;

	void Jsonize(JsonIO& jio) {
		jio("type", type)("description", description)("severity", severity)
		   ("related_targets", related_targets);
	}
	
	FrictionSignal() {}
	FrictionSignal(const FrictionSignal& s, int) {
		type = s.type; description = s.description; severity = s.severity;
		related_targets <<= s.related_targets;
	}
};

struct UsageSummary : Moveable<UsageSummary> {
	int total_actions = 0;
	int sessions_count = 0;
	VectorMap<String, int> top_actions;
	VectorMap<String, int> top_targets;
	Vector<String> unused_features;

	void Jsonize(JsonIO& jio) {
		jio("total_actions", total_actions)("sessions_count", sessions_count)
		   ("top_actions", top_actions)("top_targets", top_targets)
		   ("unused_features", unused_features);
	}
};

struct OverviewerProject;

class UsageTracker {
public:
	static UsageSummary GetSummary(const OverviewerProject& p);
	static Vector<FrictionSignal> GetFriction(const OverviewerProject& p);
};

#endif
