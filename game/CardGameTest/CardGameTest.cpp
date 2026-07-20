#include <Core/Core.h>
#include <CardGame/CardGame.h>

using namespace Upp;

void TestSuitFollowing() {
	GameState state;
	state.Deal();
	
	state.players[0].Clear();
	state.players[0].Add(Card("clubs", "2"));
	state.players[0].Add(Card("hearts", "3"));
	
	state.players[1].Clear();
	state.players[1].Add(Card("clubs", "5"));
	state.players[1].Add(Card("diamonds", "2"));

	state.turn = 0;
	state.phase = "PLAYING";
	state.hearts_broken = false;
	state.trick.Clear();
	state.leading_suit = "";

	PlayResult r1 = state.PlayCard(0, Card("clubs", "2"));
	ASSERT(r1.success);

	// Player 1 has clubs, so trying to play diamonds must fail
	PlayResult r2 = state.PlayCard(1, Card("diamonds", "2"));
	ASSERT(!r2.success);

	// Playing clubs must succeed
	PlayResult r3 = state.PlayCard(1, Card("clubs", "5"));
	ASSERT(r3.success);

	Cout() << "✓ TestSuitFollowing passed\n";
}

void TestHeartsBreaking() {
	GameState state;
	state.Deal();
	
	state.players[0].Clear();
	state.players[0].Add(Card("clubs", "3"));
	state.players[0].Add(Card("hearts", "3"));

	state.turn = 0;
	state.phase = "PLAYING";
	state.hearts_broken = false;
	state.trick.Clear();
	state.leading_suit = "";

	// Cannot lead hearts when not broken
	PlayResult r1 = state.PlayCard(0, Card("hearts", "3"));
	ASSERT(!r1.success);

	// Can lead other suit
	PlayResult r2 = state.PlayCard(0, Card("clubs", "3"));
	ASSERT(r2.success);

	Cout() << "✓ TestHeartsBreaking passed\n";
}

void TestTrickResolution() {
	GameState state;
	state.Deal();

	state.players[0].Clear(); state.players[0].Add(Card("clubs", "2"));
	state.players[1].Clear(); state.players[1].Add(Card("clubs", "5"));
	state.players[2].Clear(); state.players[2].Add(Card("clubs", "jack"));
	state.players[3].Clear(); state.players[3].Add(Card("clubs", "3"));

	state.turn = 0;
	state.phase = "PLAYING";
	state.trick.Clear();
	state.leading_suit = "";

	state.PlayCard(0, Card("clubs", "2"));
	state.PlayCard(1, Card("clubs", "5"));
	state.PlayCard(2, Card("clubs", "jack"));
	state.PlayCard(3, Card("clubs", "3"));

	ASSERT(state.trick_pending);
	int pts = 0;
	int winner = state.GetTrickResult(pts);
	ASSERT(winner == 2); // Jack of clubs should win
	ASSERT(pts == 0);

	state.ResolveTrick();
	ASSERT(state.turn == 2);

	Cout() << "✓ TestTrickResolution passed\n";
}

void TestPointsCalculation() {
	GameState state;
	state.Deal();

	state.players[0].Clear(); state.players[0].Add(Card("clubs", "2"));
	state.players[1].Clear(); state.players[1].Add(Card("hearts", "5"));
	state.players[2].Clear(); state.players[2].Add(Card("spades", "queen"));
	state.players[3].Clear(); state.players[3].Add(Card("clubs", "jack"));

	state.turn = 0;
	state.phase = "PLAYING";
	state.trick.Clear();
	state.leading_suit = "";

	state.PlayCard(0, Card("clubs", "2"));
	state.PlayCard(1, Card("hearts", "5")); // Discard heart (1 pt)
	state.PlayCard(2, Card("spades", "queen")); // Discard Q-Spades (13 pts)
	state.PlayCard(3, Card("clubs", "jack")); // Wins trick

	int pts = 0;
	int winner = state.GetTrickResult(pts);
	ASSERT(winner == 3); // Jack of clubs wins (highest lead suit)
	ASSERT(pts == 14); // 1 (heart) + 13 (Q-spades)

	Cout() << "✓ TestPointsCalculation passed\n";
}

