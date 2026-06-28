#ifndef _game_TexasHoldem_Bots_h_
#define _game_TexasHoldem_Bots_h_

struct BotAction : public Moveable<BotAction> {
	String action;
	int amount;

	BotAction() : action("fold"), amount(0) {}
	BotAction(const String& action, int amount) : action(action), amount(amount) {}
};

class RandomBot {
public:
	BotAction choose_action(const Vector<ActionInfo>& valid_actions, const HandState& state, int player_idx);
};

class CallBot {
public:
	BotAction choose_action(const Vector<ActionInfo>& valid_actions, const HandState& state, int player_idx);
};

#endif
