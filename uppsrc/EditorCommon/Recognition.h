#ifndef _ScreenGame_Recognition_h_
#define _ScreenGame_Recognition_h_

#include <Draw/Draw.h>
#include <ConvNet/ConvNet.h>
#include <ComputerVision/ComputerVision.h>
#include <EditorCommon/RecognitionData.h>

namespace Upp {

class GpuPreprocessEngine;

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
		// Backward compatibility: pre-text class ids from old enum RuleType.
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
	String parent_name; // For relative positioning
	Vector<String> samples;
	Vector<String> images;
	
	// Properties
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
				// Backward compatibility
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

class OCRModule {
	typedef void* TessBaseAPI;
#ifdef PLATFORM_WIN32
	HMODULE hTess = NULL;
#else
	void*   hTess = NULL;
#endif
	
	TessBaseAPI (*TessBaseAPICreate)();
	int (*TessBaseAPIInit3)(TessBaseAPI, const char*, const char*);
	void (*TessBaseAPISetImage)(TessBaseAPI, const unsigned char*, int, int, int, int);
	char* (*TessBaseAPIGetUTF8Text)(TessBaseAPI);
	void (*TessBaseAPIDelete)(TessBaseAPI);
	void (*TessDeleteText)(const char*);

	TessBaseAPI api = NULL;
	bool loaded = false;

public:
	OCRModule();
	~OCRModule();
	
	bool IsLoaded() const { return loaded; }
	Image Preprocess(const Image& img);
	String ReadText(const Image& img);
	String ReadText(const ByteMat& gray);
};

class CardClassifier {
	ConvNet::Session suit_ses;
	ConvNet::Session rank_ses;
	ConvNet::Session unified_ses;
	Vector<String>   suit_classes;
	Vector<String>   rank_classes;
	Vector<String>   unified_classes;
	bool             suit_loaded = false;
	bool             rank_loaded = false;
	bool             unified_loaded = false;
	int              suit_w = 0, suit_h = 0, suit_d = 0;
	int              rank_w = 0, rank_h = 0, rank_d = 0;
	int              unified_w = 0, unified_h = 0, unified_d = 0;

public:
	CardClassifier();
	bool Load(const String& model_dir, const String& platform = "");
	int  ClassifySuit(const Image& img);
	int  ClassifyRank(const Image& img);
	int  ClassifyUnified(const Image& img);
	int  ClassifySuit(const ByteMat& gray);
	int  ClassifyRank(const ByteMat& gray);
	int  ClassifyUnified(const ByteMat& gray);
	double GetConfidence(int id) { return unified_ses.GetNetwork().GetOutput().Get(id); }
	bool IsUnified() const { return unified_loaded; }
	bool IsLoaded() const { return unified_loaded || (suit_loaded && rank_loaded); }
};

class SceneMachine {
	GameRound current_round = ROUND_NONE;
	int       stable_frames = 0;
	GameRound target_round = ROUND_NONE;
	
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

class StateFilter {
	struct Field : public Moveable<Field> {
		String value;
		int    count = 0;
	};
	VectorMap<String, Vector<Field>> history;
	const int MAX_HISTORY = 5;
	
public:
	String Filter(const String& key, const String& value);
	int    Filter(const String& key, int value);
	void   Clear() { history.Clear(); }
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

double ComputeHeroEquity(const Vector<int>& hole_cards, const Vector<int>& board_cards, void* eval_ptr);
String GetStrategyAdvice(const Vector<int>& hole_cards, const Vector<int>& board_cards, int pot, const Vector<byte>& history, void* strategy_ptr, Vector<double>& out_probs);
void   InitStrategy(void*& eval_ptr, void*& strategy_ptr);
void   CleanupStrategy(void* eval_ptr, void* strategy_ptr);

Image CropRectSafe(const Image& src, const Rect& r);

class RecognitionEngine {
public:
	OCRModule      ocr;
	RuleManager    rule_mgr;
	OrbSystem      orb;
	CardClassifier card_clf;
	SceneMachine   scene_machine;
	HandHistoryBuilder hh_builder;
	StateFilter    state_filter;
	HysteresisFilter hysteresis;
	void*          eval_pimpl = nullptr;
	void*          strategy_pimpl = nullptr;
	Image          anchor_img;
	bool           anchor_trained = false;

public:
	RecognitionEngine();
	~RecognitionEngine();
	
	GameState Process(const Image& frame);
	GameState ProcessGpu(GpuPreprocessEngine& gpu);
	Rect FindPokerTH(const Image& frame);
	
	void SetAnchor(const Image& img);
	RuleManager& GetRuleManager() { return rule_mgr; }
};

}

#endif