void TestAI() {
	GameState state;
	state.Deal();

	// AI plays when leading
	Vector<Card> hand;
	hand.Add(Card("hearts", "5"));
	hand.Add(Card("diamonds", "10"));
	hand.Add(Card("spades", "queen"));

	Vector<TrickItem> trick;
	Card chosen = HeartsAI::ChooseCard(0, hand, trick, "", false);
	ASSERT(chosen.suit == "diamonds"); // Cannot lead hearts, and should not lead Q-Spades if possible

	Cout() << "✓ TestAI passed\n";
}

void TestDistributedPassiveReconstruction() {
	DistributedStateSnapshot before, after;
	before.phase = after.phase = "phase-1";
	before.total = after.total = 10;
	for(int i = 0; i < 3; i++) {
		DistributedParticipantState& a = before.participants.Add();
		DistributedParticipantState& b = after.participants.Add();
		a.active = b.active = true;
	}
	Vector<DistributedActionObservation> observed;
	DistributedActionObservation& action = observed.Add();
	action.participant = 0;
	action.kind = DISTRIBUTED_ACTION_PASSIVE;
	DistributedReconstructionResult result =
		DistributedEventReconstructor().Reconstruct(before, after, observed);
	ASSERT(result.complete);
	ASSERT(!result.ambiguous);
	ASSERT(result.actions.GetCount() == 3);
	ASSERT(result.actions[1].inferred);
	ASSERT(result.actions[2].inferred);
	Cout() << "TestDistributedPassiveReconstruction passed\n";
}

void TestDistributedIncreaseGap() {
	DistributedStateSnapshot before, after;
	before.phase = after.phase = "phase-1";
	before.total = 10;
	after.total = 20;
	for(int i = 0; i < 2; i++) {
		DistributedParticipantState& a = before.participants.Add();
		DistributedParticipantState& b = after.participants.Add();
		a.active = b.active = true;
	}
	Vector<DistributedActionObservation> observed;
	DistributedActionObservation& action = observed.Add();
	action.participant = 0;
	action.kind = DISTRIBUTED_ACTION_INCREASE;
	DistributedReconstructionResult result =
		DistributedEventReconstructor().Reconstruct(before, after, observed);
	ASSERT(!result.complete);
	ASSERT(result.ambiguous);
	Cout() << "TestDistributedIncreaseGap passed\n";
}

void TestDistributedUnobservedRemoval() {
	DistributedStateSnapshot before, after;
	before.phase = after.phase = "phase-1";
	before.total = after.total = 10;
	for(int i = 0; i < 2; i++) {
		DistributedParticipantState& a = before.participants.Add();
		DistributedParticipantState& b = after.participants.Add();
		a.active = true;
		b.active = i == 0;
	}
	DistributedReconstructionResult result =
		DistributedEventReconstructor().Reconstruct(before, after, {});
	ASSERT(!result.complete);
	ASSERT(result.ambiguous);
	Cout() << "TestDistributedUnobservedRemoval passed\n";
}

void TestDistributedSameTimeBatch() {
	DistributedStateSnapshot before, after;
	before.phase = after.phase = "phase-1";
	before.total = after.total = 10;
	for(int i = 0; i < 2; i++) {
		DistributedParticipantState& a = before.participants.Add();
		DistributedParticipantState& b = after.participants.Add();
		a.active = b.active = true;
	}
	Vector<DistributedActionObservation> observed;
	for(int i = 0; i < 2; i++) {
		DistributedActionObservation& action = observed.Add();
		action.participant = i;
		action.kind = DISTRIBUTED_ACTION_PASSIVE;
		action.timestamp = 100;
	}
	DistributedReconstructionResult result =
		DistributedEventReconstructor().Reconstruct(before, after, observed);
	ASSERT(result.complete);
	ASSERT(result.actions.GetCount() == 2);
	ASSERT(result.actions[0].observation.timestamp == result.actions[1].observation.timestamp);
	Cout() << "TestDistributedSameTimeBatch passed\n";
}

