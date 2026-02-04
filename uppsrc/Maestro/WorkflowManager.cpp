#include "Maestro.h"

namespace Upp {

WorkflowManager::WorkflowManager(const String& maestro_root)
{
	base_path = NormalizePath(maestro_root);
	workflows_dir = AppendFileName(base_path, "docs/workflows");
	RealizeDirectory(workflows_dir);
}

Vector<String> WorkflowManager::ListWorkflows()
{
	Vector<String> list;
	FindFile ff(AppendFileName(workflows_dir, "*.puml"));
	while(ff) {
		list.Add(ff.GetName());
		ff.Next();
	}
	return list;
}

String WorkflowManager::LoadWorkflow(const String& name)
{
	String path = AppendFileName(workflows_dir, name);
	if(!FileExists(path) && !name.EndsWith(".puml")) path << ".puml";
	return LoadFile(path);
}

bool WorkflowManager::SaveWorkflow(const String& name, const String& content)
{
	String path = AppendFileName(workflows_dir, name);
	if(!name.EndsWith(".puml")) path << ".puml";
	return SaveFile(path, content);
}

bool WorkflowManager::DeleteWorkflow(const String& name)
{
	String path = AppendFileName(workflows_dir, name);
	if(!FileExists(path) && !name.EndsWith(".puml")) path << ".puml";
	if(FileExists(path)) return DeleteFile(path);
	return false;
}

String WorkflowManager::Visualize(const String& name, const String& format)
{
	// Stub implementation: Return the path to the potential output
	// In a real implementation, this would call plantuml or graphviz
	String path = AppendFileName(workflows_dir, name);
	if(!name.EndsWith(".puml")) path << ".puml";
	
	if(!FileExists(path)) return "Error: Workflow not found.";
	
	if(format == "plantuml") return LoadFile(path);
	
	// Stub for other formats
	return "Visualization for " + name + " in " + format + " is not yet implemented.";
}

}
