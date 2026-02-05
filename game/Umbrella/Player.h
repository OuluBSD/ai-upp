#ifndef _Umbrella_Player_h_
#define _Umbrella_Player_h_

#include <Core/Core.h>

using namespace Upp;

// Forward declarations
struct InputState;
class CollisionHandler;

class Player {
public:
	// Collision handler interface
	class CollisionHandler {
	public:
		virtual bool IsFullBlockTile(int col, int row) = 0;
		virtual bool IsWallTile(int col, int row) = 0;
		virtual bool IsFloorTile(int col, int row) = 0;
		virtual float GetGridSize() = 0;
	};

private:
	Rectf bounds;
	Pointf velocity;
	int facing;          // -1 left, 1 right
	bool onGround;
	float coyoteTimer;
	float jumpBufferTimer;
	bool jumpHeld;
	bool jumpHoldQueued;
	int lives;
	int score;
	float invincibleTimer;
	float knockbackTimer;

	// Physics constants (from RainbowGame Player.java)
	static constexpr float GRID_SIZE = 14.0f;
	static constexpr float MOVE_SPEED = GRID_SIZE * 10.0f;  // 140.0f
	static constexpr float GRAVITY = -GRID_SIZE * 35.0f;    // -490.0f
	static constexpr float MAX_FALL_SPEED = -GRID_SIZE * 20.0f;  // -280.0f
	static constexpr float JUMP_VELOCITY = GRID_SIZE * 20.0f;    // 280.0f
	static constexpr float MIN_JUMP_VELOCITY = GRID_SIZE * 7.0f; // 98.0f
	static constexpr float COYOTE_TIME = 0.1f;
	static constexpr float JUMP_BUFFER_TIME = 0.12f;
	static constexpr float COLLISION_STEP = 3.5f;  // Max pixels per collision check
	static constexpr float KNOCKBACK_DURATION = 0.35f;

	// Collision helpers
	bool IsTouchingWallOnLeft(CollisionHandler& collision);
	bool IsTouchingWallOnRight(CollisionHandler& collision);
	void ResolveCollisionX(float deltaX, CollisionHandler& collision);
	void ResolveCollisionY(float deltaY, CollisionHandler& collision);

public:
	Player(float x, float y, float width, float height);

	void Update(float delta, const InputState& input, CollisionHandler& collision);
	void Render(Draw& w, Point cameraOffset, float zoom);

	// Accessors
	Rectf GetBounds() const { return bounds; }
	Pointf GetPosition() const { return Pointf(bounds.left, bounds.top); }
	Pointf GetCenter() const { return Pointf(bounds.left + bounds.Width()/2, bounds.top + bounds.Height()/2); }
	int GetLives() const { return lives; }
	int GetScore() const { return score; }
	bool IsOnGround() const { return onGround; }
	int GetFacing() const { return facing; }

	// Mutators
	void TakeDamage(int amount);
	void AddScore(int points) { score += points; }
	void SetPosition(float x, float y);
};

// Input state structure
struct InputState {
	bool moveLeft;
	bool moveRight;
	bool jumpHeld;
	bool jumpPressed;
	bool attackPressed;
	bool glideHeld;
	bool pausePressed;

	InputState() : moveLeft(false), moveRight(false), jumpHeld(false),
	               jumpPressed(false), attackPressed(false), glideHeld(false),
	               pausePressed(false) {}
};

#endif
