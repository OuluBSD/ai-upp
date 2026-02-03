#ifndef _Maestro_Breadcrumb_h_
#define _Maestro_Breadcrumb_h_

#include <Core/Core.h>

namespace Upp {

struct ToolCall : Moveable<ToolCall> {
	String   tool;
	ValueMap args;
	Value    result;
	String   error;
	Time     timestamp;
	
	void Jsonize(JsonIO& jio) {
		jio("tool", tool)("args", args)("result", result)("error", error)("timestamp", timestamp);
	}
};

struct FileModification : Moveable<FileModification> {
	String path;
	String operation; // create, modify, delete
	String diff;
	Time   timestamp;
	int64  size;
	
	void Jsonize(JsonIO& jio) {
		jio("path", path)("operation", operation)("diff", diff)("timestamp", timestamp)("size", size);
	}
};

struct Breadcrumb : Moveable<Breadcrumb> {
	String   timestamp_id; // YYYYMMDD_HHMMSS_microseconds
	String   breadcrumb_id;
	
	String   prompt;
	String   response;
	Array<ToolCall>         tools_called;
	Array<FileModification> files_modified;
	
	String   parent_session_id;
	int      depth_level = 0;
	
	String   model_used;
	int      input_tokens = 0;
	int      output_tokens = 0;
	double   cost = 0;
	String   error;
	String   kind = "note";
	Vector<String> tags;
	ValueMap payload;

	void Jsonize(JsonIO& jio);
	
	Breadcrumb();
};

class BreadcrumbManager {
public:
	static String GenerateTimestampId();
	static bool SaveBreadcrumb(const Breadcrumb& b, const String& docs_root, const String& session_id);
	static Array<Breadcrumb> ListBreadcrumbs(const String& docs_root, const String& session_id, int depth = -1);
};

}

#endif
