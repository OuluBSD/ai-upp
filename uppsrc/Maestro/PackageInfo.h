#ifndef _Maestro_PackageInfo_h_
#define _Maestro_PackageInfo_h_

struct FileGroup : Moveable<FileGroup> {
	String         name;
	Vector<String> files;
	bool           readonly = false;
	bool           auto_generated = false;

	void Jsonize(JsonIO& jio) {
		jio
			("name", name)
			("files", files)
			("readonly", readonly)
			("auto_generated", auto_generated)
		;
	}
};

struct PackageInfo : Moveable<PackageInfo> {
	String            name;
	String            dir;
	String            upp_path;
	String            build_system = "upp";
	Vector<String>    dependencies;
	Vector<FileGroup> groups;
	Vector<String>    ungrouped_files;
	Vector<String>    files;
	bool              is_virtual = false;
	String            virtual_type;
	ValueMap          metadata;

	void Jsonize(JsonIO& jio) {
		jio
			("name", name)
			("dir", dir)
			("upp_path", upp_path)
			("build_system", build_system)
			("dependencies", dependencies)
			("groups", groups)
			("ungrouped_files", ungrouped_files)
			("files", files)
			("is_virtual", is_virtual)
			("virtual_type", virtual_type)
			("metadata", metadata)
		;
	}
};

#endif