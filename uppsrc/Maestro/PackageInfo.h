#ifndef _Maestro_PackageInfo_h_
#define _Maestro_PackageInfo_h_

struct FileGroup {
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

#endif
