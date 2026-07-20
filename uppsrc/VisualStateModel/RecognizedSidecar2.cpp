#include "VisualStateModel.h"

DistributedStateSnapshot VsmRecognizedSidecar2::MakeSnapshot(
	const String& phase, const bool active[6], const double committed[6], double total)
{
	DistributedStateSnapshot snapshot;
	snapshot.phase = phase;
	snapshot.total = total;
	for(int i = 0; i < 6; i++) {
		DistributedParticipantState& participant = snapshot.participants.Add();
		participant.active = active[i];
		participant.committed = committed[i];
	}
	return snapshot;
}

static void CopySidecar2Legality(DistributedLegalityReport& dst,
	const DistributedLegalityReport& src)
{
	dst.stream = src.stream;
	dst.round = src.round;
	dst.timestamp = src.timestamp;
	dst.status = src.status;
	dst.override_applied = src.override_applied;
	dst.override_reason = src.override_reason;
	dst.issues.Clear();
	for(const DistributedLegalityIssue& issue : src.issues) {
		DistributedLegalityIssue& copy = dst.issues.Add();
		copy.code = issue.code;
		copy.message = issue.message;
	}
}

void VsmRecognizedSidecar2::BeginHand(const String& stream, int hand,
	int64 timestamp_seconds, const DistributedStateSnapshot& before,
	const String& event_text)
{
	ASSERT(!stream.IsEmpty());
	ASSERT(hand >= 1);
	ASSERT(!open_);
	stream_ = stream;
	hand_ = hand;
	next_sequence_ = 0;
	open_ = true;
	adapter_.Begin(stream_, before);
	hand_marker_ = lines_.GetCount();
	DistributedSidecar2Line& marker = lines_.Add();
	marker.stream = stream_;
	marker.timestamp_seconds = timestamp_seconds;
	marker.text = event_text;
	marker.hand = hand_;
	marker.hand_start = true;
}

bool VsmRecognizedSidecar2::AddEvent(int64 timestamp_seconds, const String& event_text)
{
	if(!open_ || event_text.IsEmpty())
		return false;
	DistributedSidecar2Line& line = lines_.Add();
	line.stream = stream_;
	line.timestamp_seconds = timestamp_seconds;
	line.text = event_text;
	return true;
}

bool VsmRecognizedSidecar2::AddAction(int64 timestamp_seconds, int participant,
	DistributedActionKind kind, bool amount_known, double amount,
	const String& event_text)
{
	if(!open_ || participant < 0 || participant >= 6 || event_text.IsEmpty())
		return false;
	VsmDistributedObservation observation;
	observation.stream = stream_;
	observation.identity = Format("hand%d-action%d", hand_, (int)next_sequence_);
	observation.participant = participant;
	observation.kind = kind;
	observation.sequence = next_sequence_++;
	observation.timestamp = timestamp_seconds;
	observation.amount_known = amount_known;
	observation.amount = amount;
	observation.source = "video-recognition";
	if(!adapter_.Observe(observation))
		return false;
	return AddEvent(timestamp_seconds, event_text);
}

VsmRecognizedSidecar2HandResult VsmRecognizedSidecar2::EndHand(
	int64 timestamp_seconds, const DistributedStateSnapshot& after)
{
	ASSERT(open_);
	VsmRecognizedSidecar2HandResult result;
	if(!open_)
		return result;
	DistributedServiceResult service = adapter_.Complete(stream_, after, hand_,
	                                                     timestamp_seconds);
	for(const DistributedReconstructedAction& action : service.reconstruction.actions) {
		if(!action.inferred)
			continue;
		DistributedSidecar2Line& line = lines_.Add();
		line.stream = stream_;
		line.timestamp_seconds = timestamp_seconds;
		line.hand = hand_;
		line.comment = true;
		line.text = Format("inferred participant%d passive reason=\"%s\"",
		                   action.observation.participant + 1, action.reason);
	}
	CopySidecar2Legality(result.legality, service.legality);
	result.authoritative = service.authoritative_applied;
	ASSERT(hand_marker_ >= 0 && hand_marker_ < lines_.GetCount());
	CopySidecar2Legality(lines_[hand_marker_].legality, result.legality);
	open_ = false;
	return result;
}

void VsmRecognizedSidecar2::EndOpenHand(int64 timestamp_seconds,
	const DistributedStateSnapshot& after)
{
	if(open_)
		EndHand(timestamp_seconds, after);
}

String VsmRecognizedSidecar2::Generate() const
{
	return DistributedSidecar2Writer().Generate(lines_);
}
