#include "VideoGameEngineSyncer.h"

using namespace Upp;

NAMESPACE_UPP

class GameEngineSyncer {
public:
	bool tracked_hand_id_known = false;
	int tracked_hand_id = -1;
	bool tracked_dealer_known = false;
	int tracked_dealer_seat = -1;
	bool tracked_street_known = false;
	int tracked_street = -1;
	bool tracked_pot_known = false;
	int tracked_pot = 0;
	bool tracked_board_known = false;
	Vector<int> tracked_board_cards;

	Vector<TrackedPlayer> tracked_players;
	Vector<DivergenceEvent> divergence_events;

	void AddDivergence(int frame_id, const String& field, const Value& expected, const Value& actual, const String& description) {
		DivergenceEvent ev;
		ev.frame_id = frame_id;
		ev.field = field;
		ev.expected = expected;
		ev.actual = actual;
		ev.description = description;
		divergence_events.Add(ev);

		Cerr() << "DIVERGENCE DETECTED at Frame " << frame_id << ":\n"
		       << "  Field:       " << field << "\n"
		       << "  Expected:    " << expected << "\n"
		       << "  Actual:      " << actual << "\n"
		       << "  Description: " << description << "\n\n";
	}

	bool IsBoardEmpty(const Vector<int>& cards) {
		for (int c : cards) {
			if (c != -1) return false;
		}
		return true;
	}

	int GetActiveCardCount(const Vector<int>& cards) {
		int count = 0;
		for (int c : cards) {
			if (c != -1) count++;
		}
		return count;
	}

	String FormatCard(int card) {
		if (card < 0 || card >= 52) return "?";
		static const char* suits[] = { "c", "d", "h", "s" };
		static const char* ranks[] = { "2", "3", "4", "5", "6", "7", "8", "9", "T", "J", "Q", "K", "A" };
		return String(ranks[card % 13]) + suits[card / 13];
	}

	String FormatCards(const Vector<int>& cards) {
		String s = "[";
		for (int i = 0; i < cards.GetCount(); i++) {
			if (i > 0) s << ", ";
			s << FormatCard(cards[i]);
		}
		s << "]";
		return s;
	}

