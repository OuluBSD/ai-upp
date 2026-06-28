#ifndef _ScreenGame_RecognitionData_h_
#define _ScreenGame_RecognitionData_h_

#include <Draw/Draw.h>
#include <GameRules/EngineDefs.h>

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

enum RuleType {
	RULE_WINDOW,           // 0
	RULE_AREA_TOPLEFT,     // 1
	RULE_BUTTON,           // 2
	RULE_IMAGE_BOOLEAN,    // 3
	RULE_SLIDER,           // 4
	RULE_IMAGE_ANCHOR,     // 5
	RULE_BOARD_CARD_AREA,  // 6
	RULE_BOARD_CARD,       // 7
	RULE_AREA,             // 8 (PlayerArea)
	RULE_HAND_CARD_AREA,   // 9
	RULE_HAND_CARD,        // 10
	RULE_DEALER_CHIP,      // 11
	RULE_SMALL_BLIND,      // 12
	RULE_BIG_BLIND,        // 13
	RULE_PLAYER_AVATAR,    // 14
	RULE_PLAYER_NAME,      // 15
	RULE_PLAYER_STACK,     // 16 (PlayerAccount)
	RULE_POT_TOTAL,        // 17
	RULE_TURN_BETS,        // 18
	RULE_TURN_NAME,        // 19
	RULE_GAME_ID,          // 20
	RULE_HAND_ID,          // 21
	RULE_LOG,              // 22
	RULE_TEXT_AREA,        // 23 (Text)
	RULE_RAISE_TEXT,       // 24
	RULE_CALL_TEXT,        // 25
	RULE_PLAYER_BET,       // 26
	RULE_PLAYER_CARD,      // 27
	RULE_PLAYER_ACTION,    // 28
	RULE_ACTIVE_PLAYER,    // 29
};

inline const Vector<String>& GetRuleClassNames() {
	static Vector<String> names;
	if (names.IsEmpty()) {
		names
			<< "Window"          // 0
			<< "WindowCorner"    // 1
			<< "Button"          // 2
			<< "CheckBox"        // 3
			<< "Slider"          // 4
			<< "ImageAnchor"     // 5
			<< "BoardCardArea"   // 6
			<< "BoardCard"       // 7
			<< "PlayerArea"      // 8
			<< "HandCardArea"    // 9
			<< "HandCard"        // 10
			<< "DealerChip"      // 11
			<< "SmallBlindChip"  // 12
			<< "BigBlindChip"    // 13
			<< "PlayerAvatar"    // 14
			<< "PlayerName"      // 15
			<< "PlayerAccount"   // 16
			<< "PotTotal"        // 17
			<< "TurnBets"        // 18
			<< "TurnName"        // 19
			<< "GameId"          // 20
			<< "HandId"          // 21
			<< "Log"             // 22
			<< "Text"            // 23
			<< "RaiseText"       // 24
			<< "CallText"        // 25
			<< "PlayerBet"       // 26
			<< "PlayerCard"      // 27
			<< "PlayerAction"    // 28
			<< "ActivePlayer";   // 29
	}
	return names;
}

inline String GetRuleClassName(int type) {
	const Vector<String>& names = GetRuleClassNames();
	return (type >= 0 && type < names.GetCount()) ? names[type] : ("Type " + AsString(type));
}

inline int ParseRuleClassValue(const Value& v) {
	auto map_legacy = [](int legacy_type) -> int {
		switch (legacy_type) {
		case 0:        return 0;  // Window (RULE_WINDOW)
		case 7:        return 8;  // PlayerArea -> Area
		case 2:        return 2;  // Button
		case 1:        return 1;  // WindowCorner
		case 22:       return 23; // Text -> TextArea
		case 3:        return 3;  // CheckBox
		default:       return legacy_type;
		}
	};

	if (IsNull(v))
		return 0;
	if (v.Is<int>())
		return map_legacy((int)v);
	String s = TrimBoth(AsString(v));
	if (s.IsEmpty())
		return 0;
	if (IsDigit((byte)s[0]) || s[0] == '-')
		return map_legacy(StrInt(s));
	const Vector<String>& names = GetRuleClassNames();
	if (ToLower(s) == "humanavatar")
		return 14; // PlayerAvatar
	for (int i = 0; i < names.GetCount(); i++) {
		if (names[i] == s || ToLower(names[i]) == ToLower(s))
			return i;
	}
	if (s.StartsWith("Type "))
		return StrInt(s.Mid(5));
	return 0;
}

