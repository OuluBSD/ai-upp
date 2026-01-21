#ifndef _Maestro_PlanModels_h_
#define _Maestro_PlanModels_h_

#include <Core/Core.h>

NAMESPACE_UPP

// --- Track / Phase / Task System ---

enum TaskStatus {
	STATUS_TODO,
	STATUS_IN_PROGRESS,
	STATUS_DONE,
	STATUS_BLOCKED
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
	return STATUS_TODO;
}

struct Task : Moveable<Task> {
	String     id;
	String     name;
	String     description;
	TaskStatus status = STATUS_TODO;
	String     path;
	String     priority = "P2";
	bool       completed = false;
	Time       created_at;
	Time       updated_at;
	Vector<String> tags;
	Vector<String> dependencies;

	void Jsonize(JsonIO& jio) {
		String s = StatusToString(status);
		jio("id", id)("name", name)("description", description)("status", s)("path", path)
		   ("priority", priority)("completed", completed)("created_at", created_at)
		   ("updated_at", updated_at)("tags", tags)("dependencies", dependencies);
		if(jio.IsLoading()) status = StringToStatus(s);
	}
};

struct Phase : Moveable<Phase> {
	String      id;
	String      name;
	String      path;
	String      status = "planned";
	int         completion = 0;
	Array<Task> tasks;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("name", name)("path", path)("status", status)("completion", completion)("tasks", tasks);
	}
};

struct Track : Moveable<Track> {
	String       id;
	String       name;
	String       path;
	String       status = "planned";
	int          completion = 0;
	Array<Phase> phases;
	bool         is_top_priority = false;

	void Jsonize(JsonIO& jio) {
		jio("track_id", id)("name", name)("path", path)("status", status)("completion", completion)
		   ("phases", phases)("is_top_priority", is_top_priority);
	}
};

// --- Runbook System ---

struct RunbookStep : Moveable<RunbookStep> {
	int    n = 0;
	String actor;
	String action;
	String expected;
	String command;
	String details;
	Vector<String> variants;

	void Jsonize(JsonIO& jio) {
		jio("n", n)("actor", actor)("action", action)("expected", expected)
		   ("command", command)("details", details)("variants", variants);
	}
};

struct Runbook : Moveable<Runbook> {
	String id;
	String title;
	String goal;
	Vector<String> prerequisites;
	Array<RunbookStep> steps;
	Vector<String> invariants;
	Vector<String> tags;
	Time   created_at;
	Time   updated_at;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("title", title)("goal", goal)("prerequisites", prerequisites)
		   ("steps", steps)("invariants", invariants)("tags", tags)
		   ("created_at", created_at)("updated_at", updated_at);
	}
};

// --- WorkGraph (Workflow) System ---

struct DefinitionOfDone : Moveable<DefinitionOfDone> {
	String kind;
	String cmd;
	String path;
	String expect;

	void Jsonize(JsonIO& jio) {
		jio("kind", kind)("cmd", cmd)("path", path)("expect", expect);
	}
};

struct WorkTask : Moveable<WorkTask> {
	String id;
	String title;
	String intent;
	Array<DefinitionOfDone> definition_of_done;
	Array<DefinitionOfDone> verification;
	Vector<String> inputs;
	Vector<String> outputs;
	bool   safe_to_execute = false;
	Vector<String> depends_on;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("title", title)("intent", intent)
		   ("definition_of_done", definition_of_done)("verification", verification)
		   ("inputs", inputs)("outputs", outputs)("safe_to_execute", safe_to_execute)
		   ("depends_on", depends_on);
	}
};

struct WorkPhase : Moveable<WorkPhase> {
	String id;
	String name;
	Array<WorkTask> tasks;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("name", name)("tasks", tasks);
	}
};

struct WorkGraphTrack : Moveable<WorkGraphTrack> {
	String id;
	String name;
	String goal;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("name", name)("goal", goal);
	}
};

struct WorkGraph : Moveable<WorkGraph> {
	String id;
	String title;
	String goal;
	String domain;
	String profile;
	WorkGraphTrack track;
	Array<WorkPhase> phases;
	Time   created_at;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("title", title)("goal", goal)("domain", domain)("profile", profile)
		   ("track", track)("phases", phases)("created_at", created_at);
	}
};

END_UPP_NAMESPACE

#endif