	void ProcessFrame(const TexasHoldemLogicState& state) {
		int fid = state.frame_id;
		Cout() << "Processing Frame " << fid << "...\n";

		if (fid == 5) {
			Cout() << "DEBUG Frame 5:\n"
			       << "  state.board_cards_known: " << (state.board_cards_known ? "true" : "false") << "\n"
			       << "  IsBoardEmpty(state.board_cards): " << (IsBoardEmpty(state.board_cards) ? "true" : "false") << "\n"
			       << "  tracked_board_known: " << (tracked_board_known ? "true" : "false") << "\n"
			       << "  IsBoardEmpty(tracked_board_cards): " << (IsBoardEmpty(tracked_board_cards) ? "true" : "false") << "\n"
			       << "  state.dealer_seat_known: " << (state.dealer_seat_known ? "true" : "false") << "\n"
			       << "  tracked_dealer_known: " << (tracked_dealer_known ? "true" : "false") << "\n"
			       << "  state.dealer_seat: " << state.dealer_seat << "\n"
			       << "  tracked_dealer_seat: " << tracked_dealer_seat << "\n";
		}

		// 1. Hand Start / End detection
		bool new_hand_started = false;
		if (state.hand_id_known && tracked_hand_id_known && state.hand_id != tracked_hand_id) {
			new_hand_started = true;
			Cout() << "  Hand ID changed: " << tracked_hand_id << " -> " << state.hand_id << "\n";
		} else if (state.board_cards_known && IsBoardEmpty(state.board_cards) && tracked_board_known && !IsBoardEmpty(tracked_board_cards)) {
			new_hand_started = true;
			Cout() << "  New hand started (community cards reset)\n";
		} else if (state.dealer_seat_known && tracked_dealer_known && state.dealer_seat != tracked_dealer_seat) {
			new_hand_started = true;
			Cout() << "  New hand started (dealer seat changed: " << tracked_dealer_seat << " -> " << state.dealer_seat << ")\n";
		}

		if (new_hand_started) {
			// Verify community cards are reset
			if (state.board_cards_known && !IsBoardEmpty(state.board_cards)) {
				AddDivergence(fid, "board_cards", FormatCards(Vector<int>()), FormatCards(state.board_cards),
				              Format("New hand started, but community cards are not reset. Expected empty, got %s", FormatCards(state.board_cards)));
			}
			// Verify stacks are updated / chip conservation across hand transition
			if (state.pot_known && tracked_pot_known) {
				int total_prev = tracked_pot;
				int total_curr = state.pot;
				bool all_players_known = true;
				
				Vector<const TexasHoldemLogicPlayerState*> curr_players_by_seat;
				curr_players_by_seat.SetCount(max(tracked_players.GetCount(), 10), nullptr);
				for (const auto& p : state.players) {
					if (p.seat >= 0) {
						if (p.seat >= curr_players_by_seat.GetCount()) {
							curr_players_by_seat.SetCount(p.seat + 1, nullptr);
						}
						curr_players_by_seat[p.seat] = &p;
					}
				}

				for (int seat = 0; seat < max(curr_players_by_seat.GetCount(), tracked_players.GetCount()); ++seat) {
					bool tp_ok = (seat < tracked_players.GetCount() && tracked_players[seat].seat != -1 && tracked_players[seat].stack_known && tracked_players[seat].bet_known);
					bool cp_ok = (seat < curr_players_by_seat.GetCount() && curr_players_by_seat[seat] != nullptr && curr_players_by_seat[seat]->stack_known && curr_players_by_seat[seat]->bet_known);
					if (tp_ok != cp_ok) {
						all_players_known = false;
						break;
					}
					if (tp_ok) {
						total_prev += tracked_players[seat].stack + tracked_players[seat].bet;
						total_curr += curr_players_by_seat[seat]->stack + curr_players_by_seat[seat]->bet;
					}
				}
				if (all_players_known && total_curr != total_prev) {
					AddDivergence(fid, "total_chips", total_prev, total_curr,
					              Format("Total chips not conserved across hand transition. Prev total: %d, Curr total: %d (pot: %d -> %d)",
					                     total_prev, total_curr, tracked_pot, state.pot));
				}
			}
		}

		// 2. Dealer seat shifting check
		if (state.dealer_seat_known && tracked_dealer_known && state.dealer_seat != tracked_dealer_seat) {
			if (!new_hand_started) {
				Cout() << "  Dealer seat changed: " << tracked_dealer_seat << " -> " << state.dealer_seat << "\n";
				int max_seat = -1;
				for (const auto& p : state.players) {
					if (p.seat > max_seat) max_seat = p.seat;
				}
				for (const auto& tp : tracked_players) {
					if (tp.seat > max_seat) max_seat = tp.seat;
				}
				int num_seats = max_seat + 1;
				if (num_seats > 1) {
					int next_expected = -1;
					for (int i = 1; i < num_seats; i++) {
						int candidate = (tracked_dealer_seat + i) % num_seats;
						bool is_active = false;
						bool active_known_flag = false;
						for (const auto& p : state.players) {
							if (p.seat == candidate) {
								if (p.active_known) {
									is_active = p.active;
									active_known_flag = true;
								}
								break;
							}
						}
						if (!active_known_flag && candidate < tracked_players.GetCount()) {
							const auto& tp = tracked_players[candidate];
							if (tp.seat != -1 && tp.active_known) {
								is_active = tp.active;
								active_known_flag = true;
							}
						}
						if (active_known_flag && is_active) {
							next_expected = candidate;
							break;
						}
					}
					if (next_expected != -1 && state.dealer_seat != next_expected) {
						AddDivergence(fid, "dealer_seat", next_expected, state.dealer_seat,
						              Format("Dealer seat transitioned from %d to %d, but expected next active seat clockwise is %d",
						                     tracked_dealer_seat, state.dealer_seat, next_expected));
					}
				}
			}
		}

		// 3. Street Progression & community cards
		bool street_transitioned = false;
		if (state.street_known && tracked_street_known && state.street != tracked_street) {
			Cout() << "  Street changed: " << tracked_street << " -> " << state.street << "\n";
			if (!new_hand_started) {
				street_transitioned = true;
				if (state.street < tracked_street) {
					AddDivergence(fid, "street", tracked_street, state.street,
					              Format("Street went backwards from %d to %d in the same hand", tracked_street, state.street));
				}
			}
		}

		if (state.board_cards_known && tracked_board_known) {
			if (!new_hand_started) {
				int tracked_count = GetActiveCardCount(tracked_board_cards);
				int state_count = GetActiveCardCount(state.board_cards);
				if (state_count < tracked_count) {
					AddDivergence(fid, "board_cards_count", tracked_count, state_count,
					              Format("Community card count decreased from %d to %d in the same hand", tracked_count, state_count));
				}
				int check_count = min(tracked_board_cards.GetCount(), state.board_cards.GetCount());
				for (int i = 0; i < check_count; i++) {
					if (tracked_board_cards[i] != -1 && state.board_cards[i] != -1 && state.board_cards[i] != tracked_board_cards[i]) {
						AddDivergence(fid, "board_card_" + AsString(i), tracked_board_cards[i], state.board_cards[i],
						              Format("Community card at index %d changed from %d (%s) to %d (%s) in the same hand",
						                     i, tracked_board_cards[i], FormatCard(tracked_board_cards[i]),
						                     state.board_cards[i], FormatCard(state.board_cards[i])));
					}
				}
			}
		}

		if (state.street_known && state.board_cards_known) {
			int active_count = GetActiveCardCount(state.board_cards);
			if (state.street == 0 && active_count != 0) {
				AddDivergence(fid, "board_cards", 0, active_count,
				              Format("Street is Preflop, but board has %d cards instead of 0", active_count));
			} else if (state.street == 1 && active_count != 3) {
				AddDivergence(fid, "board_cards", 3, active_count,
				              Format("Street is Flop, but board has %d cards instead of 3", active_count));
			} else if (state.street == 2 && active_count != 4) {
				AddDivergence(fid, "board_cards", 4, active_count,
				              Format("Street is Turn, but board has %d cards instead of 4", active_count));
			} else if (state.street == 3 && active_count != 5) {
				AddDivergence(fid, "board_cards", 5, active_count,
				              Format("Street is River, but board has %d cards instead of 5", active_count));
			}
		}

		// 4. Conservation of Chips & Action Legitimacy & Inactive players
		Vector<const TexasHoldemLogicPlayerState*> curr_players_by_seat;
		curr_players_by_seat.SetCount(max(tracked_players.GetCount(), 10), nullptr);
		for (const auto& p : state.players) {
			if (p.seat >= 0) {
				if (p.seat >= curr_players_by_seat.GetCount()) {
					curr_players_by_seat.SetCount(p.seat + 1, nullptr);
				}
				curr_players_by_seat[p.seat] = &p;
			}
		}

		// Check street transitions: verify bets reset and sum added to pot
		if (street_transitioned && state.pot_known && tracked_pot_known) {
			int sum_prev_bets = 0;
			bool all_bets_known = true;
			for (int seat = 0; seat < tracked_players.GetCount(); seat++) {
				const auto& tp = tracked_players[seat];
				if (tp.seat != -1) {
					if (tp.bet_known) {
						sum_prev_bets += tp.bet;
					} else {
						all_bets_known = false;
					}
				}
			}
			if (all_bets_known) {
				int expected_pot = tracked_pot + sum_prev_bets;
				if (state.pot != expected_pot) {
					AddDivergence(fid, "pot", expected_pot, state.pot,
					              Format("Street transitioned, expected pot to increase by sum of player bets (%d) to %d, but got %d",
					                     sum_prev_bets, expected_pot, state.pot));
				}
			}
		}

		// Individual player validations
		for (int seat = 0; seat < max(curr_players_by_seat.GetCount(), tracked_players.GetCount()); seat++) {
			bool cp_valid = (seat < curr_players_by_seat.GetCount() && curr_players_by_seat[seat] != nullptr);
			bool tp_valid = (seat < tracked_players.GetCount() && tracked_players[seat].seat != -1);
			if (!cp_valid || !tp_valid) continue;

			const auto& cp = *curr_players_by_seat[seat];
			const auto& tp = tracked_players[seat];

			// Action legitimacy: bet/raise exceeding stack
			if (cp.bet_known && tp.bet_known && tp.stack_known) {
				if (cp.bet > tp.bet) {
					int bet_increase = cp.bet - tp.bet;
					if (bet_increase > tp.stack) {
						AddDivergence(fid, Format("player_%d_bet", seat), tp.stack, bet_increase,
						              Format("Player %d bet/raise amount %d exceeds their remaining stack %d", seat, bet_increase, tp.stack));
					}
				}
			}

			// Inactive / Folded checks
			bool cp_active = cp.active_known ? cp.active : tp.active;
			bool tp_active = tp.active_known ? tp.active : false;

			if (!cp_active) {
				if (!tp_active) {
					// Remained inactive
					if (cp.stack_known && tp.stack_known && cp.stack != tp.stack) {
						AddDivergence(fid, Format("player_%d_stack", seat), tp.stack, cp.stack,
						              Format("Inactive Player %d stack changed from %d to %d", seat, tp.stack, cp.stack));
					}
					if (cp.bet_known && tp.bet_known && cp.bet != tp.bet) {
						AddDivergence(fid, Format("player_%d_bet", seat), tp.bet, cp.bet,
						              Format("Inactive Player %d bet changed from %d to %d", seat, tp.bet, cp.bet));
					}
				} else {
					// Folded in this frame
					if (cp.stack_known && tp.stack_known && cp.stack != tp.stack) {
						AddDivergence(fid, Format("player_%d_stack", seat), tp.stack, cp.stack,
						              Format("Folding Player %d stack changed from %d to %d", seat, tp.stack, cp.stack));
					}
					if (!street_transitioned && cp.bet_known && tp.bet_known && cp.bet != tp.bet) {
						AddDivergence(fid, Format("player_%d_bet", seat), tp.bet, cp.bet,
						              Format("Folding Player %d bet changed from %d to %d in the same street", seat, tp.bet, cp.bet));
					}
				}
			} else {
				// Player is active in current frame
				if (street_transitioned) {
					// Bet must reset to 0
					if (cp.bet_known && cp.bet != 0) {
						AddDivergence(fid, Format("player_%d_bet", seat), 0, cp.bet,
						              Format("Street transitioned, but active Player %d bet was not reset to 0 (got %d)", seat, cp.bet));
					}
					// Stack must stay the same as previous
					if (cp.stack_known && tp.stack_known && cp.stack != tp.stack) {
						AddDivergence(fid, Format("player_%d_stack", seat), tp.stack, cp.stack,
						              Format("Street transitioned, but active Player %d stack changed from %d to %d", seat, tp.stack, cp.stack));
					}
				} else {
					// Normal frame (no street transition): verify conservation of player chips (unless pot payout)
					bool is_payout = (state.pot_known && tracked_pot_known && state.pot < tracked_pot);
					if (!new_hand_started && !is_payout) {
						if (cp.stack_known && tp.stack_known && cp.bet_known && tp.bet_known) {
							int prev_sum = tp.stack + tp.bet;
							int curr_sum = cp.stack + cp.bet;
							if (curr_sum != prev_sum) {
								AddDivergence(fid, Format("player_%d_conservation", seat), prev_sum, curr_sum,
								              Format("Player %d stack change (%d -> %d) does not match bet change (%d -> %d). Conservation sum: expected %d, got %d",
								                     seat, tp.stack, cp.stack, tp.bet, cp.bet, prev_sum, curr_sum));
							}
						}
					}
				}
			}
		}

		// 5. Total Chip Conservation (within hand, no payout)
		if (!new_hand_started && state.pot_known && tracked_pot_known) {
			bool is_payout = (state.pot < tracked_pot);
			if (!is_payout) {
				int total_prev = tracked_pot;
				int total_curr = state.pot;
				bool all_players_known = true;

				for (int seat = 0; seat < max(curr_players_by_seat.GetCount(), tracked_players.GetCount()); ++seat) {
					bool tp_ok = (seat < tracked_players.GetCount() && tracked_players[seat].seat != -1 && tracked_players[seat].stack_known && tracked_players[seat].bet_known);
					bool cp_ok = (seat < curr_players_by_seat.GetCount() && curr_players_by_seat[seat] != nullptr && curr_players_by_seat[seat]->stack_known && curr_players_by_seat[seat]->bet_known);
					if (tp_ok != cp_ok) {
						all_players_known = false;
						break;
					}
					if (tp_ok) {
						total_prev += tracked_players[seat].stack + tracked_players[seat].bet;
						total_curr += curr_players_by_seat[seat]->stack + curr_players_by_seat[seat]->bet;
					}
				}
				if (all_players_known && total_curr != total_prev) {
					AddDivergence(fid, "total_chips", total_prev, total_curr,
					              Format("Total table chips not conserved between frames. Prev total: %d, Curr total: %d (pot: %d -> %d)",
					                     total_prev, total_curr, tracked_pot, state.pot));
				}
			}
		}

		// Update tracked state
		if (state.hand_id_known) {
			tracked_hand_id_known = true;
			tracked_hand_id = state.hand_id;
		}
		if (state.dealer_seat_known) {
			tracked_dealer_known = true;
			tracked_dealer_seat = state.dealer_seat;
		}
		if (state.street_known) {
			tracked_street_known = true;
			tracked_street = state.street;
		}
		if (state.pot_known) {
			tracked_pot_known = true;
			tracked_pot = state.pot;
		}
		if (state.board_cards_known) {
			tracked_board_known = true;
			tracked_board_cards = clone(state.board_cards);
		}

		for (const auto& cp : state.players) {
			if (cp.seat >= 0) {
				if (cp.seat >= tracked_players.GetCount()) {
					tracked_players.SetCount(cp.seat + 1);
				}
				auto& tp = tracked_players[cp.seat];
				tp.seat = cp.seat;
				if (cp.active_known) {
					tp.active_known = true;
					tp.active = cp.active;
				}
				if (cp.stack_known) {
					tp.stack_known = true;
					tp.stack = cp.stack;
				}
				if (cp.bet_known) {
					tp.bet_known = true;
					tp.bet = cp.bet;
				}
				if (cp.hole_cards_known) {
					tp.hole_cards_known = true;
					tp.hole_cards = clone(cp.hole_cards);
				}
			}
		}
	}
};

