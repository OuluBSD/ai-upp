#ifndef _ScreenGame_RecognitionData_h_
#define _ScreenGame_RecognitionData_h_

#include <GameCommon/Rules/EngineDefs.h>

namespace Upp {

enum GameRound {
	ROUND_NONE,
	ROUND_PREFLOP,
	RULE_PREFLOP = ROUND_PREFLOP, // backward-compat alias
	ROUND_FLOP,
	ROUND_TURN,
	ROUND_RIVER,
	ROUND_SHOWDOWN,
};

struct GameState : public Moveable<GameState> {
	struct Player : public Moveable<Player> {
		String name;
		int    stack = 0;
		int    bet = 0;
		Vector<int> hand;
		String last_action;
		bool   is_active = false;
		bool   is_dealer = false;
		bool   is_folded = false;
		bool   is_allin  = false;
		
		Player() {}
		Player(const Player& p) { *this = p; }
		void operator=(const Player& p) {
			name = p.name;
			stack = p.stack;
			bet = p.bet;
			hand <<= p.hand;
			last_action = p.last_action;
			is_active = p.is_active;
			is_dealer = p.is_dealer;
			is_folded = p.is_folded;
			is_allin  = p.is_allin;
		}
		Player(const Player& p, int) { *this = p; }
		
		void Jsonize(JsonIO& jio) {
			jio("name", name)("stack", stack)("bet", bet)("hand", hand)("action", last_action)("active", is_active)("dealer", is_dealer)("folded", is_folded)("allin", is_allin);
		}
	};

	int pot = 0;
	Vector<int> stacks;
	Vector<int> community_cards;
	Vector<Player> players;
	Vector<byte>   history;
	GameRound round = ROUND_NONE;
	double hero_equity = 0.0;
	String advice;
	Vector<double> advice_probs;
	bool my_turn = false;
	String active_platform;
	Rect pokerth_rect;
	bool found = false;
	
	GameType hand_type = GAME_TYPE_NLTH;
	int      table_id = 0;
	int      tournament_mode = 0;
	
	// Performance instrumentation
	int64 ocr_us = 0;
	int64 orb_us = 0;
	int64 logic_us = 0;
	
	GameState() {}
	GameState(const GameState& s) { *this = s; }
	
	void operator=(const GameState& s) {
		pot = s.pot;
		stacks <<= s.stacks;
		community_cards <<= s.community_cards;
		players <<= s.players;
		history <<= s.history;
		round = s.round;
		hero_equity = s.hero_equity;
		advice = s.advice;
		advice_probs <<= s.advice_probs;
		my_turn = s.my_turn;
		active_platform = s.active_platform;
		pokerth_rect = s.pokerth_rect;
		found = s.found;
		hand_type = s.hand_type;
		table_id = s.table_id;
		tournament_mode = s.tournament_mode;
		ocr_us = s.ocr_us;
		orb_us = s.orb_us;
		logic_us = s.logic_us;
	}
	
	void Jsonize(JsonIO& jio) {
		jio
			("pot", pot)
			("stacks", stacks)
			("community_cards", community_cards)
			("players", players)
			("history", history)
			("round", (int&)round)
			("hero_equity", hero_equity)
			("advice", advice)
			("advice_probs", advice_probs)
			("my_turn", my_turn)
			("active_platform", active_platform)
			("found", found)
			("hand_type", (int&)hand_type)
			("table_id", table_id)
			("tournament_mode", tournament_mode)
			("ocr_us", ocr_us)
			("orb_us", orb_us)
			("logic_us", logic_us)
		;
	}
	
	uint64 GetHash() const {
		CombineHash h;
		h << (int)round << (int)hand_type << my_turn;
		for(int c : community_cards) h << c;
		for(const auto& p : players) {
			h << p.is_active << p.is_dealer << p.is_folded << p.is_allin << p.last_action;
			for(int c : p.hand) h << c;
		}
		// Note: we omit pot/stack from structural hash to allow for minor OCR jitter
		return h;
	}
};

struct HandAction : public Moveable<HandAction> {
	int    player_id;
	String action;
	int    amount = 0;
	GameRound round;
	
	void Jsonize(JsonIO& jio) {
		jio("player", player_id)("action", action)("amount", amount)("round", (int&)round);
	}
};

struct HandHistory : public Moveable<HandHistory> {
	Vector<HandAction> actions;
	Vector<int>        community_cards;
	Vector<GameState::Player> players;
	int                final_pot = 0;
	
	void Jsonize(JsonIO& jio) {
		jio("actions", actions)("community", community_cards)("players", players)("pot", final_pot);
	}
};

}

#endif
