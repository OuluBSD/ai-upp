#ifndef _Maestro_UppParser_h_
#define _Maestro_UppParser_h_

struct UseEntry : Moveable<UseEntry> {
	String package;
	String flags;
};

struct FileEntry : Moveable<FileEntry> {
	String path;
	String flags;
	bool separator = false;
	bool readonly = false;
};

struct FileGroup : Moveable<FileGroup> {
	String name;
	Vector<String> files;
	bool readonly = false;
	
	FileGroup() {}
	FileGroup(const FileGroup& f) {
		name = f.name;
		files = clone(f.files);
		readonly = f.readonly;
	}
};

class UppParser {
public:
	Vector<UseEntry> uses;
	Vector<FileEntry> files;
	String description_text;
	Color  description_color = Null;
	
	Vector<String> mainconfigs;
	Vector<String> acceptflags;
	Vector<String> libraries;
	Vector<String> static_libraries;
	Vector<String> unparsed_lines;
	Vector<String> raw_lines;
	String raw_description;

	void ParseFile(const String& path);
	void Parse(const String& content);
	void ProcessFileGroups(Vector<FileGroup>& groups, Vector<String>& ungrouped_files);
	
	int ParseDescription(const Vector<String>& lines, int i);
	int ParseUses(const Vector<String>& lines, int i);
	int ParseFiles(const Vector<String>& lines, int i);
	int ParseMainConfig(const Vector<String>& lines, int i);
	int ParseAcceptFlags(const Vector<String>& lines, int i);
	int ParseLibrary(const Vector<String>& lines, int i, bool is_static);
	int ParseLink(const Vector<String>& lines, int i);
	
	FileEntry ParseFileEntry(const String& line);
	
	void Reset();
};

#endif
