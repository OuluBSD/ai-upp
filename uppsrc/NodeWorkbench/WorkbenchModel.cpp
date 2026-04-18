#include "WorkbenchModel.h"

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// WorkbenchGraph
// ---------------------------------------------------------------------------

void WorkbenchGraph::Jsonize(JsonIO& jio) {
	jio("version",     version)
	   ("name",        name)
	   ("description", description)
	   ("metadata",    metadata);
}

bool WorkbenchGraph::Load(const String& path) {
	String json = LoadFile(path);
	if(json.IsEmpty()) return false;
	Value root = ParseJSON(json);
	if(IsNull(root) || !IsValueMap(root)) return false;
	WorkbenchGraph tmp;
	LoadFromJsonValue(tmp, root);
	if(tmp.version <= 0) tmp.version = 1;
	*this = pick(tmp);
	return true;
}

bool WorkbenchGraph::Save(const String& path) const {
	ValueMap root;
	root.Add("version",     version);
	root.Add("name",        name);
	root.Add("description", description);
	root.Add("metadata",    metadata);
	return SaveFile(path, StoreAsJson(root, true));
}

// ---------------------------------------------------------------------------
// WorkbenchProject
// ---------------------------------------------------------------------------

void WorkbenchProject::Jsonize(JsonIO& jio) {
	jio("version",       version)
	   ("name",          name)
	   ("graphs",        graphs)
	   ("startup_graph", startup_graph)
	   ("metadata",      metadata);
}

bool WorkbenchProject::Load(const String& path) {
	String json = LoadFile(path);
	if(json.IsEmpty()) return false;
	Value root = ParseJSON(json);
	if(IsNull(root) || !IsValueMap(root)) return false;
	WorkbenchProject tmp;
	LoadFromJsonValue(tmp, root);
	if(tmp.version <= 0) tmp.version = 1;
	if(tmp.version > 1) return false;
	*this = pick(tmp);
	return true;
}

bool WorkbenchProject::Save(const String& path) const {
	ValueMap root;
	ValueArray graphs_arr;
	for(const String& g : graphs) graphs_arr.Add(g);
	root.Add("version",       version);
	root.Add("name",          name);
	root.Add("graphs",        graphs_arr);
	root.Add("startup_graph", startup_graph);
	root.Add("metadata",      metadata);
	return SaveFile(path, StoreAsJson(root, true));
}

// ---------------------------------------------------------------------------
// WorkbenchSolution
// ---------------------------------------------------------------------------

void WorkbenchSolution::Jsonize(JsonIO& jio) {
	jio("version",        version)
	   ("name",           name)
	   ("projects",       projects)
	   ("active_project", active_project)
	   ("metadata",       metadata);
}

bool WorkbenchSolution::Load(const String& path) {
	String json = LoadFile(path);
	if(json.IsEmpty()) return false;
	Value root = ParseJSON(json);
	if(IsNull(root) || !IsValueMap(root)) return false;
	WorkbenchSolution tmp;
	LoadFromJsonValue(tmp, root);
	if(tmp.version <= 0) tmp.version = 1;
	if(tmp.version > 1) return false;
	*this = pick(tmp);
	return true;
}

bool WorkbenchSolution::Save(const String& path) const {
	ValueMap root;
	ValueArray projects_arr;
	for(const String& p : projects) projects_arr.Add(p);
	root.Add("version",        version);
	root.Add("name",           name);
	root.Add("projects",       projects_arr);
	root.Add("active_project", active_project);
	root.Add("metadata",       metadata);
	return SaveFile(path, StoreAsJson(root, true));
}

// ---------------------------------------------------------------------------
// WorkbenchExtensions
// ---------------------------------------------------------------------------

String WorkbenchExtensions::KindFromPath(const String& path) {
	String ext = ToLower(GetFileExt(path));
	if(ext == ".sln" || ext == ".slnx" || ext == ".nnsln") return "solution";
	if(ext == ".grfproj" || ext == ".nnprj")               return "project";
	if(ext == ".grf"     || ext == ".nngrf")               return "graph";
	if(ext == ".nnpy")                                      return "script";
	return "file";
}

bool WorkbenchExtensions::IsKnownKind(const String& ext) {
	String e = ToLower(ext);
	return e == ".sln"  || e == ".slnx"   || e == ".nnsln"  ||
	       e == ".grfproj" || e == ".nnprj"  ||
	       e == ".grf"  || e == ".nngrf"   ||
	       e == ".nnpy";
}

END_UPP_NAMESPACE
