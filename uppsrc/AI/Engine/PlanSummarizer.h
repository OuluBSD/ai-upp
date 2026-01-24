#ifndef _AI_Engine_PlanSummarizer_h_
#define _AI_Engine_PlanSummarizer_h_

#include <Maestro/PlanModels.h>

NAMESPACE_UPP

class PlanSummarizer {
public:
	static String GetPlanSummaryText(const Array<Track>& tracks, const String& current_track, const String& current_phase, const String& current_task);
	static String GetRunbookSummary(const Runbook& rb);
	static String GetWorkGraphSummary(const WorkGraph& wg);
};

END_UPP_NAMESPACE

#endif
