#ifndef _Umbrella_Enemy_h_
#define _Umbrella_Enemy_h_

#include <Core/Core.h>
#include "Player.h"

using namespace Upp;

enum EnemyType {
	ENEMY_PATROLLER,
	ENEMY_JUMPER,
	ENEMY_SHOOTER
};

class Enemy {
protected:
	Rectf bounds;
	Pointf velocity;
	int health;
	bool alive;
	bool active;
	float stateTimer;
	int facing;  // -1 left, 1 right
	EnemyType type;

	// Physics constants
	static constexpr float GRAVITY = -490.0f;
	static constexpr float MAX_FALL_SPEED = -280.0f;
	static constexpr float COLLISION_STEP = 3.5f;

	// Collision helpers
	bool IsTouchingWallOnLeft(Player::CollisionHandler& collision);
	bool IsTouchingWallOnRight(Player::CollisionHandler& collision);
	bool IsOnGround(Player::CollisionHandler& collision);
	bool IsFloorAhead(Player::CollisionHandler& collision);
	void ResolveCollisionX(float deltaX, Player::CollisionHandler& collision);
	void ResolveCollisionY(float deltaY, Player::CollisionHandler& collision);

public:
	Enemy(float x, float y, float width, float height, EnemyType t);
	virtual ~Enemy() {}

	virtual void Update(float delta, const Player& player, Player::CollisionHandler& collision) = 0;
	virtual void Render(Draw& w, Player::CoordinateConverter& coords) = 0;

	// Accessors
	bool IsAlive() const { return alive; }
	bool IsActive() const { return active; }
	Rectf GetBounds() const { return bounds; }
	Pointf GetVelocity() const { return velocity; }
	EnemyType GetType() const { return type; }
	int GetFacing() const { return facing; }

	// Mutators
	void TakeDamage(int amount);
	void Kill();
	void SetActive(bool act) { active = act; }
	float GetGridSize(Player::CollisionHandler& collision) { return collision.GetGridSize(); }
};

#endif
