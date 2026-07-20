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

static void CopySnapshot(DistributedStateSnapshot& dst,
	                      const DistributedStateSnapshot& src)
{
	dst.phase = src.phase;
	dst.total = src.total;
	dst.participants.Clear();
	for(const DistributedParticipantState& participant : src.participants) {
		DistributedParticipantState& copy = dst.participants.Add();
		copy.active = participant.active;
		copy.committed = participant.committed;
	}
}

DistributedEventBuffer::DistributedEventBuffer(int max_events)
{
	ASSERT(max_events > 0);
	this->max_events = max_events > 0 ? max_events : 1;
}

bool DistributedEventBuffer::Equivalent(const DistributedBufferedEvent& a,
	                                      const DistributedBufferedEvent& b)
{
	const DistributedActionObservation& x = a.observation;
	const DistributedActionObservation& y = b.observation;
	return a.stream == b.stream && a.identity == b.identity &&
		a.sequence == b.sequence && a.timestamp == b.timestamp &&
		x.participant == y.participant && x.kind == y.kind &&
		x.amount_known == y.amount_known &&
		(!x.amount_known || fabs(x.amount - y.amount) < 0.001) &&
		x.source == y.source;
}

String DistributedEventBuffer::IdentityKey(const DistributedBufferedEvent& event)
{
	return event.stream + "::" + event.identity;
}

int DistributedEventBuffer::Compare(const DistributedBufferedEvent& a,
	                                  const DistributedBufferedEvent& b)
{
	bool a_sequence = a.sequence >= 0;
	bool b_sequence = b.sequence >= 0;
	if(a_sequence != b_sequence)
		return a_sequence ? -1 : 1;
	int64 ak = a_sequence ? a.sequence : a.timestamp;
	int64 bk = b_sequence ? b.sequence : b.timestamp;
	if(ak != bk)
		return ak < bk ? -1 : 1;
	if(a.identity != b.identity)
		return a.identity < b.identity ? -1 : 1;
	return 0;
}

bool DistributedEventBuffer::Push(const DistributedBufferedEvent& input)
{
	DistributedBufferedEvent event = input;
	if(!event.identity.IsEmpty()) {
		String key = IdentityKey(event);
		int id = identities.Find(key);
		if(id >= 0) {
			for(const DistributedBufferedEvent& old : seen)
				if(IdentityKey(old) == key) {
					if(!Equivalent(old, event)) {
						conflicts.Add(event);
						diagnostics.Add(Format("conflicting event identity: %s", event.identity));
					}
					return false;
				}
		}
		else
			identities.Add(key);
	}
	if(has_last_order) {
		bool sequence_order = event.sequence >= 0;
		int64 key = sequence_order ? event.sequence : event.timestamp;
		if(sequence_order == last_sequence_order && key < last_order_key) {
			event.late = true;
			event.authoritative = false;
			diagnostics.Add("late event retained as non-authoritative");
		}
	}
	if(pending.GetCount() >= max_events) {
		overflow = true;
		diagnostics.Add("event buffer capacity exceeded");
		return false;
	}
	pending.Add(event);
	seen.Add(event);
	return true;
}

DistributedEventBufferResult DistributedEventBuffer::Drain()
{
	DistributedEventBufferResult result;
	result.overflow = overflow;
	result.conflicts = pick(conflicts);
	result.diagnostics = pick(diagnostics);
	Sort(pending, [](const DistributedBufferedEvent& a, const DistributedBufferedEvent& b) {
		return DistributedEventBuffer::Compare(a, b) < 0;
	});
	for(int i = 0; i < pending.GetCount();) {
		DistributedBufferedEvent first = pending[i];
		DistributedEventBatch& batch = result.batches.Add();
		batch.order_key = first.sequence >= 0 ? first.sequence : first.timestamp;
		batch.sequence_order = first.sequence >= 0;
		while(i < pending.GetCount()) {
			const DistributedBufferedEvent& event = pending[i];
			bool same_order = (event.sequence >= 0) == batch.sequence_order;
			int64 key = event.sequence >= 0 ? event.sequence : event.timestamp;
			if(!same_order || key != batch.order_key)
				break;
			batch.events.Add(event);
			i++;
		}
		last_order_key = batch.order_key;
		last_sequence_order = batch.sequence_order;
		has_last_order = true;
	}
	pending.Clear();
	return result;
}

