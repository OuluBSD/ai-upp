#include "Breadcrumb.h"

namespace Upp {

Breadcrumb::Breadcrumb() {
	breadcrumb_id = AsString(Uuid::Create());
	timestamp_id = BreadcrumbManager::GenerateTimestampId();
}

void Breadcrumb::Jsonize(JsonIO& jio) {
	jio("timestamp", timestamp_id)
	   ("breadcrumb_id", breadcrumb_id)
	   ("prompt", prompt)
	   ("response", response)
	   ("tools_called", tools_called)
	   ("files_modified", files_modified)
	   ("parent_session_id", parent_session_id)
	   ("depth_level", depth_level)
	   ("model_used", model_used)
	   ("input_tokens", input_tokens)
	   ("output_tokens", output_tokens)
	   ("cost", cost)
	   ("error", error)
	   ("kind", kind)
	   ("tags", tags)
	   ("payload", payload);
}

String BreadcrumbManager::GenerateTimestampId() {
	Time now = GetSysTime();
	// For simplicity, using Time format, real version uses microseconds
	return Format("%04d%02d%02d_%02d%02d%02d", now.year, now.month, now.day, now.hour, now.minute, now.second);
}

bool BreadcrumbManager::SaveBreadcrumb(const Breadcrumb& b, const String& docs_root, const String& session_id) {
	String base = AppendFileName(AppendFileName(AppendFileName(docs_root, "docs"), "sessions"), session_id);
	String bdir = AppendFileName(AppendFileName(base, "breadcrumbs"), AsString(b.depth_level));
	RealizeDirectory(bdir);
	
	String path = AppendFileName(bdir, b.timestamp_id + ".json");
	return StoreAsJsonFile(b, path, true);
}

Array<Breadcrumb> BreadcrumbManager::ListBreadcrumbs(const String& docs_root, const String& session_id, int depth) {
	Array<Breadcrumb> list;
	String base = AppendFileName(AppendFileName(AppendFileName(docs_root, "docs"), "sessions"), session_id);
	String bdir = AppendFileName(base, "breadcrumbs");
	
	if(!DirectoryExists(bdir)) return list;
	
	auto scan_dir = [&](String d) {
		FindFile ff(AppendFileName(d, "*.json"));
		while(ff) {
			String content = LoadFile(ff.GetPath());
			if(!content.IsEmpty()) {
				Breadcrumb& b = list.Add();
				if(!LoadFromJson(b, content))
					list.Drop();
			}
			ff.Next();
		}
	};
	
	if(depth >= 0) {
		scan_dir(AppendFileName(bdir, AsString(depth)));
	} else {
		FindFile ff(AppendFileName(bdir, "*"));
		while(ff) {
			if(ff.IsDirectory())
				scan_dir(ff.GetPath());
			ff.Next();
		}
	}
	
	Sort(list, [](const Breadcrumb& a, const Breadcrumb& b) { return a.timestamp_id < b.timestamp_id; });
	return list;
}

}
