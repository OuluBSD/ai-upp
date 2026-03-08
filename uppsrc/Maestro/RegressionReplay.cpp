#include "Maestro.h"

namespace Upp {

RegressionReplay::RegressionReplay(const String& maestro_root)
{
	base_path = NormalizePath(maestro_root);
	RealizeDirectory(AppendFileName(base_path, "docs/maestro/convert/runs"));
}

String RegressionReplay::GetRunPath(const String& id)
{
	return AppendFileName(base_path, "docs/maestro/convert/runs/" + id);
}

RunManifest RegressionReplay::CaptureManifest(const String& pipeline_id, const String& source, const String& target, const ConversionMemory& memory)
{
	RunManifest m;
	m.run_id = FormatIntHex(Random(), 8);
	m.timestamp = GetSysTime();
	m.pipeline_id = pipeline_id;
	m.source_path = source;
	m.target_path = target;
	m.decision_fingerprint = memory.ComputeDecisionFingerprint();
	m.status = "running";
	
	// Revisions would be captured from Git if available
	m.source_revision = "unknown";
	m.target_revision_before = "unknown";
	
	return m;
}

bool RegressionReplay::SaveManifest(const RunManifest& manifest)
{
	String dir = GetRunPath(manifest.run_id);
	RealizeDirectory(dir);
	return StoreAsJsonFile(manifest, AppendFileName(dir, "manifest.json"), true);
}

RunManifest RegressionReplay::LoadManifest(const String& run_id)
{
	RunManifest m;
	LoadFromJsonFile(m, AppendFileName(GetRunPath(run_id), "manifest.json"));
	return m;
}

Array<RunManifest> RegressionReplay::ListRuns()
{
	Array<RunManifest> list;
	FindFile ff(AppendFileName(base_path, "docs/maestro/convert/runs/*"));
	while(ff) {
		if(ff.IsDirectory()) {
			String m_file = AppendFileName(ff.GetPath(), "manifest.json");
			if(FileExists(m_file)) {
				RunManifest& m = list.Add();
				if(!LoadFromJsonFile(m, m_file))
					list.Drop();
			}
		}
		ff.Next();
	}
	return list;
}

DriftReport RegressionReplay::DetectDrift(const String& run_id, const String& current_target, const ConversionMemory& memory)
{
	DriftReport r;
	r.run_id = run_id;
	r.timestamp = GetSysTime();
	
	RunManifest original = LoadManifest(run_id);
	
	// Detect decision drift
	String current_fingerprint = memory.ComputeDecisionFingerprint();
	r.decision_drift("drift_detected") = (current_fingerprint != original.decision_fingerprint);
	r.decision_drift("current") = current_fingerprint;
	r.decision_drift("original") = original.decision_fingerprint;
	
	if((bool)r.decision_drift["drift_detected"])
		r.drift_detected = true;
	
	// Structural drift (file hashes) would be computed here
	r.structural_drift("drift_detected") = false;
	
	return r;
}

}
