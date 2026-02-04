#ifndef _Maestro_DependencyResolver_h_
#define _Maestro_DependencyResolver_h_

class DependencyResolver {
public:
	static Array<PackageInfo> TopologicalSort(const Array<PackageInfo>& packages);
	static Vector<String> GetDependencies(const Array<PackageInfo>& all_packages, const String& start_package);
	static Vector<String> GetDependents(const Array<PackageInfo>& all_packages, const String& start_package);
};

#endif