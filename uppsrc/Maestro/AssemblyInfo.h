#ifndef _Maestro_AssemblyInfo_h_
#define _Maestro_AssemblyInfo_h_

struct AssemblyInfo : Moveable<AssemblyInfo> {
	String         name;
	String         dir;
	String         assembly_type;
	Vector<String> packages;
	Vector<String> package_dirs;
	Vector<String> build_systems;

	void Jsonize(JsonIO& jio) {
		jio
			("name", name)
			("dir", dir)
			("assembly_type", assembly_type)
			("packages", packages)
			("package_dirs", package_dirs)
			("build_systems", build_systems)
		;
	}
};

#endif
