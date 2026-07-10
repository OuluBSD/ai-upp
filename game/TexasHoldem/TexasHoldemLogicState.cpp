#include "TexasHoldemLogicState.h"

NAMESPACE_UPP

void TexasHoldemLogicPlayerState::Jsonize(JsonIO& jio)
{
	jio
		("seat", seat)
		("uid_known", uid_known)("uid", uid)
		("name_known", name_known)("name", name)
		("hero_known", hero_known)("hero", hero)
		("active_known", active_known)("active", active)
		("stack_known", stack_known)("stack", stack)
		("bet_known", bet_known)("bet", bet)
		("action_known", action_known)("action", action)
		("button_known", button_known)("button", button)
		("hole_cards_known", hole_cards_known)("hole_cards", hole_cards)
	;
}

void TexasHoldemLogicState::Jsonize(JsonIO& jio)
{
	jio
		("schema", schema)
		("session_id_known", session_id_known)("session_id", session_id)
		("frame_id", frame_id)
		("render_step_known", render_step_known)("render_step", render_step)
		("timestamp_ms_known", timestamp_ms_known)("timestamp_ms", timestamp_ms)
		("provider_known", provider_known)("provider", provider)
		("table_size_known", table_size_known)
			("table_width", table_width)("table_height", table_height)
		("seed_known", seed_known)("seed", seed)
		("game_id_known", game_id_known)("game_id", game_id)
		("hand_id_known", hand_id_known)("hand_id", hand_id)
		("street_known", street_known)("street", street)
		("turn_uid_known", turn_uid_known)("turn_uid", turn_uid)
		("pot_known", pot_known)("pot", pot)
		("board_cards_known", board_cards_known)("board_cards", board_cards)
		("players_known", players_known)("players", players)
		("dealer_seat_known", dealer_seat_known)("dealer_seat", dealer_seat)
	;
}

END_UPP_NAMESPACE
