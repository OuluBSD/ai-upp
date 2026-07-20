#include "VisualStateModel.h"

namespace Upp {

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
	const String& stream, const DistributedStateSnapshot& after)
{
	DistributedServiceResult result = service_.Complete(stream, after);
	for(const String& diagnostic : result.reconstruction.diagnostics)
		diagnostics_.Add(diagnostic);
	return result;
}

bool VsmDistributedReconstructionAdapter::GetAuthoritative(
	const String& stream, DistributedStateSnapshot& out) const
{
	return service_.GetAuthoritative(stream, out);
}

} // namespace Upp
