#ifndef _Maestro_PlanModels_h_
#define _Maestro_PlanModels_h_


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

	Task() {}
	Task(const Task& t) {
		id = t.id; name = t.name; description = t.description; status = t.status;
		path = t.path; priority = t.priority; depends_on = clone(t.depends_on);
		created_at = t.created_at; updated_at = t.updated_at;
	}
};

struct Phase : Moveable<Phase> {
	String      id;
	String      name;
	String      status; // "planned", "active", "done"
	int         completion = 0;
	Array<Task> tasks;
	String      path; // Directory path

	Phase() {}
	Phase(const Phase& p) {
		id = p.id; name = p.name; status = p.status; completion = p.completion;
		tasks = clone(p.tasks); path = p.path;
	}
};

struct Track : Moveable<Track> {
	String       id;
	String       name;
	String       status;
	int          completion = 0;
	Array<Phase> phases;
	String       path; // Directory path

	Track() {}
	Track(const Track& t) {
		id = t.id; name = t.name; status = t.status; completion = t.completion;
		phases = clone(t.phases); path = t.path;
	}
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
	Runbook() {}
	Runbook(const Runbook& r) {
		id = r.id; title = r.title; goal = r.goal; steps = clone(r.steps);
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
	WorkGraphTask() {}
	WorkGraphTask(const WorkGraphTask& t) {
		id = t.id; title = t.title; status = t.status; intent = t.intent;
		definition_of_done = clone(t.definition_of_done);
		verification = clone(t.verification);
		inputs = clone(t.inputs); outputs = clone(t.outputs);
		risk = clone(t.risk); depends_on = clone(t.depends_on); tags = clone(t.tags);
		effort = clone(t.effort); impact = t.impact; risk_score = t.risk_score;
		purpose = t.purpose; safe_to_execute = t.safe_to_execute;
	}
};

struct WorkGraphPhase : Moveable<WorkGraphPhase> {
	String id;
	String name;
	Array<WorkGraphTask> tasks;
	
	void Jsonize(JsonIO& jio) {
		jio("id", id)("name", name)("tasks", tasks);
	}
	WorkGraphPhase() {}
	WorkGraphPhase(const WorkGraphPhase& p) {
		id = p.id; name = p.name; tasks = clone(p.tasks);
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
	WorkGraph() {}
	WorkGraph(const WorkGraph& w) {
		schema_version = w.schema_version; id = w.id; title = w.title;
		goal = w.goal; domain = w.domain; profile = w.profile;
		track = w.track; phases = clone(w.phases);
		stop_conditions = clone(w.stop_conditions);
		repo_discovery = clone(w.repo_discovery);
	}
};


#endif

