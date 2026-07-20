#include "CardGame.h"

NAMESPACE_UPP

static bool SameAmount(double a, double b)
{
	return a >= 0 && b >= 0 && fabs(a - b) < 0.001;
}

static void AddDiagnostic(DistributedReconstructionResult& result, const String& text)
{
	result.diagnostics.Add(text);
}

DistributedReconstructionResult DistributedEventReconstructor::Reconstruct(
	const DistributedStateSnapshot& before,
	const DistributedStateSnapshot& after,
	const Vector<DistributedActionObservation>& observed) const
{
	DistributedReconstructionResult result;
	if(before.participants.GetCount() != after.participants.GetCount()) {
		result.invalid = true;
		AddDiagnostic(result, "participant count changed");
		return result;
	}

	const int count = before.participants.GetCount();
	Vector<bool> covered;
	covered.SetCount(count, false);
	bool increase_seen = false;
	for(const DistributedActionObservation& action : observed) {
		if(action.participant < 0 || action.participant >= count) {
			result.invalid = true;
			AddDiagnostic(result, "observation references an invalid participant");
			continue;
		}
		if(covered[action.participant]) {
			AddDiagnostic(result, Format("participant %d has multiple observations", action.participant));
		}
		covered[action.participant] = true;
		if(action.kind == DISTRIBUTED_ACTION_INCREASE)
			increase_seen = true;
		DistributedReconstructedAction& copy = result.actions.Add();
		copy.observation = action;
		copy.inferred = false;
		copy.reason = "observed";
	}

	Vector<int> missing_active;
	Vector<int> removed;
	for(int i = 0; i < count; i++) {
		if(before.participants[i].active && !after.participants[i].active && !covered[i])
			removed.Add(i);
		if(before.participants[i].active && after.participants[i].active && !covered[i])
			missing_active.Add(i);
	}

	if(!removed.IsEmpty()) {
		result.ambiguous = true;
		AddDiagnostic(result, "participant removal is unobserved");
	}

	bool total_unchanged = SameAmount(before.total, after.total);
	bool phase_unchanged = before.phase == after.phase;
	if(!missing_active.IsEmpty()) {
		if(!phase_unchanged) {
			result.ambiguous = true;
			AddDiagnostic(result, "phase changed while participant actions were missing");
		}
		else if(increase_seen || !total_unchanged) {
			result.ambiguous = true;
			AddDiagnostic(result, "missing actions coexist with an unresolved increase or total change");
		}
		else {
			for(int participant : missing_active) {
				DistributedReconstructedAction& inferred = result.actions.Add();
				inferred.observation.participant = participant;
				inferred.observation.kind = DISTRIBUTED_ACTION_PASSIVE;
				inferred.observation.timestamp = after.total >= 0 ? 0 : -1;
				inferred.inferred = true;
				inferred.reason = "only passive completion preserves phase and total";
			}
		}
	}

	result.complete = !result.invalid && !result.ambiguous;
	return result;
}

END_UPP_NAMESPACE
