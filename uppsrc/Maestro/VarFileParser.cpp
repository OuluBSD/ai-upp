#include "Maestro.h"

namespace Upp {

void VarFileParser::Parse(const String& content) {
	vars.Clear();
	unparsed_lines.Clear();
	
	Vector<String> lines = Split(content, '\n', false);
	for(const auto& line : lines) {
		String stripped = TrimBoth(line);
		if(stripped.IsEmpty() || stripped.StartsWith("//") || stripped.StartsWith("#"))
			continue;
			
		// Simple KEY = "value"; parser
		int eq = stripped.Find('=');
		if(eq >= 0) {
			String key = TrimBoth(stripped.Left(eq));
			String val_part = TrimBoth(stripped.Mid(eq + 1));
			if(val_part.StartsWith("\"")) {
				int end_quote = val_part.Find('"', 1);
				if(end_quote >= 0) {
					String val = val_part.Mid(1, end_quote - 1);
					vars.Add(key, val);
					continue;
				}
			}
		}
		unparsed_lines.Add(line);
	}
}

bool VarFileParser::Load(const String& path) {
	String content = LoadFile(path);
	if(content.IsVoid()) return false;
	Parse(content);
	return true;
}

Array<UppAssemblyReader::AssemblyInfo> UppAssemblyReader::ReadAll(const String& repo_root) {
	Array<AssemblyInfo> res;
	
	String ide_config_dir = AppendFileName(GetHomeDirectory(), ".config/u++/ide");
	if(!DirectoryExists(ide_config_dir)) return res;
	
	FindFile ff(AppendFileName(ide_config_dir, "*.var"));
	while(ff) {
		VarFileParser parser;
		if(parser.Load(ff.GetPath())) {
			AssemblyInfo& a = res.Add();
			a.var_file = ff.GetPath();
			a.assembly_name = GetFileTitle(ff.GetName());
			a.vars = clone(parser.vars);
			
			String upp = parser.Get("UPP");
			Vector<String> paths = Split(upp, ';', true);
			for(auto& p : paths) {
				String normalized = NormalizePath(p);
				a.upp_paths.Add(normalized);
			}
		}
		ff.Next();
	}
	
	return res;
}

} // namespace Upp
