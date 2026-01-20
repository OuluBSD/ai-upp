#ifndef _Maestro_PlanModels_h_
#define _Maestro_PlanModels_h_

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

#endif