void DistributedEventBuffer::Clear()
{
	pending.Clear();
	conflicts.Clear();
	seen.Clear();
	diagnostics.Clear();
	identities.Clear();
	overflow = false;
	last_order_key = -1;
	has_last_order = false;
}

DistributedReconstructionService::StreamState*
DistributedReconstructionService::Find(const String& id)
{
	for(StreamState& stream : streams)
		if(stream.id == id)
			return &stream;
	return nullptr;
}

const DistributedReconstructionService::StreamState*
DistributedReconstructionService::Find(const String& id) const
{
	for(const StreamState& stream : streams)
		if(stream.id == id)
			return &stream;
	return nullptr;
}

void DistributedReconstructionService::Begin(const String& stream,
	                                           const DistributedStateSnapshot& before)
{
	StreamState* state = Find(stream);
	if(!state) {
		state = &streams.Add();
		state->id = stream;
	}
	CopySnapshot(state->before, before);
	CopySnapshot(state->authoritative, before);
	state->has_authoritative = true;
	state->buffer.Clear();
}

bool DistributedReconstructionService::Observe(const String& stream,
	                                             const DistributedBufferedEvent& event)
{
	StreamState* state = Find(stream);
	if(!state)
		return false;
	DistributedBufferedEvent copy = event;
	copy.stream = stream;
	return state->buffer.Push(copy);
}

DistributedServiceResult DistributedReconstructionService::Complete(
	const String& stream, const DistributedStateSnapshot& after)
{
	DistributedServiceResult service;
	service.stream = stream;
	StreamState* state = Find(stream);
	if(!state) {
		service.reconstruction.invalid = true;
		service.reconstruction.diagnostics.Add("stream was not started");
		return service;
	}
	service.buffered = state->buffer.Drain();
	for(const String& diagnostic : service.buffered.diagnostics)
		service.reconstruction.diagnostics.Add(diagnostic);
	if(!service.buffered.conflicts.IsEmpty())
		service.reconstruction.ambiguous = true;
	Vector<DistributedActionObservation> observations;
	for(const DistributedEventBatch& batch : service.buffered.batches)
		for(const DistributedBufferedEvent& event : batch.events)
			observations.Add(event.observation);
	service.reconstruction = DistributedEventReconstructor().Reconstruct(
		state->before, after, observations);
	bool has_late = false;
	for(const DistributedEventBatch& batch : service.buffered.batches)
		for(const DistributedBufferedEvent& event : batch.events)
			has_late |= event.late;
	service.authoritative_applied = service.reconstruction.complete &&
		!service.reconstruction.ambiguous && !service.reconstruction.invalid &&
		!service.buffered.overflow && service.buffered.conflicts.IsEmpty() && !has_late;
	if(service.authoritative_applied) {
		CopySnapshot(state->authoritative, after);
		CopySnapshot(state->before, after);
	}
	return service;
}

bool DistributedReconstructionService::GetAuthoritative(
	const String& stream, DistributedStateSnapshot& out) const
{
	const StreamState* state = Find(stream);
	if(!state || !state->has_authoritative)
		return false;
	CopySnapshot(out, state->authoritative);
	return true;
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
		if(!phase_unchanged && !observed.IsEmpty() && missing_active.GetCount() == 1 &&
			!increase_seen && total_unchanged && removed.IsEmpty()) {
			DistributedReconstructedAction& inferred = result.actions.Add();
			inferred.observation.participant = missing_active[0];
			inferred.observation.kind = DISTRIBUTED_ACTION_PASSIVE;
			inferred.observation.timestamp = -1;
			inferred.inferred = true;
			inferred.reason = "single passive completion is forced by the observed transition";
		}
		else if(!phase_unchanged) {
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
				inferred.observation.timestamp = -1;
				inferred.inferred = true;
				inferred.reason = "only passive completion preserves phase and total";
			}
		}
	}

	result.complete = !result.invalid && !result.ambiguous;
	return result;
}

END_UPP_NAMESPACE
