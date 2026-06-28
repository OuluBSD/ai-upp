#include "CardGame.h"

NAMESPACE_UPP

TrickItem::TrickItem() : player_index(-1) {}
TrickItem::TrickItem(int p_idx, const Card& c) : player_index(p_idx), card(c) {}

PlayResult::PlayResult(bool s, const String& m) : success(s), message(m) {}

static void ShuffleDeck(Vector<Card>& deck) {
	for (int i = deck.GetCount() - 1; i > 0; i--) {
		int j = Random(i + 1);
		Swap(deck[i], deck[j]);
	}
}

GameState::GameState() {
	for (int i = 0; i < 4; i++) {
		scores[i] = 0;
		round_scores[i] = 0;
		last_round_scores[i] = 0;
	}
	round_number = 0;
	last_trick_winner = -1;
	last_trick_points = 0;
	last_round_moon_shooter = -1;
	game_over = false;
	trick_pending = false;
	pending_trick_winner = -1;
	pending_trick_points = 0;
	hearts_broken = false;
	turn = 0;
	phase = "PASSING";
}

void GameState::Log(const String& msg) {
	if (LogCallback) {
		LogCallback(msg);
	} else {
		Cout() << msg << "\n";
	}
}

void GameState::Deal() {
	round_number++;
	
	// Create standard deck
	Vector<Card> deck;
	static const char* SUITS[] = { "clubs", "diamonds", "hearts", "spades" };
	static const char* RANKS[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king", "ace" };
	for (int s = 0; s < 4; s++) {
		for (int r = 0; r < 13; r++) {
			deck.Add(Card(SUITS[s], RANKS[r]));
		}
	}
	
	ShuffleDeck(deck);
	
	for (int i = 0; i < 4; i++) {
		players[i].Clear();
		round_scores[i] = 0;
		passed_cards[i].Clear();
		last_round_scores[i] = 0;
	}
	
	hearts_broken = false;
	last_trick_winner = -1;
	last_trick_points = 0;
	last_round_moon_shooter = -1;
	game_over = false;
	trick_pending = false;
	pending_trick_winner = -1;
	pending_trick_points = 0;
	trick.Clear();
	leading_suit = "";
	
	for (int i = 0; i < 52; i++) {
		players[i % 4].Add(deck[i]);
	}
	
	int pass_dir = round_number % 4;
	if (pass_dir == 0) {
		phase = "PLAYING";
		StartPlayPhase();
	} else {
		phase = "PASSING";
		Log("Round " + AsString(round_number) + ": Passing phase. Select 3 cards.");
	}
}

bool GameState::SelectPass(int player_index, const Vector<Card>& cards) {
	if (phase != "PASSING") return false;
	if (cards.GetCount() != 3) return false;
	
	passed_cards[player_index] <<= cards;
	
	bool everyone_selected = true;
	for (int i = 0; i < 4; i++) {
		if (passed_cards[i].GetCount() != 3) {
			everyone_selected = false;
			break;
		}
	}
	if (everyone_selected) {
		ExecutePass();
	}
	
	return true;
}

void GameState::ExecutePass() {
	int pass_dir = round_number % 4;
	int offset = 0;
	if (pass_dir == 1) offset = 1;
	else if (pass_dir == 2) offset = -1;
	else if (pass_dir == 3) offset = 2;
	else return;
	
	for (int i = 0; i < 4; i++) {
		for (const Card& c : passed_cards[i]) {
			for (int j = 0; j < players[i].GetCount(); j++) {
				if (players[i][j].id == c.id) {
					players[i].Remove(j);
					break;
				}
			}
		}
	}
	
	for (int i = 0; i < 4; i++) {
		int target = (i + offset + 4) % 4;
		for (const Card& c : passed_cards[i]) {
			players[target].Add(c);
		}
	}
	
	phase = "PLAYING";
	StartPlayPhase();
}

void GameState::StartPlayPhase() {
	for (int i = 0; i < 4; i++) {
		for (const Card& card : players[i]) {
			if (card.suit == "clubs" && card.rank == "2") {
				turn = i;
				Log("Player " + AsString(i) + " has 2 of Clubs and starts.");
				return;
			}
		}
	}
}

PlayResult GameState::ValidatePlay(int player_index, const Card& card) const {
	if (phase != "PLAYING")
		return PlayResult(false, "Not in playing phase");
	if (trick_pending)
		return PlayResult(false, "Waiting for trick resolution");
	if (player_index != turn)
		return PlayResult(false, "Not your turn");
		
	int total_cards_in_hands = 0;
	for (int i = 0; i < 4; i++) {
		total_cards_in_hands += players[i].GetCount();
	}
	bool first_trick = (total_cards_in_hands == 52);
	bool first_lead = (first_trick && leading_suit.IsEmpty());
	
	if (first_lead) {
		if (card.suit != "clubs" || card.rank != "2") {
			return PlayResult(false, "Must lead 2 of Clubs");
		}
	}
	
	// Must follow suit
	if (!leading_suit.IsEmpty()) {
		bool has_suit = false;
		for (const Card& c : players[player_index]) {
			if (c.suit == leading_suit) {
				has_suit = true;
				break;
			}
		}
		if (has_suit && card.suit != leading_suit) {
			return PlayResult(false, "Must follow suit: " + leading_suit);
		}
	}
	
	// Hearts breaking rule
	if (card.suit == "hearts" && !hearts_broken) {
		bool has_other = false;
		for (const Card& c : players[player_index]) {
			if (c.suit != "hearts") {
				has_other = true;
				break;
			}
		}
		if (has_other && leading_suit.IsEmpty()) {
			return PlayResult(false, "Hearts not broken yet");
		}
	}
	
	// First trick special rule: no points on first trick
	if (first_trick && card.GetPoints() > 0) {
		bool has_safe_card = false;
		for (const Card& c : players[player_index]) {
			if (c.GetPoints() == 0) {
				has_safe_card = true;
				break;
			}
		}
		if (has_safe_card) {
			return PlayResult(false, "No point cards allowed on first trick");
		}
	}
	
	return PlayResult(true, "OK");
}

PlayResult GameState::PlayCard(int player_index, const Card& card) {
	PlayResult res = ValidatePlay(player_index, card);
	if (!res.success) {
		return res;
	}
	
	// Remove from hand
	for (int j = 0; j < players[player_index].GetCount(); j++) {
		if (players[player_index][j].id == card.id) {
			players[player_index].Remove(j);
			break;
		}
	}
	
	trick.Add(TrickItem(player_index, card));
	
	if (leading_suit.IsEmpty()) {
		leading_suit = card.suit;
	}
	
	if (card.suit == "hearts" || (card.suit == "spades" && card.rank == "queen")) {
		hearts_broken = true;
	}
	
	if (trick.GetCount() == 4) {
		int points = 0;
		int winner_index = GetTrickResult(points);
		trick_pending = true;
		pending_trick_winner = winner_index;
		pending_trick_points = points;
	} else {
		turn = (turn + 1) % 4;
	}
	
	return PlayResult(true, "OK");
}

int GameState::GetTrickResult(int& points_out) const {
	int winner_index = 0;
	int highest_rank_idx = -1;
	
	for (const TrickItem& item : trick) {
		if (item.card.suit == leading_suit) {
			int r_idx = RankIndex(item.card.rank);
			if (r_idx > highest_rank_idx) {
				highest_rank_idx = r_idx;
				winner_index = item.player_index;
			}
		}
	}
	
	points_out = 0;
	for (const TrickItem& item : trick) {
		points_out += item.card.GetPoints();
	}
	
	return winner_index;
}

void GameState::ResolveTrick() {
	int winner_index = 0;
	int points = 0;
	if (trick_pending) {
		winner_index = pending_trick_winner;
		points = pending_trick_points;
	} else {
		winner_index = GetTrickResult(points);
	}
	
	round_scores[winner_index] += points;
	last_trick_winner = winner_index;
	last_trick_points = points;
	trick_pending = false;
	pending_trick_winner = -1;
	pending_trick_points = 0;
	
	Log("Trick resolved. Player " + AsString(winner_index) + " wins " + AsString(points) + " points.");
	
	trick.Clear();
	leading_suit = "";
	turn = winner_index;
	
	bool round_done = true;
	for (int i = 0; i < 4; i++) {
		if (players[i].GetCount() != 0) {
			round_done = false;
			break;
		}
	}
	if (round_done) {
		ResolveRound();
	}
}

void GameState::ResolveRound() {
	Log("Round finished. Calculating scores...");
	
	int moon_shooter = -1;
	for (int i = 0; i < 4; i++) {
		if (round_scores[i] == 26) {
			moon_shooter = i;
			break;
		}
	}
	
	if (moon_shooter != -1) {
		Log("PLAYER " + AsString(moon_shooter) + " SHOT THE MOON!");
		for (int i = 0; i < 4; i++) {
			if (i == moon_shooter) {
				round_scores[i] = 0;
			} else {
				round_scores[i] = 26;
			}
		}
	}
	
	for (int i = 0; i < 4; i++) {
		last_round_scores[i] = round_scores[i];
	}
	last_round_moon_shooter = moon_shooter;
	
	for (int i = 0; i < 4; i++) {
		scores[i] += round_scores[i];
		Log("Player " + AsString(i) + " total score: " + AsString(scores[i]));
	}
	
	bool is_over = false;
	for (int i = 0; i < 4; i++) {
		if (scores[i] >= 100) {
			is_over = true;
			break;
		}
	}
	game_over = is_over;
	if (game_over) {
		phase = "GAME_OVER";
		Log("GAME OVER!");
	} else {
		phase = "ROUND_END";
	}
}

void GameState::BeginNextRound() {
	if (game_over) return;
	Deal();
}

END_UPP_NAMESPACE
