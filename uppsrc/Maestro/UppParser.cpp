#include "Maestro.h"

namespace Upp {

void UppParser::Reset() {
	raw_description.Clear();
	description_text.Clear();
	description_color = Null;
	uses.Clear();
	files.Clear();
	mainconfigs.Clear();
	acceptflags.Clear();
	libraries.Clear();
	static_libraries.Clear();
	links.Clear();
	unparsed_lines.Clear();
	raw_lines.Clear();
}

void UppParser::ParseFile(const String& path) {
	Parse(LoadFile(path));
}

void UppParser::Parse(const String& content) {
	Reset();
	
	raw_lines = Split(content, '\n');
	
	int i = 0;
	while(i < raw_lines.GetCount()) {
		String line = raw_lines[i];
		String stripped = TrimBoth(line);
		
		if(stripped.IsEmpty() || stripped.StartsWith("//")) {
			i++;
			continue;
		}
		
		if(stripped.StartsWith("description")) {
			i = ParseDescription(raw_lines, i);
		} else if(stripped.StartsWith("uses")) {
			i = ParseUses(raw_lines, i);
		} else if(stripped.StartsWith("file")) {
			i = ParseFiles(raw_lines, i);
		} else if(stripped.StartsWith("mainconfig")) {
			i = ParseMainConfig(raw_lines, i);
		} else if(stripped.StartsWith("acceptflags")) {
			i = ParseAcceptFlags(raw_lines, i);
		} else if(stripped.StartsWith("library")) {
			i = ParseLibrary(raw_lines, i, false);
		} else if(stripped.StartsWith("static_library")) {
			i = ParseLibrary(raw_lines, i, true);
		} else if(stripped.StartsWith("link")) {
			i = ParseLink(raw_lines, i);
		} else {
			unparsed_lines.Add(line);
			i++;
		}
	}
}

int UppParser::ParseDescription(const Vector<String>& lines, int i) { return i + 1; }
int UppParser::ParseUses(const Vector<String>& lines, int i) { return i + 1; }
int UppParser::ParseFiles(const Vector<String>& lines, int i) { return i + 1; }
int UppParser::ParseMainConfig(const Vector<String>& lines, int i) { return i + 1; }
int UppParser::ParseAcceptFlags(const Vector<String>& lines, int i) { return i + 1; }
int UppParser::ParseLibrary(const Vector<String>& lines, int i, bool is_static) { return i + 1; }
int UppParser::ParseLink(const Vector<String>& lines, int i) { return i + 1; }

UppFileEntry UppParser::ParseFileEntry(const String& line) { return UppFileEntry(); }
void UppParser::ProcessFileGroups(Vector<FileGroup>& groups, Vector<String>& ungrouped_files) {}

}
