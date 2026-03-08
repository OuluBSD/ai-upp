
#ifndef _Maestro_RepoScanner_h_
#define _Maestro_RepoScanner_h_

struct PackageInfo : Moveable<PackageInfo> {
	String         name;
	String         dir;
	String         upp_path;
	Vector<String> files;
	ValueMap       metadata;
	String         build_system;
	Vector<String> dependencies;
	Vector<FileGroup> groups;
	Vector<String> ungrouped_files;
	
	PackageInfo() {}
	PackageInfo(const PackageInfo& p) {
		name = p.name;
		dir = p.dir;
		upp_path = p.upp_path;
		files = clone(p.files);
		metadata = clone(p.metadata);
		build_system = p.build_system;
		dependencies = clone(p.dependencies);
		groups = clone(p.groups);
		ungrouped_files = clone(p.ungrouped_files);
	}
};

struct RepoScanner {
	Array<PackageInfo> packages;
	Array<AssemblyInfo> assemblies;

	void Scan(const String& root);
	void Walk(const String& dir);
	void AddPackage(const String& path, const String& build_system);
	void DetectAssemblies();
};

#endif
