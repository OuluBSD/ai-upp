#include "VideoGameEngineReplayValidator.h"

#include <TexasHoldem/TexasHoldemLocalGame.h>
#include <Poker/LocalEngineFactory.h>
#include <GameRules/Game.h>
#include <GameRules/GameData.h>
#include <GameRules/PlayerData.h>
#include <GameRules/HandInterface.h>
#include <GameRules/BeroInterface.h>
#include <GameRules/BoardInterface.h>
#include <GameRules/PlayerInterface.h>
#include <GameRules/GuiInterface.h>
#include <GameRules/EngineLog.h>
#include <EditorCommon/ConfigFile.h>

using namespace Upp;

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// Task 0270: drive the REAL Game engine (game/GameRules/Game, game/Poker/
// LocalHand+LocalBoard+LocalBero) with the real, video-parsed action sequence
// from a recorded poker hand, force the real known cards, and compare the
// engine's own resulting pot/board/stacks/bets to the ground truth at every
// step. Unlike reference/VideoGameEngineSyncer (task 0258), which re-implements
// poker rules standalone, this instantiates and runs the production engine.
// ---------------------------------------------------------------------------

// No-op GuiInterface: same shape as RunLocalGame.cpp's file-local ScriptLoopGui
// (which is not exported), so a Game can run fully headless.
class HeadlessGui : public GuiInterface {
public:
	virtual bool isTestMode() const override { return true; }
	virtual void initGui(int) override {}
	virtual void refreshGameLabels(TexasRound) const override {}
	virtual void nextRoundCleanGui() override {}
	virtual void logNewGameHandMsg(int, int) override {}
	virtual void flushLogAtGame(int) override {}
	virtual void logNewBlindsSetsMsg(int, int, String, String) override {}
	virtual void flushLogAtHand() override {}
	virtual void dealHoleCards() override {}
	virtual void refreshPot() override {}
	virtual void refreshSet() override {}
	virtual void nextPlayerAnimation() override {}
	virtual void flipHolecardsAllIn() override {}
	virtual void logDealBoardCardsMsg(int, int, int, int, int = -1, int = -1) override {}
	virtual void refreshGroupbox(int, int) override {}
	virtual void preflopAnimation1() override {}
	virtual void flopAnimation1() override {}
	virtual void turnAnimation1() override {}
	virtual void riverAnimation1() override {}
	virtual void postRiverAnimation1() override {}
	virtual void logPlayerActionMsg(String, int, int) override {}
	virtual void logFlipHoleCardsMsg(String, int, int, int = -1, String = "shows") override {}
	virtual void logWinningHandMsg(String, String, int) override {}
	virtual void dealBeRoCards(TexasRound) override {}
	virtual void beRoAnimation2(TexasRound) override {}
	virtual void meInAction() override {}
	virtual void postRiverRunAnimation1() override {}
	virtual void refreshCash() override {}
	virtual void refreshAction(int, int) override {}
	virtual void SignalNetClientError(int, int) override {}
};

static String FormatCard(int card) {
	if (card < 0 || card >= 52) return "..";
	static const char* suits[] = { "c", "d", "h", "s" };
	static const char* ranks[] = { "2","3","4","5","6","7","8","9","T","J","Q","K","A" };
	return String(ranks[card % 13]) + suits[card / 13];
}
static String FormatCards(const Vector<int>& cards) {
	String s;
	for (int c : cards) if (c >= 0) s << FormatCard(c);
	return s.IsEmpty() ? "-" : s;
}

// A single real action extracted from the ground truth frames.
struct DerivedAction : Moveable<DerivedAction> {
	int frame_id = -1;
	int street = -1;
	int seat = -1;
	int kind = 0;          // 0 = fold, 1 = raise-to (any voluntary chip commitment)
	int target_total = 0;  // seat's total set for this street (only for kind==1)
	String gt_action;      // raw ground-truth action label (fold/raise/bet/call/allin)
	bool consumed = false;
};

static const char* ActionName(int a) {
	switch (a) {
		case 1: return "check"; case 2: return "call"; case 3: return "bet";
		case 4: return "raise"; case 5: return "fold"; case 6: return "allin";
		default: return "none";
	}
}

