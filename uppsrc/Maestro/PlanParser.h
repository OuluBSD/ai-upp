#ifndef _Maestro_PlanParser_h_
#define _Maestro_PlanParser_h_

#include "PlanModels.h"

class PlanParser {
public:
	Array<Track> tracks;

	void Load(const String& plan_root);

private:
	void LoadTrack(const String& track_dir);
	void LoadPhase(Track& track, const String& phase_dir);
	void LoadTask(Phase& phase, const String& task_file);
};

#endif