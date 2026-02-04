#ifndef _Maestro_PlanSummarizer_h_
#define _Maestro_PlanSummarizer_h_



class PlanSummarizer {
public:
	static String GetPlanSummaryText(const Array<Track>& tracks, const String& current_track, const String& current_phase, const String& current_task);
	static String GetRunbookSummary(const Runbook& rb);
	static String GetWorkGraphSummary(const WorkGraph& wg);
};


#endif
