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
	
	void Jsonize(JsonIO& jio) {
		jio("kind", kind)("cmd", cmd)("path", path);
	}
};

struct WorkGraphTask : Moveable<WorkGraphTask> {
	String title;
	String status;
	String intent;
	Array<WorkGraphDefinitionOfDone> definition_of_done;
	Vector<String> depends_on;
	
	void Jsonize(JsonIO& jio) {
		jio("title", title)("status", status)("intent", intent)
		   ("definition_of_done", definition_of_done)("depends_on", depends_on);
	}
};

struct WorkGraphPhase : Moveable<WorkGraphPhase> {
	String name;
	Array<WorkGraphTask> tasks;
	
	void Jsonize(JsonIO& jio) {
		jio("name", name)("tasks", tasks);
	}
};

struct WorkGraphTrack : Moveable<WorkGraphTrack> {
	String name;
	String goal;
	String status;
	
	void Jsonize(JsonIO& jio) {
		jio("name", name)("goal", goal)("status", status);
	}
};

struct WorkGraph : Moveable<WorkGraph> {
	String id;
	String title;
	String goal;
	String domain;
	String profile;
	WorkGraphTrack track;
	Array<WorkGraphPhase> phases;
	
	void Jsonize(JsonIO& jio) {
		jio("id", id)("title", title)("goal", goal)("domain", domain)("profile", profile)
		   ("track", track)("phases", phases);
	}
};

END_UPP_NAMESPACE

#endif