struct GameRule : public Moveable<GameRule> {
	String name;
	int    type = RULE_AREA;
	Rect   rect;
	String parent_name;
	Vector<String> samples;
	Vector<String> images;
	VectorMap<String, String> props;
	
	void Jsonize(JsonIO& jio) {
		String type_name;
		if (jio.IsStoring()) type_name = GetRuleClassName(type);
		jio("name", name)
		   ("type", type_name)
		   ("rect", rect)
		   ("parent", parent_name)
		   ("props", props)
		   ("samples", samples)
		   ("images", images);
		if (jio.IsLoading())
			type = ParseRuleClassValue(type_name);
	}
};

class RuleManager {
	struct Platform : public Moveable<Platform> {
		String name;
		Size   base_size;
		Array<GameRule> rules;
		Image  anchor;
		
		void Jsonize(JsonIO& jio) {
			jio("name", name)("base_size", base_size)("rules", rules)("anchor", anchor);
		}
	};
	VectorMap<String, Platform> platforms;
	String active_platform;
	String path;

public:
	void Load(const String& p);
	void Save();
	void Clear();
	void SetRules(Array<GameRule>&& src);
	Array<GameRule>& GetRules();
	const Array<GameRule>& GetRules() const;
	void SetPath(const String& p) { path = p; }
	String GetPath() const { return path; }
	
	GameRule& Add() { return GetRules().Add(); }
	void      Remove(int i) { GetRules().Remove(i); }
	int       GetCount() const { return GetRules().GetCount(); }
	GameRule& GetRule(int i) { return GetRules()[i]; }
	const GameRule& GetRule(int i) const { return GetRules()[i]; }
	
	int       FindRule(const String& name) const;
	Rect      GetAbsRect(int index, Size frame_size) const;

	void      SetActivePlatform(const String& name) { active_platform = name; }
	String    GetActivePlatform() const { return active_platform; }
	Size      GetBaseSize(const String& name) const;
	Vector<String> GetPlatformNames() const { 
		Vector<String> res;
		for (int i = 0; i < platforms.GetCount(); i++) res.Add(platforms.GetKey(i));
		return res;
	}
	void      AddPlatform(const String& name, Size sz);

	void Jsonize(JsonIO& jio) {
		jio("platforms", platforms)("active_platform", active_platform);
		if (jio.IsLoading()) {
			if (platforms.IsEmpty()) {
				Array<GameRule> old_rules;
				jio("rules", old_rules);
				if (!old_rules.IsEmpty()) {
					Platform& p = platforms.Add("default");
					p.name = "default";
					p.base_size = Size(800, 480);
					p.rules = pick(old_rules);
					active_platform = "default";
				}
			}
			if (active_platform.IsEmpty() && !platforms.IsEmpty())
				active_platform = platforms.GetKey(0);
		}
	}
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

class SceneMachine {
	GameRound current_round = ROUND_NONE;
	
public:
	GameRound Update(const GameState& state);
	GameRound GetCurrentRound() const { return current_round; }
	String    GetRoundName() const;
};

class HandHistoryBuilder {
	HandHistory    current_hand;
	GameRound      last_round = ROUND_NONE;
	Vector<String> last_player_actions;
	
public:
	void Update(const GameState& state);
	const HandHistory& GetCurrentHand() const { return current_hand; }
	void Reset() { current_hand = HandHistory(); last_round = ROUND_NONE; }
	void SaveHistory(const String& path) { SaveFile(path, StoreAsJson(current_hand)); }
};

class StateStability {
	uint64    last_hash = 0;
	int       stable_frames = 0;
	GameState stable_state;
	int       required_frames = 3;
	
public:
	void      Update(const GameState& state);
	bool      IsStable() const { return stable_frames >= required_frames; }
	const GameState& GetState() const { return stable_state; }
	void      Reset() { last_hash = 0; stable_frames = 0; }
	void      SetRequiredFrames(int n) { required_frames = n; }
};

class HysteresisFilter {
	struct Entry : public Moveable<Entry> {
		int value = 0;
		int count = 0;
	};
	VectorMap<String, Entry> values;
	int threshold = 2;
	
public:
	int  Filter(const String& key, int new_value);
	void SetThreshold(int n) { threshold = n; }
	void Clear() { values.Clear(); }
};

Image CropRectSafe(const Image& src, const Rect& r);

}

#endif
