#include "Maestro.h"

NAMESPACE_UPP

String FindPlanRoot()
{
	String d = GetCurrentDirectory();
	while(d.GetCount() > 1) {
		String p = AppendFileName(d, "docs/maestro");
		if(DirectoryExists(p))
			return NormalizePath(d);
		d = GetFileDirectory(d);
	}
	return "";
}

String GetDocsRoot(const String& plan_root)
{
	if(plan_root.IsEmpty()) return "";
	String docs = NormalizePath(AppendFileName(plan_root, "docs"));
	if(DirectoryExists(docs)) return plan_root;
	return plan_root; // Fallback to plan_root itself as repo root
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

Value MaestroListDirectory(const ValueMap& params) {
	String path = params["dir_path"];
	if(path.IsEmpty()) path = GetCurrentDirectory();
	
	ValueArray va;
	FindFile ff(AppendFileName(path, "*"));
	while(ff) {
		ValueMap m;
		m.Add("name", ff.GetName());
		m.Add("is_directory", ff.IsDirectory());
		m.Add("is_file", ff.IsFile());
		m.Add("size", (int64)ff.GetLength());
		va.Add(m);
		ff.Next();
	}
	return va;
}

Value MaestroReadFile(const ValueMap& params) {
	String path = params["file_path"];
	if(path.IsEmpty()) return "Error: file_path is required.";
	String content = LoadFile(path);
	if(content.IsVoid()) return "Error: Could not read file.";
	return content;
}

Value MaestroWriteFile(const ValueMap& params) {
	String path = params["file_path"];
	String content = params["content"];
	if(path.IsEmpty()) return "Error: file_path is required.";
	if(SaveFile(path, content)) return "Success: File written.";
	return "Error: Could not write file.";
}

Value MaestroReplace(const ValueMap& params) {
	String path = params["file_path"];
	String old_text = params["old_string"];
	String new_text = params["new_string"];
	if(path.IsEmpty() || old_text.IsEmpty()) return "Error: file_path and old_string are required.";
	
	String content = LoadFile(path);
	if(content.IsVoid()) return "Error: Could not read file.";
	
	int count = 0;
	int pos = content.Find(old_text);
	while(pos >= 0) {
		content.Replace(old_text, new_text);
		count++;
		pos = content.Find(old_text, pos + new_text.GetCount());
	}
	
	if(count == 0) return "Error: old_string not found.";
	if(SaveFile(path, content)) return Format("Success: Replaced %d occurrences.", count);
	return "Error: Could not write file.";
}

void RegisterMaestroTools(MaestroToolRegistry& reg) {
	Cout() << "Registering Maestro Tools...\n";
	RegisterUxTools(reg);
	
	reg.Add(new LambdaMaestroTool("update_task_status", "Update the status of a project plan task. Params: track, phase, task, status.", 
		[](const ValueMap& params) -> Value { return MaestroUpdateTaskStatus(params); }));

	reg.Add(new LambdaMaestroTool("list_directory", "List contents of a directory. Params: dir_path.", 
		[](const ValueMap& params) -> Value { return MaestroListDirectory(params); }));

	reg.Add(new LambdaMaestroTool("read_file", "Read content of a file. Params: file_path.", 
		[](const ValueMap& params) -> Value { return MaestroReadFile(params); }));

	reg.Add(new LambdaMaestroTool("write_file", "Write content to a file. Params: file_path, content.", 
		[](const ValueMap& params) -> Value { return MaestroWriteFile(params); }));

	reg.Add(new LambdaMaestroTool("replace", "Replace text in a file. Params: file_path, old_string, new_string.", 
		[](const ValueMap& params) -> Value { return MaestroReplace(params); }));

	for(int i = 0; i < reg.GetTools().GetCount(); i++)
		Cout() << "Registered: " << reg.GetTools().GetKey(i) << "\n";
	Cout() << "Total tools: " << reg.GetTools().GetCount() << "\n";
}

END_UPP_NAMESPACE
