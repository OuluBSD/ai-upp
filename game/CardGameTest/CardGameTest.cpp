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

CONSOLE_APP_MAIN {
	Cout() << "Running C++ CardGame tests...\n";
	TestSuitFollowing();
	TestHeartsBreaking();
	TestTrickResolution();
	TestPointsCalculation();
	TestAI();
	Cout() << "All CardGame tests passed! 🐍💎✨🚀❤️🃏\n";
}