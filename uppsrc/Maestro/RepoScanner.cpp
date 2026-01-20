#include "Maestro.h"

namespace Upp {

void RepoScanner::Scan(const String& root) {
	packages.Clear();
	assemblies.Clear();
	Walk(root);
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
}

void RepoScanner::DetectAssemblies() {}

}
