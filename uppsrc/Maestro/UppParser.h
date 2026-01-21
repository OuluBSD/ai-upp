#ifndef _Maestro_UppParser_h_
#define _Maestro_UppParser_h_

struct UppUseEntry : Moveable<UppUseEntry> {
	String package;
	String condition;
};

struct UppFileEntry : Moveable<UppFileEntry> {
	String path;
	String options;
	bool   readonly = false;
	bool   separator = false;
	String highlight;
	String charset;
	String condition;
};

struct UppConfigEntry : Moveable<UppConfigEntry> {
	String name;
	String param;
	String condition;
};

struct UppLibraryEntry : Moveable<UppLibraryEntry> {
	String libs;
	String condition;
};

struct UppLinkEntry : Moveable<UppLinkEntry> {
	String flags;
	String condition;
};

struct FileGroup : Moveable<FileGroup> {
	String name;
	bool   readonly = false;
	Vector<String> files;
};

class UppParser {
public:
	String raw_description;
	String description_text;
	Color  description_color;
	
	Array<UppUseEntry> uses;
	Array<UppFileEntry> files;
	Array<UppConfigEntry> mainconfigs;
	Vector<String> acceptflags;
	Array<UppLibraryEntry> libraries;
	Array<UppLibraryEntry> static_libraries;
	Array<UppLinkEntry> links;
	
	Vector<String> unparsed_lines;
	Vector<String> raw_lines;

	void Reset();
	void ParseFile(const String& path);
	void Parse(const String& content);
	void ProcessFileGroups(Vector<FileGroup>& groups, Vector<String>& ungrouped_files);

private:
	int ParseDescription(const Vector<String>& lines, int i);
	int ParseUses(const Vector<String>& lines, int i);
	int ParseFiles(const Vector<String>& lines, int i);
	int ParseMainConfig(const Vector<String>& lines, int i);
	int ParseAcceptFlags(const Vector<String>& lines, int i);
	int ParseLibrary(const Vector<String>& lines, int i, bool is_static);
	int ParseLink(const Vector<String>& lines, int i);
	
	UppFileEntry ParseFileEntry(const String& line);
};

#endif
