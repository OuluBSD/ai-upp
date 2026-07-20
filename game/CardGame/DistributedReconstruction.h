#ifndef _game_CardGame_DistributedReconstruction_h_
#define _game_CardGame_DistributedReconstruction_h_

enum DistributedActionKind {
	DISTRIBUTED_ACTION_PASSIVE,
	DISTRIBUTED_ACTION_MATCH,
	DISTRIBUTED_ACTION_INCREASE,
	DISTRIBUTED_ACTION_REMOVE,
};

struct DistributedParticipantState : Moveable<DistributedParticipantState> {
	bool active = false;
	double committed = -1;
};

struct DistributedStateSnapshot : Moveable<DistributedStateSnapshot> {
	String phase;
	Vector<DistributedParticipantState> participants;
	double total = -1;
};

struct DistributedActionObservation : Moveable<DistributedActionObservation> {
	int participant = -1;
	DistributedActionKind kind = DISTRIBUTED_ACTION_PASSIVE;
	int64 timestamp = -1;
	bool amount_known = false;
	double amount = -1;
	String source;
};

struct DistributedReconstructedAction : Moveable<DistributedReconstructedAction> {
	DistributedActionObservation observation;
	bool inferred = false;
	String reason;
};

struct DistributedReconstructionResult : Moveable<DistributedReconstructionResult> {
	Vector<DistributedReconstructedAction> actions;
	Vector<String> diagnostics;
	bool complete = false;
	bool ambiguous = false;
	bool invalid = false;
};

struct DistributedBufferedEvent : Moveable<DistributedBufferedEvent> {
	String stream;
	String identity;
	int64 sequence = -1;
	int64 timestamp = -1;
	DistributedActionObservation observation;
	bool authoritative = true;
	bool late = false;
};

struct DistributedEventBatch : Moveable<DistributedEventBatch> {
	int64 order_key = -1;
	bool sequence_order = false;
	Vector<DistributedBufferedEvent> events;
};

struct DistributedEventBufferResult : Moveable<DistributedEventBufferResult> {
	Vector<DistributedEventBatch> batches;
	Vector<DistributedBufferedEvent> conflicts;
	Vector<String> diagnostics;
	bool overflow = false;
};

struct DistributedServiceResult : Moveable<DistributedServiceResult> {
	String stream;
	DistributedEventBufferResult buffered;
	DistributedReconstructionResult reconstruction;
	bool authoritative_applied = false;
};

class DistributedEventBuffer {
	int max_events = 256;
	Vector<DistributedBufferedEvent> pending;
	Vector<DistributedBufferedEvent> conflicts;
	Vector<DistributedBufferedEvent> seen;
	Vector<String> diagnostics;
	Index<String> identities;
	bool overflow = false;
	int64 last_order_key = -1;
	bool last_sequence_order = false;
	bool has_last_order = false;

	static bool Equivalent(const DistributedBufferedEvent& a,
	                       const DistributedBufferedEvent& b);
	static String IdentityKey(const DistributedBufferedEvent& event);
	static int Compare(const DistributedBufferedEvent& a,
	                   const DistributedBufferedEvent& b);

public:
	DistributedEventBuffer() = default;
	DistributedEventBuffer(int max_events);

	bool Push(const DistributedBufferedEvent& event);
	DistributedEventBufferResult Drain();
	void Clear();
	int GetPendingCount() const { return pending.GetCount(); }
};

class DistributedReconstructionService {
	struct StreamState : Moveable<StreamState> {
		String id;
		DistributedStateSnapshot before;
		DistributedStateSnapshot authoritative;
		DistributedEventBuffer buffer;
		bool has_authoritative = false;
	};

	Vector<StreamState> streams;
	StreamState* Find(const String& id);
	const StreamState* Find(const String& id) const;

public:
	void Begin(const String& stream, const DistributedStateSnapshot& before);
	bool Observe(const String& stream, const DistributedBufferedEvent& event);
	DistributedServiceResult Complete(const String& stream,
	                                  const DistributedStateSnapshot& after);
	bool GetAuthoritative(const String& stream, DistributedStateSnapshot& out) const;
};

class DistributedEventReconstructor {
public:
	DistributedReconstructionResult Reconstruct(
		const DistributedStateSnapshot& before,
		const DistributedStateSnapshot& after,
		const Vector<DistributedActionObservation>& observed) const;
};

#endif
