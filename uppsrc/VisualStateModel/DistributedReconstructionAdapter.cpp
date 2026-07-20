#include "VisualStateModel.h"

namespace Upp {

VsmDistributedReconstructionAdapter::VsmDistributedReconstructionAdapter()
{
	service_.SetLiveAssertion(&assertion_);
}

void VsmDistributedReconstructionAdapter::Begin(
	const String& stream, const DistributedStateSnapshot& before)
{
	service_.Begin(stream, before);
}

bool VsmDistributedReconstructionAdapter::Observe(
	const VsmDistributedObservation& observation)
{
	DistributedBufferedEvent event;
	event.stream = observation.stream;
	event.identity = observation.identity;
	event.sequence = observation.sequence;
	event.timestamp = observation.timestamp;
	event.observation.participant = observation.participant;
	event.observation.kind = observation.kind;
	event.observation.timestamp = observation.timestamp;
	event.observation.amount_known = observation.amount_known;
	event.observation.amount = observation.amount;
	event.observation.source = observation.source;
	return service_.Observe(observation.stream, event);
}

DistributedServiceResult VsmDistributedReconstructionAdapter::Complete(
	const String& stream, const DistributedStateSnapshot& after,
	int64 round, int64 timestamp)
{
	DistributedServiceResult result = service_.Complete(stream, after, round, timestamp);
	for(const String& diagnostic : result.reconstruction.diagnostics)
		diagnostics_.Add(diagnostic);
	for(const DistributedLegalityIssue& issue : result.legality.issues)
		diagnostics_.Add(Format("legality %s: %s", issue.code, issue.message));
	return result;
}

bool VsmDistributedReconstructionAdapter::ApplyOverride(
	const String& stream, const String& reason, int64 timestamp)
{
	return service_.ApplyOverride(stream, reason, timestamp);
}

bool VsmDistributedReconstructionAdapter::GetAuthoritative(
	const String& stream, DistributedStateSnapshot& out) const
{
	return service_.GetAuthoritative(stream, out);
}

} // namespace Upp