// Per-seat helpers on a ground-truth record.
static const TexasHoldemLogicPlayerState* SeatOf(const TexasHoldemLogicState& s, int seat) {
	for (const auto& p : s.players) if (p.seat == seat) return &p;
	return nullptr;
}

struct HandFrames : Moveable<HandFrames> {
	int hand_id = -1;
	Vector<const TexasHoldemLogicState*> frames;
};

// ---------------------------------------------------------------------------
// Derivation: walk a hand's frames in order and emit the real voluntary action
// events (folds + chip-commitment increases). Checks carry no chip change and
// are inferred at drive time (a seat the engine puts on turn with no pending
// event must be checking). Blind posts are produced by the engine itself and
// are never emitted here.
static Vector<DerivedAction> DeriveActions(const HandFrames& hf, int num_seats) {
	Vector<DerivedAction> events;
	Vector<int> committed(num_seats, 0);
	Vector<bool> folded(num_seats, false);
	int cur_street = -1;
	for (const TexasHoldemLogicState* fp : hf.frames) {
		const TexasHoldemLogicState& f = *fp;
		if (!f.street_known) continue;
		if (f.street != cur_street) {
			cur_street = f.street;
			for (int i = 0; i < num_seats; i++) committed[i] = 0; // per-street bets reset
		}
		for (int seat = 0; seat < num_seats; seat++) {
			const TexasHoldemLogicPlayerState* p = SeatOf(f, seat);
			if (!p) continue;
			if (p->action_known && p->action == 5 /*FOLD*/ && !folded[seat]) {
				DerivedAction a;
				a.frame_id = f.frame_id; a.street = f.street; a.seat = seat;
				a.kind = 0; a.gt_action = "fold";
				events.Add(a);
				folded[seat] = true;
				continue;
			}
			if (p->bet_known && p->bet > committed[seat]) {
				DerivedAction a;
				a.frame_id = f.frame_id; a.street = f.street; a.seat = seat;
				a.kind = 1; a.target_total = p->bet;
				a.gt_action = p->action_known ? ActionName(p->action) : "commit";
				events.Add(a);
				committed[seat] = p->bet;
			}
		}
	}
	return events;
}

// Find the ground-truth frame a derived action maps to.
static const TexasHoldemLogicState* FrameById(const HandFrames& hf, int frame_id) {
	for (const TexasHoldemLogicState* f : hf.frames) if (f->frame_id == frame_id) return f;
	return nullptr;
}

