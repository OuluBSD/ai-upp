#ifndef _VisualStateModel_ShaderEvidenceAdapter_h_
#define _VisualStateModel_ShaderEvidenceAdapter_h_

namespace Upp {

struct VsmShaderWindowEvidence : Moveable<VsmShaderWindowEvidence> {
	String id;
	int64 timestamp_ms = 0;
	VsmShaderEvidence evidence;
	Vector<VsmEvidenceTextRun> runs;
	String error;

	void Jsonize(JsonIO& json) { json("id", id)("timestamp_ms", timestamp_ms)
		("runs", runs)("error", error); }
};

class VsmShaderEvidenceAdapter {
	const VsmShaderRecognitionService *service_ = nullptr;

	static bool Slice(const VsmImageBuffer& source, const VsmPackedWindow& window,
	                 VsmImageBuffer& output, String& error);

public:
	void SetService(const VsmShaderRecognitionService& service) { service_ = &service; }
	const VsmShaderRecognitionService& GetService() const { ASSERT(service_); return *service_; }

	bool ProcessPacked(const VsmImageBuffer& packed,
	                   const Vector<VsmPackedWindow>& windows,
	                   Vector<VsmShaderWindowEvidence>& output,
	                   String& error) const;
};

}

#endif
