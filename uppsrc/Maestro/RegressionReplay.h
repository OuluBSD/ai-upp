#ifndef _Maestro_RegressionReplay_h_
#define _Maestro_RegressionReplay_h_

struct DriftReport : Moveable<DriftReport> {
	String   run_id;
	String   replay_mode; // "dry", "apply"
	Time     timestamp;
	ValueMap structural_drift;
	ValueMap decision_drift;
	ValueMap semantic_drift;
	bool     drift_detected = false;

	void Jsonize(JsonIO& jio) {
		jio("run_id", run_id)("replay_mode", replay_mode)("timestamp", timestamp)
		   ("structural_drift", structural_drift)("decision_drift", decision_drift)
		   ("semantic_drift", semantic_drift)("drift_detected", drift_detected);
	}
};

class RegressionReplay {
	String base_path;

public:
	RegressionReplay(const String& maestro_root = ".");
	
	RunManifest CaptureManifest(const String& pipeline_id, const String& source, const String& target, const ConversionMemory& memory);
	bool        SaveManifest(const RunManifest& manifest);
	RunManifest LoadManifest(const String& run_id);
	Array<RunManifest> ListRuns();
	
	DriftReport DetectDrift(const String& run_id, const String& current_target, const ConversionMemory& memory);
	
private:
	String GetRunPath(const String& id);
};

#endif
