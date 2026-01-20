#ifndef _Maestro_RepoScanner_h_
#define _Maestro_RepoScanner_h_

#include <Core/Core.h>
#include "UppParser.h"

NAMESPACE_UPP

struct PackageInfo : Moveable<PackageInfo> {
	String name;
	String path;
	String upp_path;
	String dir;
	String build_system;
	Vector<String> dependencies;
	Vector<String> files;
	Vector<FileGroup> groups;
	Vector<String> ungrouped_files;
	ValueMap metadata;
};

struct AssemblyInfo : Moveable<AssemblyInfo> {
	String name;
	String dir;
	String assembly_type;
	Vector<String> packages;
	Vector<String> package_dirs;
	Vector<String> build_systems;
};

class RepoScanner {
public:
	Array<PackageInfo>  packages;
	Array<AssemblyInfo> assemblies;
	
	void Scan(const String& root);
	void DetectAssemblies();

private:
	void Walk(const String& dir);
	void AddPackage(const String& path, const String& build_system);
};

END_UPP_NAMESPACE

#endif
