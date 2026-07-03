#ifndef _VisualStateModel_SessionValidator_h_
#define _VisualStateModel_SessionValidator_h_

namespace Upp {

struct VsmValidationIssue : Moveable<VsmValidationIssue> {
	String severity; // "error" or "warning"
	String message;
	void Jsonize(JsonIO& json);
};

struct VsmValidationResult : Moveable<VsmValidationResult> {
	bool   ok = false;
	int    frames_checked = 0;
	int    crops_checked  = 0;
	Vector<VsmValidationIssue> issues;
	void Jsonize(JsonIO& json);
};

class VsmSessionValidator {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }
	VsmValidationResult Validate(const String& session_dir);

private:
	CoreLog log_;
};

} // namespace Upp

#endif
