
#ifndef _Maestro_StructureTools_h_
#define _Maestro_StructureTools_h_

class StructureTools {
public:
	static bool FixHeaderGuards(const String& header_path, const String& package_name);
	static bool EnsureMainHeaderContent(const String& main_header_path, const String& package_name);
	static bool NormalizeCppIncludes(const String& source_path, const String& main_header_name);
	static bool ReduceSecondaryHeaderIncludes(const String& header_path);

private:
	static String GetGuardName(const String& package_name, const String& header_path);
};

#endif