static void CompareEngineToFrame(const std::shared_ptr<Game>& game, const HandFrames& hf,
                                 int step, int frame_id, int street, int seat_acted,
                                 const String& action, int num_seats, ReplayHandReport& hr) {
	const TexasHoldemLogicState* fp = FrameById(hf, frame_id);
	if (!fp) return;
	const TexasHoldemLogicState& f = *fp;
	auto hand = game->getCurrentHand();
	std::shared_ptr<BoardInterface> board = hand ? hand->getBoard() : nullptr;

	auto emit = [&](const String& field, const String& ev, const String& gv, bool m, const String& note) {
		ReplayFieldCompare c;
		c.step = step; c.frame_id = frame_id; c.street = street;
		c.seat_acted = seat_acted; c.action = action; c.field = field;
		c.engine_value = ev; c.gt_value = gv; c.match = m; c.note = note;
		hr.compares.Add(c);
		hr.compared++;
		if (m) hr.matched++; else hr.mismatched++;
	};

	// Pot. NOTE: the engine's board->getPot() is NOT usable here -- collectSets()
	// is only ever called once per hand (LocalHand.cpp:136, capturing the blinds)
	// while collectPot() (switchRounds) never re-derives sets, so getPot() stays
	// frozen at the blind total for the whole hand. This is a pre-existing engine
	// accounting bug, reproducible in the stock --local-game-script path, NOT
	// something this task introduced. We therefore compare the ground-truth pot
	// against the engine's TRUE money-in-play, which is exactly what the (correct)
	// side-pot code LocalBoard::distributePot() itself uses: committed =
	// roundStartCash - cash, summed over seats. `committed_excl` removes the
	// current street's uncollected bets (the on-screen pot sometimes shows chips
	// still in front of players, sometimes not -- the human-parsed ground truth is
	// not self-consistent on this), so a match under EITHER convention counts.
	if (f.pot_known) {
		int committed = 0, live_sets = 0, buggy = board ? board->getPot() : -1;
		for (int s = 0; s < num_seats; s++) {
			auto p = game->getPlayerByNumber(s);
			if (!p) continue;
			committed += p->getMyRoundStartCash() - p->getMyCash();
			live_sets += p->getMySet();
		}
		int committed_excl = committed - live_sets;
		bool m = (f.pot == committed) || (f.pot == committed_excl);
		int rake_incl = committed - f.pot;
		String note = Format("eng_committed=%d eng_committed_excl_live=%d getPot_buggy=%d gt=%d delta=%d",
		                     committed, committed_excl, buggy, f.pot, rake_incl);
		if (!m && rake_incl > 0 && rake_incl <= 15) note << " (consistent with rake)";
		emit("pot", AsString(committed), AsString(f.pot), m, note);
	}

	// Board cards (engine forced these; confirm the forcing held).
	if (f.board_cards_known && board) {
		const int* bc = board->getMyCards();
		Vector<int> ebd; for (int i = 0; i < 5; i++) ebd.Add(bc ? bc[i] : -1);
		// compare only the ground-truth's revealed (>=0) positions
		bool m = true;
		for (int i = 0; i < f.board_cards.GetCount() && i < 5; i++)
			if (f.board_cards[i] >= 0 && f.board_cards[i] != ebd[i]) m = false;
		emit("board", FormatCards(ebd), FormatCards(f.board_cards), m, "");
	}

	// Per-seat stack + bet.
	for (int seat = 0; seat < num_seats; seat++) {
		const TexasHoldemLogicPlayerState* gp = SeatOf(f, seat);
		if (!gp) continue;
		auto ep = game->getPlayerByNumber(seat);
		if (!ep) continue;
		if (gp->stack_known)
			emit(Format("player_%d_stack", seat), AsString(ep->getMyCash()), AsString(gp->stack),
			     ep->getMyCash() == gp->stack, "");
		if (gp->bet_known)
			emit(Format("player_%d_bet", seat), AsString(ep->getMySet()), AsString(gp->bet),
			     ep->getMySet() == gp->bet, "");
	}
}

