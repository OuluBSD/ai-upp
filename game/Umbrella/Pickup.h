#ifndef _Umbrella_Pickup_h_
#define _Umbrella_Pickup_h_

#include <Core/Core.h>
#include "Player.h"

using namespace Upp;

enum PickupType { PU_HEART, PU_GEM, PU_LIGHTNING, PU_SPEED };

class Pickup {
	Rectf  bounds;
	PickupType type;
	bool   active;
	float  bobTimer;   // drives sinusoidal vertical offset

	static constexpr float SIZE      = 12.0f;
	static constexpr float BOB_SPEED = 2.5f;   // radians/sec
	static constexpr float BOB_AMP   = 3.0f;   // pixels

public:
	Pickup(float x, float y, PickupType t);

	void Update(float delta);
	void Render(Draw& w, Player::CoordinateConverter& coords);

	bool        IsActive()   const { return active; }
	void        Collect()          { active = false; }
	Rectf       GetBounds()  const { return bounds; }
	PickupType  GetType()    const { return type; }
};

#endif
