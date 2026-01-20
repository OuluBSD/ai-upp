#ifndef _Maestro_AssemblyInfo_h_
#define _Maestro_AssemblyInfo_h_

struct AssemblyInfo : Moveable<AssemblyInfo> {
	String         name;
	String         dir;
	Vector<String> packages;
	Vector<String> package_dirs;

	void Jsonize(JsonIO& jio) {
		jio
			("name", name)
			("dir", dir)
			("packages", packages)
			("package_dirs", package_dirs)
		;
	}
};

#endif