// ---------------------------------------------------------------------------
// Replay a single hand through the real engine.
static void ReplayHand(const HandFrames& hf, int hand_index, int num_seats, int small_blind,
                       class ConfigFile& config, ReplayReport& report) {
	ReplayHandReport hr;
	hr.hand_index = hand_index;
	hr.gt_hand_id = hf.hand_id;
	hr.small_blind = small_blind;

	// --- dealer seat (from the vision-derived dealer_seat field) ---
	int dealer_seat = -1;
	for (const TexasHoldemLogicState* f : hf.frames)
		if (f->dealer_seat_known) { dealer_seat = f->dealer_seat; break; }
	if (dealer_seat < 0) dealer_seat = 0;
	hr.dealer_seat = dealer_seat;

	// blind seats: first active clockwise after dealer (all seats active at hand
	// start here), matching LocalHand::assignButtons().
	int sb_seat = (dealer_seat + 1) % num_seats;
	int bb_seat = (dealer_seat + 2) % num_seats;

	// --- starting stacks (first known stack per seat in this hand's frames) ---
	Vector<int> start_stack(num_seats, -1);
	for (const TexasHoldemLogicState* f : hf.frames)
		for (int seat = 0; seat < num_seats; seat++) {
			if (start_stack[seat] >= 0) continue;
			const TexasHoldemLogicPlayerState* p = SeatOf(*f, seat);
			if (p && p->stack_known) start_stack[seat] = p->stack;
		}
	// The blind seats' first observed stack is already post-blind on screen; add
	// the blind back so the engine (which posts blinds itself) lands on the same
	// on-screen post-blind value.
	Vector<int> seed_cash(num_seats, 1000);
	for (int seat = 0; seat < num_seats; seat++) {
		int s = start_stack[seat] >= 0 ? start_stack[seat] : 1000;
		if (seat == sb_seat) s += small_blind;
		else if (seat == bb_seat) s += small_blind * 2;
		seed_cash[seat] = max(s, 1);
	}

	// --- forced cards: final board + known hole cards (only the shown seats) ---
	Vector<int> board_final(5, -1);
	for (const TexasHoldemLogicState* f : hf.frames)
		if (f->board_cards_known)
			for (int i = 0; i < f->board_cards.GetCount() && i < 5; i++)
				if (f->board_cards[i] >= 0) board_final[i] = f->board_cards[i];
	Vector<Vector<int>> hole(num_seats);
	for (const TexasHoldemLogicState* f : hf.frames)
		for (int seat = 0; seat < num_seats; seat++) {
			if (!hole[seat].IsEmpty()) continue;
			const TexasHoldemLogicPlayerState* p = SeatOf(*f, seat);
			if (p && p->hole_cards_known && !p->hole_cards.IsEmpty()) hole[seat] = clone(p->hole_cards);
		}

	// --- derive real actions ---
	Vector<DerivedAction> events = DeriveActions(hf, num_seats);
	{
		String da;
		for (auto& e : events)
			da << Format("F%d s%d %s%s | ", e.frame_id, e.seat,
			             e.kind == 0 ? "fold" : "to", e.kind == 0 ? "" : ~Format("%d(%s)", e.target_total, e.gt_action));
		hr.derived_actions = da;
	}

	// --- build a real headless Game with 6 explicit-cash HUMAN seats ---
	// All HUMAN so LocalBero::nextPlayer() stalls on each seat and waits for an
	// injected action (rather than auto-deciding as a bot), letting us feed the
	// real recorded action for that seat.
	auto gui = std::make_shared<HeadlessGui>();
	auto engineLog = std::make_shared<EngineLog>(&config);
	PlayerDataList pd;
	for (int i = 0; i < num_seats; i++) {
		auto d = std::make_shared<PlayerData>(i, i, PLAYER_TYPE_HUMAN,
		                                      i == 0 ? PLAYER_RIGHTS_ADMIN : PLAYER_RIGHTS_NORMAL, i == 0);
		d->SetName(Format("Seat%d", i));
		d->SetStartCash(seed_cash[i]);
		pd.push_back(d);
	}
	GameData gd;
	gd.maxNumberOfPlayers = num_seats;
	gd.startMoney = 1000;
	gd.firstSmallBlind = small_blind;
	gd.raiseSmallBlindEveryHandsValue = 100000; // never raise blinds during our single hand
	StartData sd;
	sd.numberOfPlayers = num_seats;
	sd.startDealerPlayerId = dealer_seat;

	auto factory = std::make_shared<LocalEngineFactory>();
	auto game = std::make_shared<Game>(gui.get(), factory, pd, gd, sd, 1, engineLog.get(), &config);
	game->SetBaseSeed(12345); // deterministic RNG for the (unused) non-shown hole cards

	game->initHand();
	game->startHand();

	// force the real known cards immediately after hand construction
	if (auto hand = game->getCurrentHand()) {
		if (auto board = hand->getBoard()) {
			int bc[5];
			for (int i = 0; i < 5; i++) bc[i] = board_final[i];
			board->setMyCards(bc);
		}
	}
	for (int seat = 0; seat < num_seats; seat++) {
		if (hole[seat].IsEmpty()) continue;
		auto p = game->getPlayerByNumber(seat);
		if (p) p->setMyCards(hole[seat].begin(), hole[seat].GetCount());
	}

	Cout() << Format("\n=== HAND %d (gt hand_id=%d) dealer=seat%d SB=%d BB=%d ===\n",
	                 hand_index, hf.hand_id, dealer_seat, small_blind, small_blind * 2);
	Cout() << "  seed_cash: ";
	for (int i = 0; i < num_seats; i++) Cout() << Format("s%d=%d ", i, seed_cash[i]);
	Cout() << "\n  board_forced=" << FormatCards(board_final) << "\n";
	for (int i = 0; i < num_seats; i++)
		if (!hole[i].IsEmpty()) Cout() << Format("  hole s%d=%s\n", i, ~FormatCards(hole[i]));
	Cout() << "  derived_actions: " << hr.derived_actions << "\n";

	// --- drive the engine turn-by-turn ---
	int step = 0, guard = 0, no_turn = 0;
	while (guard++ < 600) {
		auto hand = game->getCurrentHand();
		if (!hand) break;
		int round = (int)hand->getCurrentRound();
		if (round == GAME_STATE_POST_RIVER) break;
		auto bero = hand->getCurrentBeRo();
		if (!bero) break;

		// whose turn is it? use the authoritative turn flag (LocalBero sets exactly
		// one active player's turn flag; getCurrentPlayersTurnId can read one seat
		// ahead in the multi-human advance path, so the flag is the reliable signal)
		int turn_seat = -1;
		for (int s = 0; s < num_seats; s++) {
			auto p = game->getPlayerByNumber(s);
			if (p && p->getMyTurn()) { turn_seat = s; break; }
		}
		if (turn_seat < 0) {
			if (++no_turn > 8) break;
			bero->nextPlayer();
			continue;
		}
		no_turn = 0;

		auto p = game->getPlayerByNumber(turn_seat);
		// Consume strictly in ground-truth frame order: the next real action to
		// occur globally is the earliest unconsumed event. If it belongs to the
		// seat the engine is asking, apply it; otherwise this seat is acting with
		// no recorded chip change, i.e. it CHECKED (a real call would appear as a
		// bet increase == an event). This is what lets a "check then call" seat
		// check on its first turn instead of being served its later call early.
		int head = -1;
		for (int i = 0; i < events.GetCount(); i++)
			if (!events[i].consumed) { head = i; break; }

		String applied, apply_kind;
		int src_frame = -1, src_street = round;
		if (head >= 0 && events[head].seat == turn_seat) {
			DerivedAction& e = events[head];
			e.consumed = true;
			src_frame = e.frame_id;
			src_street = e.street;
			if (e.kind == 0) {
				TexasHoldemApplyExternalAction(game, bero, p, TEXAS_EXT_ACTION_FOLD);
				applied = "fold"; apply_kind = "fold";
			} else {
				TexasHoldemApplyExternalAction(game, bero, p, TEXAS_EXT_ACTION_RAISE_TO, e.target_total);
				applied = Format("to%d(%s->eng_set=%d)", e.target_total, ~e.gt_action, p->getMySet());
				apply_kind = e.gt_action;
			}
		} else {
			// no recorded chip change for this seat on turn => it checked (or flat-
			// called an unraised pot); mirror with the engine's own check/call math.
			TexasHoldemApplyExternalAction(game, bero, p, TEXAS_EXT_ACTION_CHECK_CALL);
			applied = Format("check(eng_set=%d)", p->getMySet());
			apply_kind = "check";
		}

		int true_pot = 0;
		for (int s = 0; s < num_seats; s++) {
			auto pp = game->getPlayerByNumber(s);
			if (pp) true_pot += pp->getMyRoundStartCash() - pp->getMyCash();
		}
		Cout() << Format("  step %2d: seat%d %-30s eng_getPot=%d eng_committed=%d round=%d\n",
		                 step, turn_seat, ~applied,
		                 hand->getBoard() ? hand->getBoard()->getPot() : -1, true_pot, round);

		if (src_frame >= 0)
			CompareEngineToFrame(game, hf, step, src_frame, src_street, turn_seat,
			                     apply_kind, num_seats, hr);

		bero->nextPlayer();
		step++;
	}

	// unconsumed events?
	int leftover = 0;
	for (auto& e : events) if (!e.consumed) leftover++;
	if (leftover)
		Cout() << Format("  WARNING: %d derived actions never matched an engine turn\n", leftover);

	// --- engine showdown result ---
	{
		String res;
		auto hand = game->getCurrentHand();
		int committed = 0;
		for (int s = 0; s < num_seats; s++) {
			auto p = game->getPlayerByNumber(s);
			if (p) committed += p->getMyRoundStartCash() - p->getMyCash();
		}
		int buggy = hand && hand->getBoard() ? hand->getBoard()->getPot() : -1;
		res << Format("final_round=%d true_pot(committed)=%d getPot_buggy=%d | ",
		              hand ? (int)hand->getCurrentRound() : -1, committed, buggy);
		for (int s = 0; s < num_seats; s++) {
			auto p = game->getPlayerByNumber(s);
			if (!p) continue;
			int won = p->getLastMoneyWon();
			res << Format("s%d cash=%d%s ", s, p->getMyCash(), won > 0 ? ~Format("(won %d)", won) : "");
		}
		hr.engine_result = res;
		Cout() << "  ENGINE RESULT: " << res << "\n";
	}

	Cout() << Format("  compared=%d matched=%d mismatched=%d\n", hr.compared, hr.matched, hr.mismatched);
	report.hands.Add(pick(hr));
}

