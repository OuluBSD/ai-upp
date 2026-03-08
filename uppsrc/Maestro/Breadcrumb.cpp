#include "Maestro.h"

namespace Upp {

Breadcrumb::Breadcrumb() {
	timestamp_id = BreadcrumbManager::GenerateTimestampId();
}

void Breadcrumb::Jsonize(JsonIO& jio) {
	jio("timestamp_id", timestamp_id)
	   ("session_id", session_id)
	   ("depth_level", depth_level)
	   ("model_used", model_used)
	   ("prompt", prompt)
	   ("response", response)
	   ("error", error)
	   ("input_tokens", input_tokens)
	   ("output_tokens", output_tokens)
	   ("cost", cost)
	   ("tools_called", tools_called)
	   ("files_modified", files_modified)
	   ("metadata", metadata);
}

String BreadcrumbManager::GenerateTimestampId() {
	return Format((Date)GetSysTime(), "YYYYMMDD_HHMMSS");
}

bool BreadcrumbManager::SaveBreadcrumb(const Breadcrumb& b, const String& docs_root, const String& session_id) {
	String base = AppendFileName(AppendFileName(docs_root, "docs"), "sessions");
	base = AppendFileName(base, session_id);
	
	String bdir = AppendFileName(AppendFileName(base, "breadcrumbs"), AsString(b.depth_level));
	RealizeDirectory(bdir);
	
	String path = AppendFileName(bdir, b.timestamp_id + ".json");
	return StoreAsJsonFile(b, path, true);
}

Array<Breadcrumb> BreadcrumbManager::ListBreadcrumbs(const String& docs_root, const String& session_id, int depth) {
	Array<Breadcrumb> result;
	String base = AppendFileName(AppendFileName(AppendFileName(docs_root, "docs"), "sessions"), session_id);
	String bread_root = AppendFileName(base, "breadcrumbs");
	
	if(!DirectoryExists(bread_root)) return result;
	
	FindFile ff(AppendFileName(bread_root, "*"));
	while(ff) {
		if(ff.IsDirectory() && ff.GetName() != "." && ff.GetName() != "..") {
			int d = atoi(ff.GetName());
			if(depth < 0 || d == depth) {
				FindFile fjson(AppendFileName(ff.GetPath(), "*.json"));
				while(fjson) {
					Breadcrumb b;
					if(LoadFromJsonFile(b, fjson.GetPath())) {
						result.Add(pick(b));
					}
					fjson.Next();
				}
			}
		}
		ff.Next();
	}
	
	Sort(result, [](const Breadcrumb& a, const Breadcrumb& b) { return a.timestamp_id < b.timestamp_id; });
	return result;
}

}