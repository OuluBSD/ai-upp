#include "Maestro.h"

namespace Upp {

void UppParser::Reset() {
	uses.Clear();
	files.Clear();
	mainconfigs.Clear();
	acceptflags.Clear();
	libraries.Clear();
	static_libraries.Clear();
	unparsed_lines.Clear();
	raw_lines.Clear();
	raw_description = "";
	description_text = "";
	description_color = Null;
}

void UppParser::ParseFile(const String& path) {
	Reset();
	Parse(LoadFile(path));
}

void UppParser::Parse(const String& content) {
	Vector<String> lines = Split(content, '\n', false);
	raw_lines = clone(lines);
	
	for(int i = 0; i < lines.GetCount(); i++) {
		String l = TrimBoth(lines[i]);
		if(l.IsEmpty()) continue;
		
		if(l.StartsWith("description")) i = ParseDescription(lines, i);
		else if(l.StartsWith("uses")) i = ParseUses(lines, i);
		else if(l.StartsWith("file")) i = ParseFiles(lines, i);
		else if(l.StartsWith("mainconfig")) i = ParseMainConfig(lines, i);
		else if(l.StartsWith("acceptflags")) i = ParseAcceptFlags(lines, i);
		else if(l.StartsWith("library")) i = ParseLibrary(lines, i, false);
		else if(l.StartsWith("static_library")) i = ParseLibrary(lines, i, true);
		else if(l.StartsWith("link")) i = ParseLink(lines, i);
		else unparsed_lines.Add(lines[i]);
	}
}

int UppParser::ParseDescription(const Vector<String>& lines, int i) {
	String l = lines[i];
	int start = l.Find('"');
	if(start >= 0) {
		int end = l.Find('"', start + 1);
		if(end >= 0) {
			description_text = l.Mid(start + 1, end - start - 1);
		}
	}
	return i;
}

int UppParser::ParseUses(const Vector<String>& lines, int i) {
	String l = lines[i];
	RegExp re("\"([^\"]+)\"");
	while(re.GlobalMatch(l)) {
		UseEntry& u = uses.Add();
		u.package = re[0];
	}
	return i;
}

FileEntry UppParser::ParseFileEntry(const String& line) {
	FileEntry fe;
	int start = line.Find('"');
	if(start >= 0) {
		int end = line.Find('"', start + 1);
		if(end >= 0) {
			fe.path = line.Mid(start + 1, end - start - 1);
			fe.flags = TrimBoth(line.Mid(end + 1));
			if(fe.flags.EndsWith(";")) fe.flags.Remove(fe.flags.GetCount() - 1);
		}
	}
	return fe;
}

int UppParser::ParseFiles(const Vector<String>& lines, int i) {
	FileEntry fe = ParseFileEntry(lines[i]);
	if(!fe.path.IsEmpty())
		files.Add(fe);
	return i;
}

int UppParser::ParseMainConfig(const Vector<String>& lines, int i) {
	mainconfigs.Add(lines[i]);
	return i;
}

int UppParser::ParseAcceptFlags(const Vector<String>& lines, int i) {
	acceptflags.Add(lines[i]);
	return i;
}

int UppParser::ParseLibrary(const Vector<String>& lines, int i, bool is_static) {
	if(is_static) static_libraries.Add(lines[i]);
	else libraries.Add(lines[i]);
	return i;
}

int UppParser::ParseLink(const Vector<String>& lines, int i) {
	return i;
}

void UppParser::ProcessFileGroups(Vector<FileGroup>& groups, Vector<String>& ungrouped_files) {
	FileGroup* current_group = nullptr;
	bool current_readonly = false;
	
	for(const auto& fe : files) {
		if(fe.separator) {
			current_group = &groups.Add();
			current_group->name = fe.path;
			current_group->readonly = fe.readonly;
			current_readonly = fe.readonly;
		} else {
			if(current_group) {
				current_group->files.Add(fe.path);
			} else {
				ungrouped_files.Add(fe.path);
			}
		}
	}
}

}