END_UPP_NAMESPACE

GUI_APP_MAIN {
#ifdef PLATFORM_WIN32
	AttachConsole(ATTACH_PARENT_PROCESS);
#endif
	SetVppLogName(AppendFileName(GetCurrentDirectory(), "VideoGameEngineReplayValidator.log"));

	const Vector<String>& args = CommandLine();
	String gt_path, out_report;
	int small_blind = 5;
	for (int i = 0; i < args.GetCount(); i++) {
		if (args[i] == "--ground-truth" && i + 1 < args.GetCount()) gt_path = args[++i];
		else if (args[i] == "--out-report" && i + 1 < args.GetCount()) out_report = args[++i];
		else if (args[i] == "--small-blind" && i + 1 < args.GetCount()) small_blind = StrInt(args[++i]);
	}
	if (gt_path.IsEmpty()) {
		Cout() << "VideoGameEngineReplayValidator (task 0270)\n"
		       << "Usage: --ground-truth <file.jsonl> [--out-report <file.json>] [--small-blind N]\n";
		SetExitCode(1);
		return;
	}
	if (!FileExists(gt_path)) {
		Cerr() << "ground-truth file not found: " << gt_path << "\n";
		SetExitCode(1);
		return;
	}

	// Load frames.
	Vector<TexasHoldemLogicState> frames;
	String content = LoadFile(gt_path);
	Vector<String> lines = Split(content, '\n', false);
	for (const String& raw : lines) {
		String line = TrimBoth(raw);
		if (line.IsEmpty()) continue;
		TexasHoldemLogicState s;
		if (!LoadFromJson(s, line)) {
			Cerr() << "failed to parse: " << line << "\n";
			SetExitCode(1);
			return;
		}
		frames.Add(pick(s));
	}
	Cout() << Format("Loaded %d ground-truth frames from %s\n", frames.GetCount(), ~gt_path);

	// number of seats (max seat index across frames + 1)
	int num_seats = 0;
	for (const auto& f : frames)
		for (const auto& p : f.players) num_seats = max(num_seats, p.seat + 1);
	if (num_seats <= 0) num_seats = 6;

	// group frames into hands (leading unknown-hand frames belong to the first hand)
	VectorMap<int, HandFrames> hands;
	int cur_hand = -1;
	Vector<const TexasHoldemLogicState*> lead;
	for (const auto& f : frames) {
		if (f.hand_id_known) {
			if (cur_hand < 0 && !lead.IsEmpty()) {
				HandFrames& hf = hands.GetAdd(f.hand_id);
				hf.hand_id = f.hand_id;
				for (auto* lp : lead) hf.frames.Add(lp);
				lead.Clear();
			}
			cur_hand = f.hand_id;
			HandFrames& hf = hands.GetAdd(cur_hand);
			hf.hand_id = cur_hand;
			hf.frames.Add(&f);
		} else if (cur_hand < 0) {
			lead.Add(&f);
		} else {
			hands.GetAdd(cur_hand).frames.Add(&f);
		}
	}

	class ConfigFile config(nullptr, false);
	ReplayReport report;
	report.ground_truth_path = gt_path;

	Vector<int> hand_ids;
	for (int i = 0; i < hands.GetCount(); i++) hand_ids.Add(hands.GetKey(i));
	Sort(hand_ids);

	int hand_index = 0;
	for (int hid : hand_ids) {
		const HandFrames& hf = hands.Get(hid);
		Vector<DerivedAction> ev = DeriveActions(hf, num_seats);
		if (ev.IsEmpty()) {
			Cout() << Format("\n(skipping gt hand_id=%d: no voluntary actions parsed)\n", hid);
			continue;
		}
		ReplayHand(hf, hand_index++, num_seats, small_blind, config, report);
	}

	if (!out_report.IsEmpty()) {
		RealizeDirectory(GetFileFolder(out_report));
		SaveFile(out_report, StoreAsJson(report, true));
		Cout() << "\nwrote report: " << out_report << "\n";
	}

	// summary
	Cout() << "\n===== SUMMARY =====\n";
	for (const auto& hr : report.hands)
		Cout() << Format("hand %d (gt %d): compared=%d matched=%d mismatched=%d\n",
		                 hr.hand_index, hr.gt_hand_id, hr.compared, hr.matched, hr.mismatched);
}