END_UPP_NAMESPACE

GUI_APP_MAIN {
#ifdef PLATFORM_WIN32
	AttachConsole(ATTACH_PARENT_PROCESS);
#endif
	SetVppLogName(AppendFileName(GetCurrentDirectory(), "VideoGameEngineSyncer.log"));
	RLOG("VideoGameEngineSyncer started");

	const Vector<String>& args = CommandLine();
	SaveFile("C:\\Users\\sblo\\Dev\\ai-upp\\syncer_debug.txt", "syncer started\nargs count: " + AsString(args.GetCount()) + "\n");
	String logic_jsonl_path;
	String out_divergences_path;

	for (int i = 0; i < args.GetCount(); i++) {
		if (args[i] == "--logic-jsonl" && i + 1 < args.GetCount()) {
			logic_jsonl_path = args[++i];
		} else if (args[i] == "--out-divergences" && i + 1 < args.GetCount()) {
			out_divergences_path = args[++i];
		}
	}

	if (logic_jsonl_path.IsEmpty() || out_divergences_path.IsEmpty()) {
		SaveFile("C:\\Users\\sblo\\Dev\\ai-upp\\syncer_debug.txt", "args empty\n");
		Cout() << "VideoGameEngineSyncer CLI\n"
		       << "Usage: VideoGameEngineSyncer --logic-jsonl <input_file.jsonl> --out-divergences <output_file.json>\n";
		SetExitCode(1);
		return;
	}

	SaveFile("C:\\Users\\sblo\\Dev\\ai-upp\\syncer_debug.txt", "args ok: " + logic_jsonl_path + ", out: " + out_divergences_path + "\n");

	if (!FileExists(logic_jsonl_path)) {
		SaveFile("C:\\Users\\sblo\\Dev\\ai-upp\\syncer_debug.txt", "file does not exist: " + logic_jsonl_path + "\n");
		Cerr() << "Input logic-jsonl file does not exist: " << logic_jsonl_path << "\n";
		SetExitCode(1);
		return;
	}

	SaveFile("C:\\Users\\sblo\\Dev\\ai-upp\\syncer_debug.txt", "file exists. loading...\n");

	Cout() << "Loading input records from " << logic_jsonl_path << "...\n";
	String content = LoadFile(logic_jsonl_path);
	Vector<String> lines = Split(content, '\n', false);

	SaveFile("C:\\Users\\sblo\\Dev\\ai-upp\\syncer_debug.txt", "loaded " + AsString(lines.GetCount()) + " lines\n");

	GameEngineSyncer syncer;
	int parsed_count = 0;

	for (int i = 0; i < lines.GetCount(); i++) {
		String line = TrimBoth(lines[i]);
		if (line.IsEmpty()) continue;

		TexasHoldemLogicState state;
		VsmProductionRecordOutWrapper wrapper;
		if (LoadFromJson(wrapper, line) && wrapper.logic_state.frame_id != -1) {
			state = pick(wrapper.logic_state);
			if (state.frame_id == -1) {
				state.frame_id = wrapper.frame_id;
			}
		} else {
			if (!LoadFromJson(state, line)) {
				SaveFile("C:\\Users\\sblo\\Dev\\ai-upp\\syncer_debug.txt", "failed to parse line " + AsString(i) + "\n");
				Cerr() << "Failed to parse JSON line " << i + 1 << ": " << line << "\n";
				SetExitCode(1);
				return;
			}
		}

		if (state.frame_id == -1) {
			state.frame_id = parsed_count;
		}

		syncer.ProcessFrame(state);
		parsed_count++;
	}

	SaveFile("C:\\Users\\sblo\\Dev\\ai-upp\\syncer_debug.txt", "processed " + AsString(parsed_count) + " frames. divergences: " + AsString(syncer.divergence_events.GetCount()) + "\n");

	Cout() << "Processed " << parsed_count << " frames.\n";
	Cout() << "Saving " << syncer.divergence_events.GetCount() << " divergence(s) to " << out_divergences_path << "...\n";

	String out_json = StoreAsJson(syncer.divergence_events);
	if (!SaveFile(out_divergences_path, out_json)) {
		SaveFile("C:\\Users\\sblo\\Dev\\ai-upp\\syncer_debug.txt", "failed to save output file to: " + out_divergences_path + "\n");
		Cerr() << "Failed to write output divergences file: " << out_divergences_path << "\n";
		SetExitCode(1);
		return;
	}

	SaveFile("C:\\Users\\sblo\\Dev\\ai-upp\\syncer_debug.txt", "success! saved divergences file to: " + out_divergences_path + "\n");
	Cout() << "Verification completed successfully. Total divergences found: " << syncer.divergence_events.GetCount() << "\n";
}
