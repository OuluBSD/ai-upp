#ifndef _Maestro_Breadcrumb_h_
#define _Maestro_Breadcrumb_h_

struct FileModification : Moveable<FileModification> {
	String path;
	String hunk;
	String kind;
	
	void Jsonize(JsonIO& jio) {
		jio("path", path)("hunk", hunk)("kind", kind);
	}
	FileModification() {}
	FileModification(const FileModification& f) {
		path = f.path;
		hunk = f.hunk;
		kind = f.kind;
	}
};

struct ToolCall : Moveable<ToolCall> {
	String tool;
	Value  args;
	Value  result;
	String error;
	Time   timestamp;
	
	void Jsonize(JsonIO& jio) {
		jio("tool", tool)("args", args)("result", result)("error", error)("timestamp", timestamp);
	}
	ToolCall() {}
	ToolCall(const ToolCall& t) {
		tool = t.tool; args = clone(t.args); result = clone(t.result);
		error = t.error; timestamp = t.timestamp;
	}
};

struct Breadcrumb : Moveable<Breadcrumb> {
	String timestamp_id;
	String session_id;
	int    depth_level = 0;
	String model_used;
	String prompt;
	String response;
	String error;
	int    input_tokens = 0;
	int    output_tokens = 0;
	double cost = 0;
	Array<ToolCall> tools_called;
	Array<FileModification> files_modified;
	ValueMap metadata;

	void Jsonize(JsonIO& jio);
	
	Breadcrumb();
	Breadcrumb(const Breadcrumb& b) {
		timestamp_id = b.timestamp_id;
		session_id = b.session_id;
		depth_level = b.depth_level;
		model_used = b.model_used;
		prompt = b.prompt;
		response = b.response;
		error = b.error;
		input_tokens = b.input_tokens;
		output_tokens = b.output_tokens;
		cost = b.cost;
		tools_called = clone(b.tools_called);
		files_modified = clone(b.files_modified);
		metadata = clone(b.metadata);
	}
};

class BreadcrumbManager {
public:
	static String GenerateTimestampId();
	static bool SaveBreadcrumb(const Breadcrumb& b, const String& docs_root, const String& session_id);
	static Array<Breadcrumb> ListBreadcrumbs(const String& docs_root, const String& session_id, int depth = -1);
};

#endif
