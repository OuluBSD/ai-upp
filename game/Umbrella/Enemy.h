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
	bool active;      // Active in world (not captured)
	bool captured;    // Captured by player on umbrella
	bool thrown;      // Thrown by player
	bool carriedByThrown;  // Carried by a thrown enemy
	float stateTimer;
	int facing;  // -1 left, 1 right
	EnemyType type;
	float carryWeight;  // Weight for carrying on umbrella
	float originalSize;  // Original width (for size comparison when thrown)
	float rotation;  // Rotation angle in degrees (for visual effects)

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

	// Visual effects
	void UpdateRotation(float delta);  // Update rotation for spinning effect
	Color GetTintColor() const;  // Get tint color based on state

public:
	Enemy(float x, float y, float width, float height, EnemyType t);
	virtual ~Enemy() {}

	virtual void Update(float delta, const Player& player, Player::CollisionHandler& collision) = 0;
	virtual void Render(Draw& w, Player::CoordinateConverter& coords) = 0;

	// Accessors
	bool IsAlive() const { return alive; }
	bool IsActive() const { return active; }
	bool IsCaptured() const { return captured; }
	bool IsThrown() const { return thrown; }
	bool IsCarriedByThrown() const { return carriedByThrown; }
	Rectf GetBounds() const { return bounds; }
	Pointf GetVelocity() const { return velocity; }
	EnemyType GetType() const { return type; }
	int GetFacing() const { return facing; }
	float GetCarryWeight() const { return carryWeight; }
	float GetSize() const { return originalSize; }  // For size comparison (use original, not current bounds)

	// Mutators
	void TakeDamage(int amount);
	void Kill();
	void Defeat();  // Killed by player (triggers rewards)
	void Capture();  // Captured by player umbrella
	void ThrowFrom(float x, float y, float vx, float vy);  // Thrown by player
	void CaptureByThrown(const Pointf& throwerVelocity);  // Captured by thrown enemy
	void SetActive(bool act) { active = act; }
	void SetBounds(const Rectf& b) { bounds = b; }
	void SetVelocity(const Pointf& v) { velocity = v; }
	float GetGridSize(Player::CollisionHandler& collision) { return collision.GetGridSize(); }

	// Check if thrown enemy hit wall and should be destroyed
	bool CheckThrownWallCollision(float deltaX, Player::CollisionHandler& collision);
};

#endif