void TestDistributedDuplicateObservation() {
	DistributedStateSnapshot before, after;
	before.phase = after.phase = "phase-1";
	before.total = after.total = 10;
	DistributedParticipantState& a = before.participants.Add();
	DistributedParticipantState& b = after.participants.Add();
	a.active = b.active = true;
	Vector<DistributedActionObservation> observed;
	for(int i = 0; i < 2; i++) {
		DistributedActionObservation& action = observed.Add();
		action.participant = 0;
		action.kind = DISTRIBUTED_ACTION_PASSIVE;
		action.timestamp = 100 + i;
	}
	DistributedReconstructionResult result =
		DistributedEventReconstructor().Reconstruct(before, after, observed);
	ASSERT(result.complete);
	ASSERT(result.diagnostics.GetCount() == 1);
	Cout() << "TestDistributedDuplicateObservation passed\n";
}

void TestDistributedPhaseGap() {
	DistributedStateSnapshot before, after;
	before.phase = "phase-1";
	after.phase = "phase-2";
	before.total = after.total = 10;
	DistributedParticipantState& a = before.participants.Add();
	DistributedParticipantState& b = after.participants.Add();
	a.active = b.active = true;
	DistributedReconstructionResult result =
		DistributedEventReconstructor().Reconstruct(before, after, {});
	ASSERT(!result.complete);
	ASSERT(result.ambiguous);
	ASSERT(result.diagnostics.GetCount() == 1);
	Cout() << "TestDistributedPhaseGap passed\n";
}

void TestDistributedObservedRemoval() {
	DistributedStateSnapshot before, after;
	before.phase = after.phase = "phase-1";
	before.total = after.total = 10;
	DistributedParticipantState& a = before.participants.Add();
	DistributedParticipantState& b = after.participants.Add();
	a.active = true;
	b.active = false;
	Vector<DistributedActionObservation> observed;
	DistributedActionObservation& action = observed.Add();
	action.participant = 0;
	action.kind = DISTRIBUTED_ACTION_REMOVE;
	DistributedReconstructionResult result =
		DistributedEventReconstructor().Reconstruct(before, after, observed);
	ASSERT(result.complete);
	ASSERT(!result.ambiguous);
	Cout() << "TestDistributedObservedRemoval passed\n";
}

DistributedBufferedEvent MakeBufferedEvent(const char* identity, int64 sequence,
	                                         int participant)
{
	DistributedBufferedEvent event;
	event.identity = identity;
	event.sequence = sequence;
	event.timestamp = sequence * 10;
	event.observation.participant = participant;
	event.observation.kind = DISTRIBUTED_ACTION_PASSIVE;
	return event;
}

void TestDistributedUniquePhaseCompletion() {
	DistributedStateSnapshot before, after;
	before.phase = "phase-1";
	after.phase = "phase-2";
	before.total = after.total = 10;
	for(int i = 0; i < 2; i++) {
		DistributedParticipantState& a = before.participants.Add();
		DistributedParticipantState& b = after.participants.Add();
		a.active = b.active = true;
	}
	Vector<DistributedActionObservation> observed;
	DistributedActionObservation& action = observed.Add();
	action.participant = 0;
	action.kind = DISTRIBUTED_ACTION_PASSIVE;
	DistributedReconstructionResult result =
		DistributedEventReconstructor().Reconstruct(before, after, observed);
	ASSERT(result.complete);
	ASSERT(result.actions.GetCount() == 2);
	ASSERT(result.actions[1].inferred);
	Cout() << "TestDistributedUniquePhaseCompletion passed\n";
}

void TestDistributedEventBufferOrdering() {
	DistributedEventBuffer buffer;
	ASSERT(buffer.Push(MakeBufferedEvent("second", 2, 1)));
	ASSERT(buffer.Push(MakeBufferedEvent("first", 1, 0)));
	DistributedEventBufferResult result = buffer.Drain();
	ASSERT(result.batches.GetCount() == 2);
	ASSERT(result.batches[0].order_key == 1);
	ASSERT(result.batches[1].order_key == 2);
	Cout() << "TestDistributedEventBufferOrdering passed\n";
}

