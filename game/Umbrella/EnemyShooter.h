#ifndef _Umbrella_EnemyShooter_h_
#define _Umbrella_EnemyShooter_h_

#include "Enemy.h"
#include "Projectile.h"

using namespace Upp;

class EnemyShooter : public Enemy {
private:
	static constexpr float SHOOT_COOLDOWN = 2.0f;
	static constexpr float DETECTION_RANGE = 15.0f * 14.0f;  // 15 tiles * 14 pixels per tile

	float shootTimer;
	Array<Projectile*> projectiles;

public:
	EnemyShooter(float x, float y);
	~EnemyShooter();

	virtual void Update(float delta, const Player& player, Player::CollisionHandler& collision) override;
	virtual void Render(Draw& w, Player::CoordinateConverter& coords) override;

	// Projectile access
	const Array<Projectile*>& GetProjectiles() const { return projectiles; }

private:
	bool CanSeePlayer(const Player& player);
	void ShootAtPlayer(const Player& player);
	void UpdateProjectiles(float delta, Player::CollisionHandler& collision);
};

#endif
