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

	// Public constants for GameScreen to access
	static constexpr float THROW_VELOCITY_X = 350.0f;  // Horizontal throw speed
	static constexpr float THROW_VELOCITY_Y = 100.0f;  // Upward component

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
	float speedBoostTimer;

	// Parasol state
	enum ParasolState {
		PARASOL_IDLE,
		PARASOL_ATTACKING,
		PARASOL_GLIDING
	};
	ParasolState parasolState;
	Rectf parasolHitbox;
	bool attackHeld;
	bool wasAttackHeld;
	float attackTimer;
	float attackCooldown;
	bool forceUmbrellaOnTop;  // Keep umbrella on top (for droplets)

	// Carried enemies on umbrella
	Array<class Enemy*> carriedEnemies;
	float carryWeight;
	float maxCarryWeight;

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

	// Parasol constants
	static constexpr float PARASOL_ATTACK_DURATION = 0.25f;
	static constexpr float PARASOL_ATTACK_COOLDOWN = 0.3f;
	static constexpr float PARASOL_GLIDE_GRAVITY_SCALE = 0.35f;

	// Collision helpers
	bool IsTouchingWallOnLeft(CollisionHandler& collision);
	bool IsTouchingWallOnRight(CollisionHandler& collision);
	void ResolveCollisionX(float deltaX, CollisionHandler& collision);
	void ResolveCollisionY(float deltaY, CollisionHandler& collision);

public:
	Player(float x, float y, float width, float height);

	void Update(float delta, const InputState& input, CollisionHandler& collision);

	// Render using direct screen coordinates (deprecated - use version with GameScreen)
	void Render(Draw& w, Point cameraOffset, float zoom);

	// Forward declaration for coordinate converter interface
	class CoordinateConverter {
	public:
		virtual Point WorldToScreen(Point worldPos) = 0;
	};

	// Render using GameScreen's coordinate conversion
	void Render(Draw& w, CoordinateConverter& coords);

	// Accessors
	Rectf GetBounds() const { return bounds; }
	Pointf GetPosition() const { return Pointf(bounds.left, bounds.top); }
	Pointf GetCenter() const { return Pointf(bounds.left + bounds.Width()/2, bounds.top + bounds.Height()/2); }
	int  GetLives() const { return lives; }
	void ResetLives()    { lives = 3; }

	bool IsInvincible()   const { return invincibleTimer > 0.0f; }
	bool IsSpeedBoosted() const { return speedBoostTimer > 0.0f; }
	void SetInvincible(float t)   { invincibleTimer  = t; }
	void SetSpeedBoost(float t)   { speedBoostTimer  = t; }
	int GetScore() const { return score; }
	bool IsOnGround() const { return onGround; }
	int GetFacing() const { return facing; }
	Rectf GetParasolHitbox() const { return parasolHitbox; }
	bool IsAttacking() const { return parasolState == PARASOL_ATTACKING; }
	bool IsGliding() const { return parasolState == PARASOL_GLIDING; }
	ParasolState GetParasolState() const { return parasolState; }
	const Array<class Enemy*>& GetCapturedEnemies() const { return carriedEnemies; }

	// Enemy capture
	bool CanCapture(class Enemy* enemy) const;
	void CaptureEnemy(class Enemy* enemy);
	bool HasCapturedEnemies() const { return carriedEnemies.GetCount() > 0; }
	void AlignCapturedEnemies();
	class Enemy* ReleaseCapturedEnemy();  // Returns the enemy to throw

	// Mutators
	void TakeDamage(int amount);
	void AddScore(int points) { score += points; }
	void SetPosition(float x, float y);

	// Forced state (for level transitions and droplets)
	void ForceGlideState() { parasolState = PARASOL_GLIDING; }
	void ForceIdleState() { parasolState = PARASOL_IDLE; }
	void ForceUmbrellaOnTop(bool force);  // Force umbrella to stay on top (for droplets)
	Pointf GetVelocity() const { return velocity; }
	void SetVelocity(const Pointf& vel) { velocity = vel; }
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
