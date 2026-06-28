#include <Core/Core.h>
#include <CardGame/CardGame.h>

using namespace Upp;

void TestSuitFollowing() {
	GameState state;
	state.Deal();
	state.players[0].Clear();
	state.players[0].Add(Card("clubs", "2"));
	state.players[0].Add(Card("hearts", "3"));
	state.players[0].Add(Card("spades", "4"));

	state.players[1].Clear();
	state.players[1].Add(Card("clubs", "5"));
	state.players[1].Add(Card("diamonds", "2"));
	state.players[1].Add(Card("hearts", "10"));

	state.players[2].Clear();
	state.players[2].Add(Card("clubs", "jack"));
	state.players[2].Add(Card("hearts", "ace"));
	state.players[2].Add(Card("spades", "queen"));

	state.players[3].Clear();
	state.players[3].Add(Card("clubs", "ace"));
	state.players[3].Add(Card("diamonds", "ace"));
	state.players[3].Add(Card("spades", "ace"));

	state.turn = 0;
	state.phase = "PLAYING";
	state.hearts_broken = false;
	state.trick.Clear();
	state.leading_suit = "";

	Card c2("clubs", "2");
	PlayResult r1 = state.PlayCard(0, c2);
	ASSERT(r1.success);

	// Player 1 must follow suit (clubs)
	Card d2("diamonds", "2");
	PlayResult r2 = state.PlayCard(1, d2);
	ASSERT(!r2.success); // Invalid: must follow suit

	Card c5("clubs", "5");
	PlayResult r3 = state.PlayCard(1, c5);
	ASSERT(r3.success); // Valid: followed suit
	
	Cout() << "✓ TestSuitFollowing passed\n";
}

void TestHeartsBreaking() {
	GameState state;
	state.Deal();
	state.players[0].Clear();
	state.players[0].Add(Card("clubs", "2"));
	state.players[0].Add(Card("hearts", "3"));
	state.players[0].Add(Card("spades", "4"));

	state.turn = 0;
	state.phase = "PLAYING";
	state.hearts_broken = false;
	state.trick.Clear();
	state.leading_suit = "";

	// Cannot lead hearts when not broken
	Card h3("hearts", "3");
	PlayResult r1 = state.PlayCard(0, h3);
	ASSERT(!r1.success);

	// Can lead when broken
	state.hearts_broken = true;
	PlayResult r2 = state.PlayCard(0, h3);
	ASSERT(r2.success);

	Cout() << "✓ TestHeartsBreaking passed\n";
}

void TestTrickResolution() {
	GameState state;
	state.Deal();
	state.players[0].Clear(); state.players[0].Add(Card("clubs", "2"));
	state.players[1].Clear(); state.players[1].Add(Card("clubs", "5"));
	state.players[2].Clear(); state.players[2].Add(Card("clubs", "jack"));
	state.players[3].Clear(); state.players[3].Add(Card("clubs", "ace"));

	state.turn = 0;
	state.phase = "PLAYING";
	state.hearts_broken = false;
	state.trick.Clear();
	state.leading_suit = "";

	ASSERT(state.PlayCard(0, Card("clubs", "2")).success);
	ASSERT(state.PlayCard(1, Card("clubs", "5")).success);
	ASSERT(state.PlayCard(2, Card("clubs", "jack")).success);
	ASSERT(state.PlayCard(3, Card("clubs", "ace")).success);

	ASSERT(state.trick_pending);
	state.ResolveTrick();

	// Player 3 played highest club, should win and take next turn
	ASSERT(state.turn == 3);

	Cout() << "✓ TestTrickResolution passed\n";
}

void TestPointsCalculation() {
	GameState state;
	state.Deal();
	state.trick.Clear();
	state.trick.Add(TrickItem(0, Card("clubs", "2")));
	state.trick.Add(TrickItem(1, Card("hearts", "5")));      // 1 pt
	state.trick.Add(TrickItem(2, Card("spades", "queen")));  // 13 pts
	state.trick.Add(TrickItem(3, Card("clubs", "ace")));
	state.leading_suit = "clubs";
	state.trick_pending = true;
	state.pending_trick_winner = 3;
	state.pending_trick_points = 14;

	state.ResolveTrick();

	ASSERT(state.last_round_scores[3] == 0); // round_scores won't update last_round_scores until round ends, but trick resolved round_scores
	ASSERT(state.round_scores[3] == 14);

	Cout() << "✓ TestPointsCalculation passed\n";
}

void TestAI() {
	GameState state;
	state.Deal();
	
	// Test passing chooser
	Vector<Card> hand;
	hand <<= state.players[0];
	Vector<Card> pass;
	pass <<= HeartsAI::ChoosePassCards(hand);
	ASSERT(pass.GetCount() == 3);

	// Test play card chooser leading
	Card lead = HeartsAI::ChooseCard(0, hand, state.trick, state.leading_suit, state.hearts_broken);
	ASSERT(!lead.id.IsEmpty());

	// Test play card chooser following
	state.leading_suit = "clubs";
	Card follow = HeartsAI::ChooseCard(0, hand, state.trick, state.leading_suit, state.hearts_broken);
	ASSERT(!follow.id.IsEmpty());

	Cout() << "✓ TestAI passed\n";
}

CONSOLE_APP_MAIN {
	StdLogSetup(LOG_COUT | LOG_FILE);
	Cout() << "Running C++ CardGame tests...\n";
	
	TestSuitFollowing();
	TestHeartsBreaking();
	TestTrickResolution();
	TestPointsCalculation();
	TestAI();

	Cout() << "All CardGame tests passed! 🐍💎✨🚀❤️🃏\n";
}
