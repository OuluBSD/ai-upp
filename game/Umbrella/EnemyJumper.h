#ifndef _Umbrella_EnemyJumper_h_
#define _Umbrella_EnemyJumper_h_

#include "Enemy.h"

using namespace Upp;

class EnemyJumper : public Enemy {
private:
	static constexpr float WALK_SPEED = 80.0f;
	static constexpr float JUMP_VELOCITY = 300.0f;
	static constexpr float MIN_JUMP_INTERVAL = 1.5f;
	static constexpr float MAX_JUMP_INTERVAL = 2.5f;

	float jumpTimer;
	float nextJumpTime;

public:
	EnemyJumper(float x, float y);

	virtual void Update(float delta, const Player& player, Player::CollisionHandler& collision) override;
	virtual void Render(Draw& w, Player::CoordinateConverter& coords) override;

private:
	void ResetJumpTimer();
	bool IsOnGround(Player::CollisionHandler& collision);
};

#endif