void TestDistributedEventBufferSameTimeBatch() {
	DistributedEventBuffer buffer;
	DistributedBufferedEvent second = MakeBufferedEvent("second", -1, 1);
	DistributedBufferedEvent first = MakeBufferedEvent("first", -1, 0);
	second.timestamp = first.timestamp = 100;
	ASSERT(buffer.Push(second));
	ASSERT(buffer.Push(first));
	DistributedEventBufferResult result = buffer.Drain();
	ASSERT(result.batches.GetCount() == 1);
	ASSERT(result.batches[0].events.GetCount() == 2);
	ASSERT(result.batches[0].events[0].identity == "first");
	Cout() << "TestDistributedEventBufferSameTimeBatch passed\n";
}

void TestDistributedEventBufferConflictsAndLateEvents() {
	DistributedEventBuffer buffer;
	DistributedBufferedEvent event = MakeBufferedEvent("stable", 5, 0);
	ASSERT(buffer.Push(event));
	buffer.Drain();
	ASSERT(!buffer.Push(event));
	event.observation.participant = 1;
	ASSERT(!buffer.Push(event));
	ASSERT(buffer.Push(MakeBufferedEvent("late", 4, 0)));
	DistributedEventBufferResult result = buffer.Drain();
	ASSERT(result.conflicts.GetCount() == 1);
	ASSERT(result.batches.GetCount() == 1);
	ASSERT(!result.batches[0].events[0].authoritative);
	ASSERT(result.batches[0].events[0].late);
	Cout() << "TestDistributedEventBufferConflictsAndLateEvents passed\n";
}

void TestDistributedEventBufferIndependentStreams() {
	DistributedEventBuffer buffer;
	DistributedBufferedEvent left = MakeBufferedEvent("same", 1, 0);
	DistributedBufferedEvent right = left;
	left.stream = "stream-a";
	right.stream = "stream-b";
	ASSERT(buffer.Push(left));
	ASSERT(buffer.Push(right));
	DistributedEventBufferResult result = buffer.Drain();
	ASSERT(result.conflicts.IsEmpty());
	ASSERT(result.batches.GetCount() == 1);
	ASSERT(result.batches[0].events.GetCount() == 2);
	Cout() << "TestDistributedEventBufferIndependentStreams passed\n";
}

void TestDistributedEventBufferOverflow() {
	DistributedEventBuffer buffer(1);
	ASSERT(buffer.Push(MakeBufferedEvent("one", 1, 0)));
	ASSERT(!buffer.Push(MakeBufferedEvent("two", 2, 1)));
	DistributedEventBufferResult result = buffer.Drain();
	ASSERT(result.batches.GetCount() == 1);
	ASSERT(result.overflow);
	ASSERT(result.diagnostics.GetCount() == 1);
	Cout() << "TestDistributedEventBufferOverflow passed\n";
}

CONSOLE_APP_MAIN {
	Cout() << "Running C++ CardGame tests...\n";
	TestSuitFollowing();
	TestHeartsBreaking();
	TestTrickResolution();
	TestPointsCalculation();
	TestAI();
	TestDistributedPassiveReconstruction();
	TestDistributedIncreaseGap();
	TestDistributedUnobservedRemoval();
	TestDistributedSameTimeBatch();
	TestDistributedDuplicateObservation();
	TestDistributedPhaseGap();
	TestDistributedObservedRemoval();
	TestDistributedUniquePhaseCompletion();
	TestDistributedEventBufferOrdering();
	TestDistributedEventBufferSameTimeBatch();
	TestDistributedEventBufferConflictsAndLateEvents();
	TestDistributedEventBufferIndependentStreams();
	TestDistributedEventBufferOverflow();
	Cout() << "All CardGame tests passed! 🐍💎✨🚀❤️🃏\n";
}
