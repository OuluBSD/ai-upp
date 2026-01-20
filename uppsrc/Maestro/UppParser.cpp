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

int UppParser::ParseDescription(const Vector<String>& lines, int i) {
	String line = TrimBoth(lines[i]);
	RegExp re("^description\\s+\"([^\"]*)\"");
	if(re.Match(line)) {
		raw_description = re[0];
		description_text = raw_description;
		
		RegExp reColor("\\\\377B(\\d+),(\\d+),(\\d+)$");
		RegExp reColorOct("\377B(\\d+),(\\d+),(\\d+)$");
		
		auto ExtractColor = [&](RegExp& rc) {
			if(rc.Match(description_text)) {
				description_color = Color(StrInt(rc[0]), StrInt(rc[1]), StrInt(rc[2]));
				int s, e;
				rc.GetMatchPos(0, s, e);
				description_text = description_text.Left(s);
				return true;
			}
			return false;
		};
		
		if(!ExtractColor(reColor))
			ExtractColor(reColorOct);
	} else {
		unparsed_lines.Add(lines[i]);
	}
	return i + 1;
}
int UppParser::ParseUses(const Vector<String>& lines, int i) { return i + 1; }
int UppParser::ParseFiles(const Vector<String>& lines, int i) { return i + 1; }
int UppParser::ParseMainConfig(const Vector<String>& lines, int i) { return i + 1; }
int UppParser::ParseAcceptFlags(const Vector<String>& lines, int i) { return i + 1; }
int UppParser::ParseLibrary(const Vector<String>& lines, int i, bool is_static) { return i + 1; }
int UppParser::ParseLink(const Vector<String>& lines, int i) { return i + 1; }

UppFileEntry UppParser::ParseFileEntry(const String& line) { return UppFileEntry(); }
void UppParser::ProcessFileGroups(Vector<FileGroup>& groups, Vector<String>& ungrouped_files) {}

}
