#include "Maestro.h"

namespace Upp {

const char *MAESTRO_NOTE_COMMENT = "// NOTE: This header is normally included inside namespace Upp";

String StructureTools::GetGuardName(const String& package_name, const String& header_path) {
	String stem = GetFileTitle(header_path);
	String raw = package_name + "_" + stem + "_H";
	String res;
	for(int i = 0; i < raw.GetCount(); i++) {
		if(IsAlNum(raw[i])) res << (char)ToUpper(raw[i]);
		else res << '_';
	}
	return res;
}

bool StructureTools::FixHeaderGuards(const String& header_path, const String& package_name) {
	String original = LoadFile(header_path);
	if(original.IsEmpty()) return false;
	
	String guard = GetGuardName(package_name, header_path);
	Vector<String> lines = Split(original, '\n', false);
	String body;
	
	for(const auto& l : lines) {
		String nl = TrimBoth(l);
		if(nl.StartsWith("#pragma once")) continue;
		if(nl.StartsWith("#ifndef") && nl.Find(guard) >= 0) continue;
		if(nl.StartsWith("#define") && nl.Find(guard) >= 0) continue;
		if(nl.StartsWith("#endif") && nl.Find(guard) >= 0) continue;
		body << l << "\n";
	}
	
	body = TrimBoth(body);
	
	String content;
	content << "#ifndef " << guard << "\n"
	        << "#define " << guard << "\n\n"
	        << MAESTRO_NOTE_COMMENT << "\n\n"
	        << body << "\n\n"
	        << "#endif // " << guard << "\n";
	        
	return SaveFile(header_path, content);
}

bool StructureTools::EnsureMainHeaderContent(const String& main_header_path, const String& package_name) {
	String guard = GetGuardName(package_name, main_header_path);
	String content;
	content << "#ifndef " << guard << "\n"
	        << "#define " << guard << "\n\n"
	        << MAESTRO_NOTE_COMMENT << "\n\n"
	        << "// Main header for " << package_name << "\n\n"
	        << "#endif // " << guard << "\n";
	return SaveFile(main_header_path, content);
}

bool StructureTools::NormalizeCppIncludes(const String& source_path, const String& main_header_name) {
	String original = LoadFile(source_path);
	if(original.IsEmpty()) return false;
	
	Vector<String> lines = Split(original, '\n', false);
	String includes;
	String body;
	
	for(const auto& l : lines) {
		String nl = TrimBoth(l);
		if(nl.StartsWith("#include")) {
			if(nl.Find("\"" + main_header_name + "\"") < 0)
				includes << l << "\n";
		} else {
			body << l << "\n";
		}
	}
	
	String content;
	content << "#include \"" << main_header_name << "\"\n" << includes << "\n" << body;
	return SaveFile(source_path, content);
}

bool StructureTools::ReduceSecondaryHeaderIncludes(const String& header_path) {
	String original = LoadFile(header_path);
	if(original.IsEmpty()) return false;
	if(original.Find(MAESTRO_NOTE_COMMENT) >= 0) return true;
	
	Vector<String> lines = Split(original, '\n', false);
	int insert_at = 0;
	for(int i = 0; i < min((int)lines.GetCount(), 10); i++) {
		if(TrimBoth(lines[i]).StartsWith("#define")) {
			insert_at = i + 1;
			break;
		}
	}
	
	lines.Insert(insert_at, "");
	lines.Insert(insert_at + 1, MAESTRO_NOTE_COMMENT);
	
	return SaveFile(header_path, Join(lines, "\n"));
}

}