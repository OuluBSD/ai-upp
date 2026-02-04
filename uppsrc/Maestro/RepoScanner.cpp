#include "Maestro.h"

namespace Upp {

void RepoScanner::Scan(const String& root) {
	packages.Clear();
	assemblies.Clear();
	Walk(root);
	DetectAssemblies();
}

void RepoScanner::Walk(const String& dir) {
	FindFile ff(AppendFileName(dir, "*"));
	while(ff) {
		String name = ff.GetName();
		if(ff.IsDirectory()) {
			if(name != "." && name != ".." && name != ".git" && name != "out") {
				Walk(ff.GetPath());
			}
		} else {
			if(name.EndsWith(".upp")) {
				AddPackage(ff.GetPath(), "upp");
			} else if(name == "CMakeLists.txt") {
				AddPackage(ff.GetPath(), "cmake");
			}
		}
		ff.Next();
	}
}

void RepoScanner::AddPackage(const String& path, const String& build_system) {
	PackageInfo& p = packages.Add();
	p.upp_path = path;
	p.dir = GetFileDirectory(path);
	p.name = GetFileTitle(p.dir);
	if(p.name.IsEmpty()) p.name = GetFileTitle(path);
	p.build_system = build_system;
	
	if(build_system == "upp") {
		UppParser parser;
		parser.ParseFile(path);
		
		for(const auto& u : parser.uses)
			p.dependencies.Add(u.package);
		
		for(const auto& fe : parser.files)
			p.files.Add(fe.path);
			
		parser.ProcessFileGroups(p.groups, p.ungrouped_files);
		
		p.metadata.Add("description", parser.description_text);
		if(!IsNull(parser.description_color))
			p.metadata.Add("color", ColorToHtml(parser.description_color));
	}
}

void RepoScanner::DetectAssemblies() {
	assemblies.Clear();
	
	VectorMap<String, Vector<int>> dir_to_pkgs;
	for(int i = 0; i < packages.GetCount(); i++) {
		String parent = GetFileDirectory(packages[i].dir);
		dir_to_pkgs.GetAdd(parent).Add(i);
	}
	
	for(int i = 0; i < dir_to_pkgs.GetCount(); i++) {
		const Vector<int>& indices = dir_to_pkgs[i];
		String dir = dir_to_pkgs.GetKey(i);
		
		AssemblyInfo& a = assemblies.Add();
		a.dir = dir;
		String n = dir;
		if(n.EndsWith("/") || n.EndsWith("\\")) n.Remove(n.GetCount() - 1);
		a.name = GetFileTitle(n);
		a.assembly_type = "upp";
		
		for(int idx : indices) {
			a.packages.Add(packages[idx].name);
			a.package_dirs.Add(packages[idx].dir);
			if(FindIndex(a.build_systems, packages[idx].build_system) < 0)
				a.build_systems.Add(packages[idx].build_system);
		}
	}
}

}
