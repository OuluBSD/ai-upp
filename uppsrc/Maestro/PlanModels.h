#ifndef _Maestro_PlanModels_h_
#define _Maestro_PlanModels_h_

#include <Core/Core.h>

NAMESPACE_UPP

enum TaskStatus {
	STATUS_TODO,
	STATUS_IN_PROGRESS,
	STATUS_DONE,
	STATUS_BLOCKED
};

inline String StatusToString(TaskStatus s) {
	switch(s) {
		case STATUS_TODO: return "TODO";
		case STATUS_IN_PROGRESS: return "IN_PROGRESS";
		case STATUS_DONE: return "DONE";
		case STATUS_BLOCKED: return "BLOCKED";
		default: return "UNKNOWN";
	}
}

inline TaskStatus StringToStatus(const String& s) {
	if(s == "TODO") return STATUS_TODO;
	if(s == "IN_PROGRESS") return STATUS_IN_PROGRESS;
	if(s == "DONE") return STATUS_DONE;
	if(s == "BLOCKED") return STATUS_BLOCKED;
	return STATUS_TODO;
}

struct Task : Moveable<Task> {
	String     name;
	String     description;
	TaskStatus status = STATUS_TODO;
	String     path;

	void Jsonize(JsonIO& jio) {
		String s = StatusToString(status);
		jio("name", name)("description", description)("status", s)("path", path);
		if(jio.IsLoading()) status = StringToStatus(s);
	}
};

struct Phase : Moveable<Phase> {
	String      name;
	String      path;
	Array<Task> tasks;

	void Jsonize(JsonIO& jio) {
		jio("name", name)("path", path)("tasks", tasks);
	}
};

struct Track : Moveable<Track> {
	String       name;
	String       path;
	Array<Phase> phases;

	void Jsonize(JsonIO& jio) {
		jio("name", name)("path", path)("phases", phases);
	}
};

END_UPP_NAMESPACE

#endif