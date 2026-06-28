#ifndef _game_TexasHoldem_State_h_
#define _game_TexasHoldem_State_h_

enum TexasRound {
	ROUND_PREFLOP = 0,
	ROUND_FLOP = 1,
	ROUND_TURN = 2,
	ROUND_RIVER = 3,
	ROUND_SHOWDOWN = 4,
	ROUND_FINISHED = 5
};

struct Card : public Moveable<Card> {
	int id;

	Card() : id(-1) {}
	Card(int id) : id(id) {}

	int GetRank() const { return (id / 4) + 2; }
	int GetSuit() const { return (id % 4) + 1; }

	String GetRankStr() const {
		static const char* RANKS[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A" };
		int r = GetRank();
		if (r >= 2 && r <= 14) return RANKS[r - 2];
		return "?";
	}

	String GetSuitStr() const {
		static const char* SUITS[] = { "s", "h", "c", "d" };
		int s = GetSuit();
		if (s >= 1 && s <= 4) return SUITS[s - 1];
		return "?";
	}

	String ToString() const {
		return GetRankStr() + GetSuitStr();
	}

	String GetAssetName() const {
		static const char* SUITS[] = { "spades", "hearts", "clubs", "diamonds" };
		static const char* RANKS[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king", "ace" };
		int r = GetRank();
		int s = GetSuit();
		if (r >= 2 && r <= 14 && s >= 1 && s <= 4) {
			return String(SUITS[s - 1]) + "_" + RANKS[r - 2];
		}
		return "?";
	}

	bool operator==(const Card& other) const { return id == other.id; }
	bool operator!=(const Card& other) const { return id != other.id; }
};

struct PlayerState : public MoveableAndDeepCopyOption<PlayerState> {
	int idx;
	String name;
	int stack;
	Vector<Card> hole_cards;
	int current_bet;
	int total_bet;
	bool folded;
	bool all_in;
	bool is_human;

	PlayerState() : idx(-1), stack(0), current_bet(0), total_bet(0), folded(false), all_in(false), is_human(false) {}
	PlayerState(int idx, const String& name, int stack, bool is_human)
		: idx(idx), name(name), stack(stack), current_bet(0), total_bet(0), folded(false), all_in(false), is_human(is_human) {}
	
	// U++ Deep Copy Constructor
	PlayerState(const PlayerState& p, int) {
		idx = p.idx;
		name = p.name;
		stack = p.stack;
		hole_cards <<= p.hole_cards;
		current_bet = p.current_bet;
		total_bet = p.total_bet;
		folded = p.folded;
		all_in = p.all_in;
		is_human = p.is_human;
	}

	// Move operations
	PlayerState(PlayerState&&) = default;
	PlayerState& operator=(PlayerState&&) = default;
};

struct SidePot : public MoveableAndDeepCopyOption<SidePot> {
	int amount;
	Vector<int> eligible_players;

	SidePot() : amount(0) {}

	// U++ Deep Copy Constructor
	SidePot(const SidePot& s, int) {
		amount = s.amount;
		eligible_players <<= s.eligible_players;
	}

	// Move operations
	SidePot(SidePot&&) = default;
	SidePot& operator=(SidePot&&) = default;
};

struct HandState : public MoveableAndDeepCopyOption<HandState> {
	TexasRound round;
	Vector<Card> board;
	int pot;
	Vector<SidePot> side_pots;
	int dealer;
	int small_blind_idx;
	int big_blind_idx;
	int active_player;
	Vector<PlayerState> players;

	HandState() : round(ROUND_FINISHED), pot(0), dealer(-1), small_blind_idx(-1), big_blind_idx(-1), active_player(-1) {}

	// U++ Deep Copy Constructor
	HandState(const HandState& h, int) {
		round = h.round;
		board <<= h.board;
		pot = h.pot;
		side_pots <<= h.side_pots;
		dealer = h.dealer;
		small_blind_idx = h.small_blind_idx;
		big_blind_idx = h.big_blind_idx;
		active_player = h.active_player;
		players <<= h.players;
	}

	// Move operations
	HandState(HandState&&) = default;
	HandState& operator=(HandState&&) = default;
};

#endif
