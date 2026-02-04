#ifndef MAESTRO_PLANMODELS_H
#define MAESTRO_PLANMODELS_H

// NOTE: This header is normally included inside namespace Upp

#ifndef _Maestro_PlanModels_h_
#define _Maestro_PlanModels_h_

#include <Core/Core.h>

NAMESPACE_UPP

enum TaskStatus {
	STATUS_TODO,
	STATUS_IN_PROGRESS,
	STATUS_DONE,
	STATUS_BLOCKED,
	STATUS_UNKNOWN
};

inline String StatusToString(TaskStatus s) {
	switch(s) {
		case STATUS_TODO: return "todo";
		case STATUS_IN_PROGRESS: return "in_progress";
		case STATUS_DONE: return "done";
		case STATUS_BLOCKED: return "blocked";
		default: return "unknown";
	}
}

inline TaskStatus StringToStatus(const String& s) {
	if(s == "todo") return STATUS_TODO;
	if(s == "in_progress") return STATUS_IN_PROGRESS;
	if(s == "done") return STATUS_DONE;
	if(s == "blocked") return STATUS_BLOCKED;
	return STATUS_UNKNOWN;
}

struct Task : Moveable<Task> {
	String     id;
	String     name;
	String     description;
	TaskStatus status = STATUS_TODO;
	String     path; // Path to task file
	String     priority = "P2";
	Vector<String> depends_on;
	
	Time       created_at;
	Time       updated_at;
};

struct Phase : Moveable<Phase> {
	String      id;
	String      name;
	String      status; // "planned", "active", "done"
	int         completion = 0;
	Array<Task> tasks;
	String      path; // Directory path
};

struct Track : Moveable<Track> {
	String       id;
	String       name;
	String       status;
	int          completion = 0;
	Array<Phase> phases;
	String       path; // Directory path
};

struct RunbookStep : Moveable<RunbookStep> {
	int    n;
	String actor;
	String action;
	String command;
	String expected;
	
	void Jsonize(JsonIO& jio) {
		jio("n", n)("actor", actor)("action", action)("command", command)("expected", expected);
	}
};

struct Runbook : Moveable<Runbook> {
	String id;
	String title;
	String goal;
	Array<RunbookStep> steps;
	
	void Jsonize(JsonIO& jio) {
		jio("id", id)("title", title)("goal", goal)("steps", steps);
	}
};

struct WorkGraphDefinitionOfDone : Moveable<WorkGraphDefinitionOfDone> {
	String kind;
	String cmd;
	String path;
	String expect;
	
	void Jsonize(JsonIO& jio) {
		jio("kind", kind)("cmd", cmd)("path", path)("expect", expect);
	}
};

struct WorkGraphTask : Moveable<WorkGraphTask> {
	String id;
	String title;
	String status;
	String intent;
	Array<WorkGraphDefinitionOfDone> definition_of_done;
	Array<WorkGraphDefinitionOfDone> verification;
	Vector<String> inputs;
	Vector<String> outputs;
	ValueMap       risk;
	Vector<String> depends_on;
	Vector<String> tags;
	
	ValueMap       effort; // {min, max}
	Value          impact;
	Value          risk_score;
	Value          purpose;
	bool           safe_to_execute = false;
	
	void Jsonize(JsonIO& jio) {
		jio("id", id)("title", title)("status", status)("intent", intent)
		   ("definition_of_done", definition_of_done)("verification", verification)
		   ("inputs", inputs)("outputs", outputs)("risk", risk)("depends_on", depends_on)
		   ("tags", tags)("effort", effort)("impact", impact)("risk_score", risk_score)
		   ("purpose", purpose)("safe_to_execute", safe_to_execute);
	}
};

struct WorkGraphPhase : Moveable<WorkGraphPhase> {
	String id;
	String name;
	Array<WorkGraphTask> tasks;
	
	void Jsonize(JsonIO& jio) {
		jio("id", id)("name", name)("tasks", tasks);
	}
};

struct WorkGraphTrack : Moveable<WorkGraphTrack> {
	String id;
	String name;
	String goal;
	String status;
	
	void Jsonize(JsonIO& jio) {
		jio("id", id)("name", name)("goal", goal)("status", status);
	}
};

struct WorkGraphStopCondition : Moveable<WorkGraphStopCondition> {
	String when;
	String action;
	String notes;
	
	void Jsonize(JsonIO& jio) {
		jio("when", when)("action", action)("notes", notes);
	}
};

struct WorkGraph : Moveable<WorkGraph> {
	String schema_version = "v1";
	String id;
	String title;
	String goal;
	String domain;
	String profile;
	WorkGraphTrack track;
	Array<WorkGraphPhase> phases;
	Array<WorkGraphStopCondition> stop_conditions;
	ValueMap repo_discovery;
	
	void Jsonize(JsonIO& jio) {
		jio("schema_version", schema_version)("id", id)("title", title)("goal", goal)
		   ("domain", domain)("profile", profile)("track", track)("phases", phases)
		   ("stop_conditions", stop_conditions)("repo_discovery", repo_discovery);
	}
};

END_UPP_NAMESPACE

#endif

#endif // MAESTRO_PLANMODELS_H
