#include <Core/Core.h>
#include <TexasHoldem/TexasHoldem.h>

using namespace Upp;

CONSOLE_APP_MAIN {
	StdLogSetup(LOG_COUT | LOG_FILE);
	LOG("Starting C++ TexasHoldem chip conservation test...");

	int num_players = 6;
	int initial_stack = 1000;
	int initial_total = num_players * initial_stack;

	NLTHGame g(num_players, initial_stack, 5, 10);
	RandomBot random_bot;

	for (int i = 0; i < 100; i++) {
		g.new_hand();
		while (!g.is_hand_over()) {
			int idx = g.active_player_idx;
			Vector<ActionInfo> valid = g.valid_actions(idx);
			HandState st = g.get_state();
			
			BotAction action = random_bot.choose_action(valid, st, idx);
			g.apply_action(idx, action.action, action.amount);
		}

		int current_total = 0;
		for (int p_idx = 0; p_idx < g.players.GetCount(); p_idx++) {
			current_total += g.players[p_idx].stack;
		}

		if (current_total != initial_total) {
			LOG("CHIP CONSERVATION FAILURE at hand " << i);
			LOG("Expected: " << initial_total << " Got: " << current_total);
			ASSERT(current_total == initial_total);
		}
	}

	LOG("[PASS] C++ TexasHoldem chip conservation test");
}
