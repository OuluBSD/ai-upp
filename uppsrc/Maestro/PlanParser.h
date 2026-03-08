
#ifndef _Maestro_PlanParser_h_
#define _Maestro_PlanParser_h_

class PlanParser {
public:
	Array<Track> tracks;
	Array<Runbook> runbooks;
	Array<WorkGraph> workgraphs;

	void Load(const String& plan_root);
	void Reset();

	void LoadTrack(const String& track_dir);
	void LoadPhase(Track& track, const String& phase_dir);
	void LoadTask(Phase& phase, const String& task_file);
	
	void LoadRunbooks(const String& docs_root);
	void LoadWorkGraphs(const String& docs_root);
	void LoadMaestroTracks(const String& docs_root);
	
	// Persistence
	bool SaveMaestroPhase(const String& docs_root, const String& track_id, const String& phase_id);
	bool UpdateTaskStatus(const String& docs_root, const String& track_id, const String& phase_id, const String& task_id, TaskStatus status);
};

#endif

