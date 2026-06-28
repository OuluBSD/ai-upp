#include "TexasHoldem.h"

NAMESPACE_UPP

BettingRound::BettingRound(Vector<PlayerState>& players, int big_blind, int starting_player_idx)
	: players(players), big_blind(big_blind), current_player_idx(starting_player_idx)
{
	highest_bet = 0;
	min_raise = big_blind;
	round_over = false;

	has_acted.Clear();
	for (int i = 0; i < players.GetCount(); i++) {
		has_acted.Add(false);
	}

	for (const auto& p : players) {
		if (p.current_bet > highest_bet) {
			highest_bet = p.current_bet;
		}
	}
}

Vector<ActionInfo> BettingRound::valid_actions(int player_idx) {
	const PlayerState& p = players[player_idx];
	Vector<ActionInfo> actions;

	if (!p.folded && !p.all_in) {
		// fold action
		ActionInfo a_fold;
		a_fold.action = "fold";
		a_fold.amount = 0;
		a_fold.min_amount = 0;
		a_fold.max_amount = 0;
		actions.Add(a_fold);

		int call_amount = highest_bet - p.current_bet;

		if (call_amount == 0) {
			ActionInfo a_check;
			a_check.action = "check";
			a_check.amount = 0;
			a_check.min_amount = 0;
			a_check.max_amount = 0;
			actions.Add(a_check);
		} else {
			int actual_call = call_amount;
			if (p.stack < call_amount) {
				actual_call = p.stack;
			}
			ActionInfo a_call;
			a_call.action = "call";
			a_call.amount = actual_call;
			a_call.min_amount = 0;
			a_call.max_amount = 0;
			actions.Add(a_call);
		}

		if (p.stack > call_amount) {
			if (highest_bet == 0) {
				int actual_min = big_blind;
				if (p.stack < big_blind) {
					actual_min = p.stack;
				}
				ActionInfo a_bet;
				a_bet.action = "bet";
				a_bet.amount = 0;
				a_bet.min_amount = actual_min;
				a_bet.max_amount = p.stack;
				actions.Add(a_bet);
			} else {
				int min_raise_total = highest_bet + min_raise;
				if (p.stack + p.current_bet >= min_raise_total) {
					ActionInfo a_raise;
					a_raise.action = "raise";
					a_raise.amount = 0;
					a_raise.min_amount = min_raise_total - p.current_bet;
					a_raise.max_amount = p.stack;
					actions.Add(a_raise);
				} else {
					if (p.stack > call_amount) {
						ActionInfo a_raise2;
						a_raise2.action = "raise";
						a_raise2.amount = 0;
						a_raise2.min_amount = p.stack;
						a_raise2.max_amount = p.stack;
						actions.Add(a_raise2);
					}
				}
			}
		}
	}

	return actions;
}

void BettingRound::apply_action(int player_idx, const String& action, int amount) {
	PlayerState& p = players[player_idx];

	if (action == "fold") {
		p.folded = true;
	} else if (action == "check") {
		// do nothing
	} else if (action == "call") {
		int call_amount = highest_bet - p.current_bet;
		if (p.stack < call_amount) {
			call_amount = p.stack;
		}
		p.stack -= call_amount;
		p.current_bet += call_amount;
		p.total_bet += call_amount;
		if (p.stack == 0) {
			p.all_in = true;
		}
	} else if (action == "bet" || action == "raise") {
		p.stack -= amount;
		p.current_bet += amount;
		p.total_bet += amount;

		if (p.current_bet > highest_bet) {
			int raise_diff = p.current_bet - highest_bet;
			if (raise_diff > min_raise) {
				min_raise = raise_diff;
			}
			highest_bet = p.current_bet;
		}

		if (p.stack == 0) {
			p.all_in = true;
		}
	}

	has_acted[player_idx] = true;
	_advance_player();
	_check_round_over();
}

void BettingRound::_advance_player() {
	int num_players = players.GetCount();
	for (int i = 1; i <= num_players; i++) {
		int next_idx = (current_player_idx + i) % num_players;
		const PlayerState& p = players[next_idx];
		if (!p.folded && !p.all_in) {
			current_player_idx = next_idx;
			return;
		}
	}
	current_player_idx = -1;
}

void BettingRound::_check_round_over() {
	Vector<int> active;
	for (int i = 0; i < players.GetCount(); i++) {
		if (!players[i].folded && !players[i].all_in) {
			active.Add(i);
		}
	}

	if (active.GetCount() <= 1) {
		int total_active_in_hand = 0;
		for (int i = 0; i < players.GetCount(); i++) {
			if (!players[i].folded) {
				total_active_in_hand++;
			}
		}

		if (total_active_in_hand <= 1) {
			round_over = true;
			return;
		}

		if (active.IsEmpty()) {
			round_over = true;
			return;
		}

		int p_idx = active[0];
		if (has_acted[p_idx] && players[p_idx].current_bet == highest_bet) {
			round_over = true;
			return;
		}
	} else {
		for (int p_idx : active) {
			if (!has_acted[p_idx] || players[p_idx].current_bet != highest_bet) {
				return;
			}
		}
		round_over = true;
	}
}

PotResult collect_pot(const Vector<PlayerState>& players) {
	Vector<int> contributions;
	int total_pot = 0;
	for (const PlayerState& p : players) {
		contributions.Add(p.total_bet);
		total_pot += p.total_bet;
	}

	Vector<SidePot> side_pots;
	Vector<int> remaining;
	remaining <<= contributions;

	while (true) {
		Vector<int> active_remaining;
		for (int r : remaining) {
			if (r > 0) {
				active_remaining.Add(r);
			}
		}

		if (active_remaining.IsEmpty()) {
			break;
		}

		int slice_height = active_remaining[0];
		for (int r : active_remaining) {
			if (r < slice_height) {
				slice_height = r;
			}
		}

		Vector<int> eligible_to_win;
		int slice_total_amount = 0;

		for (int i = 0; i < remaining.GetCount(); i++) {
			if (remaining[i] > 0) {
				slice_total_amount += slice_height;
				remaining[i] -= slice_height;
				if (!players[i].folded) {
					eligible_to_win.Add(i);
				}
			}
		}

		if (!eligible_to_win.IsEmpty()) {
			SidePot& sp = side_pots.Add();
			sp.amount = slice_total_amount;
			sp.eligible_players <<= eligible_to_win;
		} else {
			Vector<int> all_non_folded;
			for (int j = 0; j < players.GetCount(); j++) {
				if (!players[j].folded) {
					all_non_folded.Add(j);
				}
			}

			if (!all_non_folded.IsEmpty()) {
				SidePot& sp = side_pots.Add();
				sp.amount = slice_total_amount;
				sp.eligible_players.Add(all_non_folded[0]);
			} else {
				SidePot& sp = side_pots.Add();
				sp.amount = slice_total_amount;
				sp.eligible_players.Add(0);
			}
		}
	}

	PotResult res;
	res.total_pot = total_pot;
	res.side_pots <<= side_pots;
	return res;
}

END_UPP_NAMESPACE
