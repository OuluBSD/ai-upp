#ifndef MAESTRO_WORKGRAPHSCORER_H
#define MAESTRO_WORKGRAPHSCORER_H

// NOTE: This header is normally included inside namespace Upp

#ifndef _Maestro_WorkGraphScorer_h_
#define _Maestro_WorkGraphScorer_h_

#include <Maestro/PlanModels.h>

namespace Upp {

struct ScoreResult : Moveable<ScoreResult> {
	String task_id;
	String task_title;
	double score = 0;
	int    effort_bucket = 0;
	int    impact = 0;
	int    risk = 0;
	int    purpose = 0;
	String rationale;
	Vector<String> inferred_fields;
	
	void Jsonize(JsonIO& jio) {
		jio("task_id", task_id)("task_title", task_title)("score", score)
		   ("effort_bucket", effort_bucket)("impact", impact)("risk", risk)
		   ("purpose", purpose)("rationale", rationale)("inferred_fields", inferred_fields);
	}
};

struct RankedWorkGraph : Moveable<RankedWorkGraph> {
	String workgraph_id;
	String profile;
	Array<ScoreResult> ranked_tasks;
	ValueMap summary;
	
	void Jsonize(JsonIO& jio) {
		jio("workgraph_id", workgraph_id)("profile", profile)
		   ("ranked_tasks", ranked_tasks)("summary", summary);
	}
};

class WorkGraphScorer {
public:
	static RankedWorkGraph Rank(const WorkGraph& wg, const String& profile = "default");
	static ScoreResult ScoreTask(const WorkGraphTask& task, const String& profile, const String& domain);

private:
	static int InferEffort(const WorkGraphTask& t);
	static int InferImpact(const WorkGraphTask& t, const String& domain);
	static int InferRisk(const WorkGraphTask& t);
	static int InferPurpose(const WorkGraphTask& t);
};

}

#endif

#endif // MAESTRO_WORKGRAPHSCORER_H
