#ifndef _Umbrella_EnemyPatroller_h_
#define _Umbrella_EnemyPatroller_h_

#include "Enemy.h"

using namespace Upp;

class EnemyPatroller : public Enemy {
private:
	static constexpr float WALK_SPEED = 70.0f;

public:
	EnemyPatroller(float x, float y);

	virtual void Update(float delta, const Player& player, Player::CollisionHandler& collision) override;
	virtual void Render(Draw& w, Player::CoordinateConverter& coords) override;
};

#endif
