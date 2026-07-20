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

class DistributedEventReconstructor {
public:
	DistributedReconstructionResult Reconstruct(
		const DistributedStateSnapshot& before,
		const DistributedStateSnapshot& after,
		const Vector<DistributedActionObservation>& observed) const;
};

#endif
