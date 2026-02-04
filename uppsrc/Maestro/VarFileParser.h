#ifndef _Maestro_VarFileParser_h_
#define _Maestro_VarFileParser_h_

#include <Core/Core.h>

namespace Upp {

class VarFileParser {
public:
	VectorMap<String, String> vars;
	Vector<String> unparsed_lines;

	void Parse(const String& content);
	bool Load(const String& path);
	
	String Get(const String& key) const { return vars.Get(key, ""); }
};

class UppAssemblyReader {
public:
	struct AssemblyInfo {
		String var_file;
		String assembly_name;
		Vector<String> upp_paths;
		VectorMap<String, String> vars;
	};

	static Array<AssemblyInfo> ReadAll(const String& repo_root = "");
};

}

#endif
