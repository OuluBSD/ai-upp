#ifndef _Maestro_UppParser_h_
#define _Maestro_UppParser_h_

struct UppFileEntry : Moveable<UppFileEntry> {
	String path;
	String options;
	bool   readonly = false;
	bool   separator = false;
	String highlight;
	String charset;
};

struct UppUseEntry : Moveable<UppUseEntry> {
	String package;
	String condition;
};

struct UppConfigEntry : Moveable<UppConfigEntry> {
	String name;
	String param;
};

struct UppLibraryEntry : Moveable<UppLibraryEntry> {
	String condition;
	String libs;
};

struct UppLinkEntry : Moveable<UppLinkEntry> {
	String condition;
	String flags;
};

class UppParser {
public:
	String              raw_description;
	String              description_text;
	Color               description_color = Null;
	
	Vector<UppUseEntry>     uses;
	Vector<UppFileEntry>    files;
	Vector<UppConfigEntry>  mainconfigs;
	Vector<String>          acceptflags;
	Vector<UppLibraryEntry> libraries;
	Vector<UppLibraryEntry> static_libraries;
	Vector<UppLinkEntry>    links;
	
	Vector<String>          unparsed_lines;
	Vector<String>          raw_lines;

	void Reset();
	void Parse(const String& content);
	void ParseFile(const String& path);

private:
	int ParseDescription(const Vector<String>& lines, int i);
	int ParseUses(const Vector<String>& lines, int i);
	int ParseFiles(const Vector<String>& lines, int i);
	int ParseMainConfig(const Vector<String>& lines, int i);
	int ParseAcceptFlags(const Vector<String>& lines, int i);
	int ParseLibrary(const Vector<String>& lines, int i, bool is_static);
	int ParseLink(const Vector<String>& lines, int i);
	
	UppFileEntry ParseFileEntry(const String& line);
	void ProcessFileGroups(Vector<FileGroup>& groups, Vector<String>& ungrouped_files);
};

#endif