#include <EditorCommon/EditorCommon.h>

namespace Upp {

Image CropRectSafe(const Image& src, const Rect& r) {
	if (src.IsEmpty() || r.IsEmpty()) return Image();
	Rect rr = r;
	rr.Intersect(src.GetSize());
	if (rr.IsEmpty()) return Image();
	return Crop(src, rr);
}

GameRound SceneMachine::Update(const GameState& state) {
	GameRound detected = ROUND_NONE;
	int cards = state.community_cards.GetCount();
	if (cards == 0) detected = ROUND_PREFLOP;
	else if (cards == 3) detected = ROUND_FLOP;
	else if (cards == 4) detected = ROUND_TURN;
	else if (cards == 5) detected = ROUND_RIVER;
	current_round = detected;
	return current_round;
}

String SceneMachine::GetRoundName() const {
	switch (current_round) {
		case ROUND_PREFLOP: return "Pre-flop";
		case ROUND_FLOP: return "Flop";
		case ROUND_TURN: return "Turn";
		case ROUND_RIVER: return "River";
		default: return "None";
	}
}

void RuleManager::Clear() { platforms.Clear(); active_platform.Clear(); }
void RuleManager::SetRules(Array<GameRule>&& src) { platforms.GetAdd(active_platform).rules = pick(src); }
Array<GameRule>& RuleManager::GetRules() { return platforms.GetAdd(active_platform).rules; }
const Array<GameRule>& RuleManager::GetRules() const {
	int q = platforms.Find(active_platform);
	if (q >= 0) return platforms[q].rules;
	static Array<GameRule> empty; return empty;
}

void RuleManager::Load(const String& p) {
	path = p;
	Clear();
	LoadFromJson(*this, LoadFile(p));
}

void RuleManager::Save() { SaveFile(path, StoreAsJson(*this)); }

int RuleManager::FindRule(const String& name) const {
	const auto& rules = GetRules();
	for (int i = 0; i < rules.GetCount(); i++) if (rules[i].name == name) return i;
	return -1;
}

Size RuleManager::GetBaseSize(const String& name) const {
	int q = platforms.Find(name);
	return q >= 0 ? platforms[q].base_size : Size(0, 0);
}

Rect RuleManager::GetAbsRect(int index, Size frame_size) const {
	const auto& rules = GetRules();
	if (index < 0 || index >= rules.GetCount()) return Rect(0, 0, 0, 0);
	const GameRule& r = rules[index];
	Size base = GetBaseSize(active_platform);
	if (base.cx <= 0 || base.cy <= 0) return r.rect;
	double sx = (double)frame_size.cx / base.cx;
	double sy = (double)frame_size.cy / base.cy;
	return Rect((int)(r.rect.left * sx), (int)(r.rect.top * sy), (int)(r.rect.right * sx), (int)(r.rect.bottom * sy));
}

void RuleManager::AddPlatform(const String& name, Size sz) {
	Platform& p = platforms.GetAdd(name);
	p.name = name; p.base_size = sz;
	active_platform = name;
}

void HandHistoryBuilder::Update(const GameState& state) {
	if (state.round != last_round) {
		last_round = state.round;
		current_hand.community_cards <<= state.community_cards;
	}
	while (last_player_actions.GetCount() < state.players.GetCount())
		last_player_actions.Add("");
	
	for (int i = 0; i < state.players.GetCount(); i++) {
		const auto& p = state.players[i];
		while (current_hand.players.GetCount() <= i) current_hand.players.Add();
		auto& hp = current_hand.players[i];
		hp.hand <<= p.hand;
		if (!p.name.IsEmpty()) hp.name = p.name;
		
		if (!p.last_action.IsEmpty() && p.last_action != last_player_actions[i]) {
			HandAction& a = current_hand.actions.Add();
			a.player_id = i;
			a.action = p.last_action;
			a.amount = p.bet;
			a.round = state.round;
			last_player_actions[i] = p.last_action;
		}
	}
}

void StateStability::Update(const GameState& state) {
	uint64 h = state.GetHash();
	if (h == last_hash) {
		stable_frames++;
		if (stable_frames == required_frames) {
			stable_state = state;
		}
	} else {
		last_hash = h;
		stable_frames = 0;
	}
}

int HysteresisFilter::Filter(const String& key, int new_value) {
	Entry& e = values.GetAdd(key);
	if (new_value == e.value) {
		e.count = 0;
	} else {
		e.count++;
		if (e.count >= threshold) {
			e.value = new_value;
			e.count = 0;
		}
	}
	return e.value;
}

}
