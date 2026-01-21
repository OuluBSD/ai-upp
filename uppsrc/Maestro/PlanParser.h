#ifndef _Maestro_PlanParser_h_
#define _Maestro_PlanParser_h_

#include "PlanModels.h"

NAMESPACE_UPP

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
};

END_UPP_NAMESPACE

#endif