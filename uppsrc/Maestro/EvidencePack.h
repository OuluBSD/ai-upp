#ifndef _Maestro_EvidencePack_h_
#define _Maestro_EvidencePack_h_

struct EvidenceItem : Moveable<EvidenceItem> {
	String kind; // "file", "command", "docs"
	String source;
	String content;
	bool   truncated = false;
	int64  size_bytes = 0;

	void Jsonize(JsonIO& jio) {
		jio("kind", kind)("source", source)("content", content)("truncated", truncated)("size_bytes", size_bytes);
	}
};

struct EvidencePackMeta : Moveable<EvidencePackMeta> {
	String   pack_id;
	String   repo_root;
	Time     created_at;
	int      evidence_count = 0;
	int64    total_bytes = 0;
	ValueMap budget_applied;
	Vector<String> truncated_items;
	Vector<String> skipped_items;

	void Jsonize(JsonIO& jio) {
		jio("pack_id", pack_id)("repo_root", repo_root)("created_at", created_at)
		   ("evidence_count", evidence_count)("total_bytes", total_bytes)
		   ("budget_applied", budget_applied)("truncated_items", truncated_items)
		   ("skipped_items", skipped_items);
	}
};

struct EvidencePack : Moveable<EvidencePack> {
	EvidencePackMeta    meta;
	Array<EvidenceItem> items;

	void Jsonize(JsonIO& jio) {
		jio("meta", meta)("items", items);
	}
};

class EvidenceCollector {
	String repo_root;
	int    max_files = 60;
	int64  max_bytes = 250000;
	int    max_help_calls = 6;
	
	Array<EvidenceItem> items;
	int64  total_bytes = 0;
	int    files_processed = 0;
	int    help_calls_made = 0;
	Vector<String> truncated_items;
	Vector<String> skipped_items;

public:
	EvidenceCollector(const String& root) : repo_root(root) {}
	
	EvidencePack CollectAll();
	
private:
	void CollectBuildSignatures();
	void CollectDocs();
	void CollectCliHelp();
	
	bool AddItem(const String& kind, const String& source, const String& content, bool truncated);
	String ReadFileSafe(const String& path, int64 max_size, bool& truncated);
};

#endif
