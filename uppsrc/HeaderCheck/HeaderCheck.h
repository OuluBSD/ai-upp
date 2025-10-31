#ifndef _HeaderCheck_HeaderCheck_h_
#define _HeaderCheck_HeaderCheck_h_

#include <Core/Core.h>

NAMESPACE_UPP

struct PackageInfo : Moveable<PackageInfo> {
	String name;
	String path;
	Vector<String> files;
	
	void Serialize(Stream& s) { s % name % path % files; }
};

struct IncludeInfo : Moveable<IncludeInfo> {
	String file;
	String include;
	
	void Serialize(Stream& s) { s % file % include; }
};

struct HeaderAnalysis {
	Vector<PackageInfo> packages;
	Index<String> assembly_dirs;
	VectorMap<String, Vector<String>> file_includes; // Maps file -> includes
	Vector<String> errors;
	
	void Clear() {
		packages.Clear();
		assembly_dirs.Clear();
		file_includes.Clear();
		errors.Clear();
	}
	
	String GetPackageNameForFile(const String& file);
	bool IsSystemInclude(const String& include);
	bool IsMainHeader(const String& include, const String& package);
	void AnalyzeIncludes();
	void PrintResult();
	
	// Additional methods needed
	bool FindPackage(const String& name, PackageInfo& out_pkg);
	void LoadDependenciesRecursively(const PackageInfo& start_pkg);
	Vector<String> FindPackageDependencies(const String& upp_file);
	Vector<String> FindIncludeStatements(const String& file_path);
};

int AnalyzeHeaderDependencies(const Vector<String>& assembly_dirs, const String& package_name);

END_UPP_NAMESPACE

#endif