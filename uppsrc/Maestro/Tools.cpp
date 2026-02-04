#include "Maestro.h"

NAMESPACE_UPP

String FindPlanRoot()
{
	String d = GetCurrentDirectory();
	while(d.GetCount() > 1) {
		String p = AppendFileName(AppendFileName(AppendFileName(d, "uppsrc"), "AI"), "plan");
		if(DirectoryExists(p))
			return NormalizePath(p);
		d = GetFileDirectory(d);
	}
	return "";
}

String GetDocsRoot(const String& plan_root)
{
	if(plan_root.IsEmpty()) return "";
	return NormalizePath(AppendFileName(plan_root, "../../.."));
}

class LambdaMaestroTool : public MaestroTool {
	String name;
	String desc;
	Function<Value(const ValueMap&)> exec;
public:
	LambdaMaestroTool(String n, String d, Function<Value(const ValueMap&)> e) 
		: name(n), desc(d), exec(e) {}
		
	virtual String GetName() const override { return name; }
	virtual String GetDescription() const override { return desc; }
	virtual Value  GetSchema() const override { return ValueMap(); } // Schema not yet supported/needed for this internal registry
	virtual Value  Execute(const ValueMap& params) const override { return exec(params); }
};

Value MaestroUpdateTaskStatus(const ValueMap& params) {
	String track = params["track"];
	String phase = params["phase"];
	String task = params["task"];
	String status_str = params["status"];
	
	if(track.IsEmpty() || phase.IsEmpty() || task.IsEmpty() || status_str.IsEmpty())
		return "Error: Missing required parameters (track, phase, task, status).";
		
	TaskStatus status = StringToStatus(ToLower(status_str));
	if(status == STATUS_UNKNOWN)
		return "Error: Invalid status. Use 'todo', 'in_progress', 'done', or 'blocked'.";
		
	PlanParser pp;
	// Use current directory as root, assuming running from project root
	if(pp.UpdateTaskStatus(GetCurrentDirectory(), track, phase, task, status))
		return "Success: Task status updated.";
	else
		return "Error: Failed to update task status. File not found or write error.";
}

void RegisterMaestroTools(MaestroToolRegistry& reg) {
	reg.Add(new LambdaMaestroTool("update_task_status", "Update the status of a project plan task. Params: track, phase, task, status.", 
		[](const ValueMap& params) -> Value {
			return MaestroUpdateTaskStatus(params);
		}
	));
}

END_UPP_NAMESPACE
