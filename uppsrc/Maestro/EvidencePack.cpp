#include "Maestro.h"

namespace Upp {

EvidencePack EvidenceCollector::CollectAll()
{
	CollectBuildSignatures();
	CollectDocs();
	CollectCliHelp();
	
	EvidencePack pack;
	pack.meta.repo_root = repo_root;
	pack.meta.created_at = GetSysTime();
	pack.meta.evidence_count = items.GetCount();
	pack.meta.total_bytes = total_bytes;
	pack.meta.truncated_items = clone(truncated_items);
	pack.meta.skipped_items = clone(skipped_items);
	
	String hash_input;
	for(const auto& item : items) hash_input << item.source << "\n";
	pack.meta.pack_id = "pack-" + SHA256String(hash_input).Left(16);
	
	pack.items = pick(items);
	return pack;
}

bool EvidenceCollector::AddItem(const String& kind, const String& source, const String& content, bool truncated)
{
	if(files_processed >= max_files || total_bytes + content.GetCount() > max_bytes) {
		skipped_items.Add(source);
		return false;
	}
	
	EvidenceItem& item = items.Add();
	item.kind = kind;
	item.source = source;
	item.content = content;
	item.truncated = truncated;
	item.size_bytes = content.GetCount();
	
	total_bytes += item.size_bytes;
	files_processed++;
	return true;
}

String EvidenceCollector::ReadFileSafe(const String& path, int64 max_size, bool& truncated)
{
	String content = LoadFile(path);
	truncated = false;
	if(content.GetCount() > max_size) {
		truncated = true;
		return content.Left((int)max_size);
	}
	return content;
}

void EvidenceCollector::CollectBuildSignatures()
{
	static const char* build_files[] = {
		"CMakeLists.txt", "Makefile", "GNUmakefile", "package.json", "pyproject.toml",
		"setup.py", "Cargo.toml", "go.mod", "pom.xml", "build.gradle"
	};
	
	for(const char* f : build_files) {
		String path = AppendFileName(repo_root, f);
		if(FileExists(path)) {
			bool truncated;
			String content = ReadFileSafe(path, 5000, truncated);
			if(content.GetCount()) {
				if(truncated) truncated_items.Add(f);
				AddItem("file", f, content, truncated);
			}
		}
	}
}

void EvidenceCollector::CollectDocs()
{
	String readme_path = AppendFileName(repo_root, "README.md");
	if(FileExists(readme_path)) {
		bool truncated;
		String content = ReadFileSafe(readme_path, 3000, truncated);
		if(content.GetCount()) {
			if(truncated) truncated_items.Add("README.md");
			AddItem("docs", "README.md", content, truncated);
		}
	}
}

void EvidenceCollector::CollectCliHelp()
{
	// For now, we don't auto-execute binaries for safety
}

}
