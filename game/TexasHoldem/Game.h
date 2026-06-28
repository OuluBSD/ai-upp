#ifndef _game_TexasHoldem_Game_h_
#define _game_TexasHoldem_Game_h_

class NLTHGame {
public:
	int num_players;
	int starting_stack;
	int small_blind;
	int big_blind;

	Vector<PlayerState> players;
	int dealer_idx;
	Vector<Card> board;
	int pot;
	Vector<SidePot> side_pots;
	TexasRound round;
	int active_player_idx;
	Vector<Card> deck;

	int small_blind_idx;
	int big_blind_idx;
	One<BettingRound> current_betting_round;

	NLTHGame(int num_players, int starting_stack, int small_blind, int big_blind);

	void new_hand();
	Vector<ActionInfo> valid_actions(int player_idx);
	void apply_action(int player_idx, const String& action, int amount);
	void _post_blind(int p_idx, int amount);
	void _advance_street();
	void _resolve_early_win(int winner_idx);
	void deal_flop();
	void deal_turn();
	void deal_river();
	Vector<int> finish_showdown();
	HandState get_state();
	bool is_hand_over() const { return round == ROUND_FINISHED; }
};

#endif
