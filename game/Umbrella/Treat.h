#ifndef _Umbrella_Treat_h_
#define _Umbrella_Treat_h_

#include <Core/Core.h>
#include "Player.h"

using namespace Upp;

enum TreatType {
	TREAT_PEAR,       // Patroller drops
	TREAT_BANANA,     // Jumper drops
	TREAT_BLUEBERRY,  // Shooter drops
	TREAT_SODA,       // Bonus
	TREAT_CAKE        // Special bonus
};

class Treat {
private:
	Rectf bounds;
	Pointf velocity;
	TreatType type;
	int scoreValue;
	bool active;
	bool onGround;   // Resting on ground (don't apply gravity)
	float lifetime;  // Time alive (for animation/effects)

	static constexpr float TREAT_SIZE = 10.0f;
	static constexpr float GRAVITY = -490.0f;
	static constexpr float INITIAL_VY = 200.0f;  // Thrown upward
	static constexpr float INITIAL_VX_RANGE = 80.0f;  // Random horizontal spread
	static constexpr float MAX_LIFETIME = 10.0f;  // Despawn after 10 seconds

public:
	Treat(float x, float y, TreatType treatType);

	void Update(float delta, Player::CollisionHandler& collision);
	void Render(Draw& w, Player::CoordinateConverter& coords);

	bool IsActive() const { return active; }
	Rectf GetBounds() const { return bounds; }
	int GetScoreValue() const { return scoreValue; }
	TreatType GetType() const { return type; }
	void Collect() { active = false; }

private:
	Color GetTreatColor() const;
};

#endif
