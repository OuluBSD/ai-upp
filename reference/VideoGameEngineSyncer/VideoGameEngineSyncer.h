#ifndef _VideoGameEngineSyncer_VideoGameEngineSyncer_h_
#define _VideoGameEngineSyncer_VideoGameEngineSyncer_h_

#include <Core/Core.h>
#include <TexasHoldem/TexasHoldemLogicState.h>

NAMESPACE_UPP

struct TrackedPlayer : Moveable<TrackedPlayer> {
	int seat = -1;
	bool active_known = false;
	bool active = false;
	bool stack_known = false;
	int stack = 0;
	bool bet_known = false;
	int bet = 0;
	bool hole_cards_known = false;
	Vector<int> hole_cards;
};

struct DivergenceEvent : Moveable<DivergenceEvent> {
	int frame_id = -1;
	String field;
	Value expected;
	Value actual;
	String description;

	void Jsonize(JsonIO& json) {
		json
			("frame_id", frame_id)
			("field", field)
			("expected", expected)
			("actual", actual)
			("description", description)
		;
	}
};

struct VsmProductionRecordOutWrapper : Moveable<VsmProductionRecordOutWrapper> {
	int frame_id = -1;
	TexasHoldemLogicState logic_state;

	void Jsonize(JsonIO& json) {
		json
			("frame_id", frame_id)
			("logic_state", logic_state)
		;
	}
};

END_UPP_NAMESPACE

#endif
