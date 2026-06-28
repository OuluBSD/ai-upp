#include "TexasHoldem.h"

NAMESPACE_UPP

BotAction RandomBot::choose_action(const Vector<ActionInfo>& valid_actions, const HandState& state, int player_idx) {
	if (valid_actions.IsEmpty()) {
		return BotAction("fold", 0);
	}

	int r_idx = Random(valid_actions.GetCount());
	const ActionInfo& a = valid_actions[r_idx];
	int amount = 0;

	if (a.action == "bet" || a.action == "raise") {
		if (a.max_amount > a.min_amount) {
			amount = a.min_amount + Random(a.max_amount - a.min_amount + 1);
		} else {
			amount = a.min_amount;
		}
	} else if (a.action == "call") {
		amount = a.amount;
	}

	return BotAction(a.action, amount);
}

BotAction CallBot::choose_action(const Vector<ActionInfo>& valid_actions, const HandState& state, int player_idx) {
	for (const auto& a : valid_actions) {
		if (a.action == "check") {
			return BotAction("check", 0);
		}
		if (a.action == "call") {
			return BotAction("call", a.amount);
		}
	}
	return BotAction("fold", 0);
}

END_UPP_NAMESPACE
