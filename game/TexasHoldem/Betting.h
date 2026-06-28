#ifndef _game_TexasHoldem_Betting_h_
#define _game_TexasHoldem_Betting_h_

struct ActionInfo : public Moveable<ActionInfo> {
	String action; // "fold", "check", "call", "bet", "raise"
	int amount;
	int min_amount;
	int max_amount;
};

class BettingRound {
public:
	Vector<PlayerState>& players;
	int big_blind;
	int current_player_idx;
	int highest_bet;
	int min_raise;
	bool round_over;
	Vector<bool> has_acted;

	BettingRound(Vector<PlayerState>& players, int big_blind, int starting_player_idx);
	
	Vector<ActionInfo> valid_actions(int player_idx);
	void apply_action(int player_idx, const String& action, int amount);
	void _advance_player();
	void _check_round_over();
	bool is_round_over() const { return round_over; }
};

struct PotResult {
	int total_pot;
	Vector<SidePot> side_pots;
};

PotResult collect_pot(const Vector<PlayerState>& players);

#endif
