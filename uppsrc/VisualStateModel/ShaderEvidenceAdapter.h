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

struct VsmShaderEvidenceObservation : Moveable<VsmShaderEvidenceObservation> {
	String path;
	String window;
	String backend;
	int64 timestamp_ms = 0;
	int runs = 0;
	int width = 0;
	int height = 0;
	String error;

	bool IsSuccessful() const { return error.IsEmpty(); }

	void Jsonize(JsonIO& json) { json("path", path)("window", window)("backend", backend)
		("timestamp_ms", timestamp_ms)("runs", runs)("width", width)
		("height", height)("error", error); }
};

inline VsmShaderEvidenceObservation MakeVsmShaderEvidenceObservation(
	const char *path, const VsmShaderWindowEvidence& evidence,
	const char *backend = "cpu-reference")
{
	VsmShaderEvidenceObservation observation;
	observation.path = path;
	observation.window = evidence.id;
	observation.backend = backend;
	observation.timestamp_ms = evidence.timestamp_ms;
	observation.runs = evidence.runs.GetCount();
	observation.width = evidence.evidence.image.width;
	observation.height = evidence.evidence.image.height;
	observation.error = evidence.error;
	return observation;
}

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
	bool ProcessSource(const VsmImageBuffer& source,
	                   const Vector<VsmPackedWindow>& windows,
	                   Vector<VsmShaderWindowEvidence>& output,
	                   String& error) const;
};

}

#endif
