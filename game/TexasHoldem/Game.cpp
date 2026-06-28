#include "TexasHoldem.h"

NAMESPACE_UPP

NLTHGame::NLTHGame(int num_players, int starting_stack, int small_blind, int big_blind)
	: num_players(num_players), starting_stack(starting_stack), small_blind(small_blind), big_blind(big_blind)
{
	players.Clear();
	for (int i = 0; i < num_players; i++) {
		players.Add(PlayerState(i, "Player " + AsString(i), starting_stack, i == 0));
	}

	dealer_idx = -1;
	pot = 0;
	round = ROUND_FINISHED;
	active_player_idx = -1;
	small_blind_idx = -1;
	big_blind_idx = -1;
}

void NLTHGame::new_hand() {
	dealer_idx = (dealer_idx + 1) % num_players;

	// Populate and shuffle deck
	deck.Clear();
	for (int i = 0; i < 52; i++) {
		deck.Add(Card(i));
	}
	
	// Shuffle deck
	for (int i = deck.GetCount() - 1; i > 0; i--) {
		int j = Random(i + 1);
		Swap(deck[i], deck[j]);
	}

	for (auto& p : players) {
		p.hole_cards.Clear();
		p.current_bet = 0;
		p.total_bet = 0;
		p.folded = false;
		p.all_in = false;
	}

	// Deal hole cards
	for (int round = 0; round < 2; round++) {
		for (int i = 0; i < num_players; i++) {
			int idx = (dealer_idx + 1 + i) % num_players;
			players[idx].hole_cards.Add(deck.Pop());
		}
	}

	if (num_players == 2) {
		small_blind_idx = dealer_idx;
		big_blind_idx = (dealer_idx + 1) % num_players;
	} else {
		small_blind_idx = (dealer_idx + 1) % num_players;
		big_blind_idx = (dealer_idx + 2) % num_players;
	}

	_post_blind(small_blind_idx, small_blind);
	_post_blind(big_blind_idx, big_blind);

	pot = 0;
	board.Clear();
	side_pots.Clear();
	round = ROUND_PREFLOP;

	int starting_player = (big_blind_idx + 1) % num_players;
	current_betting_round.Create(players, big_blind, starting_player);
	active_player_idx = current_betting_round->current_player_idx;
}

void NLTHGame::_post_blind(int p_idx, int amount) {
	PlayerState& p = players[p_idx];
	int actual = amount;
	if (p.stack < amount) {
		actual = p.stack;
	}
	p.stack -= actual;
	p.current_bet = actual;
	p.total_bet = actual;
	if (p.stack == 0) {
		p.all_in = true;
	}
}

Vector<ActionInfo> NLTHGame::valid_actions(int player_idx) {
	if (current_betting_round && !current_betting_round->is_round_over()) {
		return current_betting_round->valid_actions(player_idx);
	}
	return Vector<ActionInfo>();
}

void NLTHGame::apply_action(int player_idx, const String& action, int amount) {
	if (current_betting_round) {
		current_betting_round->apply_action(player_idx, action, amount);
		active_player_idx = current_betting_round->current_player_idx;

		if (current_betting_round->is_round_over()) {
			_advance_street();
		}
	}
}

void NLTHGame::_advance_street() {
	for (auto& p : players) {
		p.current_bet = 0;
	}

	int non_folded_count = 0;
	int non_folded_player_idx = -1;
	for (int i = 0; i < players.GetCount(); i++) {
		if (!players[i].folded) {
			non_folded_count++;
			non_folded_player_idx = i;
		}
	}

	if (non_folded_count == 1) {
		_resolve_early_win(non_folded_player_idx);
		return;
	}

	if (round == ROUND_PREFLOP) {
		deal_flop();
	} else if (round == ROUND_FLOP) {
		deal_turn();
	} else if (round == ROUND_TURN) {
		deal_river();
	} else if (round == ROUND_RIVER) {
		round = ROUND_SHOWDOWN;
		active_player_idx = -1;
		finish_showdown();
	}

	if (round == ROUND_FLOP || round == ROUND_TURN || round == ROUND_RIVER) {
		int starting_player = (dealer_idx + 1) % num_players;
		current_betting_round.Create(players, big_blind, starting_player);
		if (players[starting_player].folded || players[starting_player].all_in) {
			current_betting_round->_advance_player();
		}
		active_player_idx = current_betting_round->current_player_idx;

		if (active_player_idx == -1) {
			_advance_street();
		}
	}
}

void NLTHGame::_resolve_early_win(int winner_idx) {
	PotResult res = collect_pot(players);
	players[winner_idx].stack += res.total_pot;
	round = ROUND_FINISHED;
	active_player_idx = -1;
	for (auto& p : players) {
		p.total_bet = 0;
		p.current_bet = 0;
	}
}

void NLTHGame::deal_flop() {
	deck.Pop(); // burn card
	for (int i = 0; i < 3; i++) {
		board.Add(deck.Pop());
	}
	round = ROUND_FLOP;
}

void NLTHGame::deal_turn() {
	deck.Pop(); // burn card
	board.Add(deck.Pop());
	round = ROUND_TURN;
}

void NLTHGame::deal_river() {
	deck.Pop(); // burn card
	board.Add(deck.Pop());
	round = ROUND_RIVER;
}

Vector<int> NLTHGame::finish_showdown() {
	PotResult res = collect_pot(players);
	Vector<int> all_winners;

	for (const auto& sp : res.side_pots) {
		int amount = sp.amount;
		const Vector<int>& eligible_idxs = sp.eligible_players;

		Vector<int> sp_winners = find_winners(players, eligible_idxs, board);

		int win_share = amount / sp_winners.GetCount();
		int remainder = amount % sp_winners.GetCount();

		for (int w_idx : sp_winners) {
			players[w_idx].stack += win_share;
			all_winners.Add(w_idx);
		}

		if (remainder > 0) {
			players[sp_winners[0]].stack += remainder;
		}
	}

	round = ROUND_FINISHED;
	for (auto& p : players) {
		p.total_bet = 0;
		p.current_bet = 0;
	}
	return all_winners;
}

HandState NLTHGame::get_state() {
	PotResult res = collect_pot(players);
	HandState st;
	st.round = round;
	st.board <<= board;
	st.pot = res.total_pot;
	st.side_pots <<= res.side_pots;
	st.dealer = dealer_idx;
	st.small_blind_idx = small_blind_idx;
	st.big_blind_idx = big_blind_idx;
	st.active_player = active_player_idx;
	st.players <<= players;
	return st;
}

END_UPP_NAMESPACE
