#include "Overviewer.h"

void OverviewerProject::AnalyzeEntry(const String& rel_path) {
	EntrySuggestions& sug = suggestions.GetAdd(rel_path);
	sug.Clear();

	String name = GetFileName(rel_path);
	String ext = GetFileExt(rel_path);
	
	auto add_tag = [&](int cat, const String& tag, double conf, const String& src) {
		Suggestion s;
		s.text = tag;
		s.confidence = conf;
		s.source = src;
		if(cat == 0) sug.current_tags.Add(s);
		else if(cat == 1) sug.reason_tags.Add(s);
		else sug.gap_tags.Add(s);
	};

	// Name-based heuristics
	if(name.Find("test") >= 0 || name.Find("spec") >= 0)
		add_tag(1, "unit-test", 0.8, "filename");
	
	if(name == "examples" || name == "demo" || rel_path.Find("/examples/") >= 0)
		add_tag(1, "example/tutorial", 0.9, "path");

	// Extension-based
	if(ext == ".md" || ext == ".txt")
		add_tag(0, "documentation", 0.7, "extension");

	// Notes-based
	const FileMetadata* m = metadata.FindPtr(rel_path);
	if(m && !m->notes.IsEmpty()) {
		String n = ToLower(m->notes);
		if(n.Find("refactor") >= 0) add_tag(2, "refactor", 0.8, "notes");
		if(n.Find("performance") >= 0 || n.Find("slow") >= 0) add_tag(2, "performance", 0.8, "notes");
		if(n.Find("cleanup") >= 0) add_tag(2, "cleanup", 0.7, "notes");
		if(n.Find("todo") >= 0) {
			Suggestion s;
			s.text = "Address TODO in notes";
			s.confidence = 0.6;
			s.source = "notes";
			sug.tasks.Add(s);
		}
	}

	// Content-based (minimal)
	String abs_path = AppendFileName(working_dir, rel_path);
	if(FileExists(abs_path) && GetFileLength(abs_path) < 64000) {
		String content = LoadFile(abs_path);
		if(content.Find("FIXME") >= 0) {
			Suggestion s;
			s.text = "Fix FIXME in code";
			s.confidence = 0.9;
			s.source = "content";
			sug.problems.Add(s);
		}
	}
}
