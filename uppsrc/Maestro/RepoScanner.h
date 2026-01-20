#ifndef _Maestro_RepoScanner_h_
#define _Maestro_RepoScanner_h_

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

#endif