#include "Umbrella.h"
#include "EnemyJumper.h"

using namespace Upp;

EnemyJumper::EnemyJumper(float x, float y)
	: Enemy(x, y, 12, 12, ENEMY_JUMPER)
{
	velocity.x = -WALK_SPEED;  // Start moving left
	facing = -1;
	ResetJumpTimer();
}

void EnemyJumper::ResetJumpTimer() {
	// Random jump interval between MIN and MAX
	nextJumpTime = MIN_JUMP_INTERVAL + (Randomf() * (MAX_JUMP_INTERVAL - MIN_JUMP_INTERVAL));
	jumpTimer = 0.0f;
}

bool EnemyJumper::IsOnGround(Player::CollisionHandler& collision) {
	int gridSize = (int)collision.GetGridSize();
	int minCol = (int)(bounds.left / gridSize);
	int maxCol = (int)(bounds.right / gridSize);

	// In Y-up, feet are at the LOWER Y value
	float feetY = min(bounds.top, bounds.bottom);
	int floorRow = (int)((feetY - 1.0f) / gridSize);

	for(int col = minCol; col <= maxCol; col++) {
		if(collision.IsFloorTile(col, floorRow)) {
			return true;
		}
	}
	return false;
}

void EnemyJumper::Update(float delta, const Player& player, Player::CollisionHandler& collision) {
	if(!alive || !active) return;

	// If thrown, skip both gravity and AI - purely horizontal movement
	if(!thrown) {
		// Apply gravity
		velocity.y += GRAVITY * delta;
		if(velocity.y < MAX_FALL_SPEED) velocity.y = MAX_FALL_SPEED;
		// Set horizontal velocity based on facing
		velocity.x = facing * WALK_SPEED;

		// Check for walls - turn around and jump
		bool hitWall = (facing < 0 && IsTouchingWallOnLeft(collision)) ||
		               (facing > 0 && IsTouchingWallOnRight(collision));

		bool noFloorAhead = !IsFloorAhead(collision) && IsOnGround(collision);

		if(hitWall || noFloorAhead) {
			facing = -facing;
			velocity.x = facing * WALK_SPEED;

			// Jump when blocked or at edge
			if(IsOnGround(collision)) {
				velocity.y = JUMP_VELOCITY;
				ResetJumpTimer();
			}
		}

		// Jump timer - periodic jumps
		jumpTimer += delta;
		if(jumpTimer >= nextJumpTime && IsOnGround(collision)) {
			velocity.y = JUMP_VELOCITY;
			ResetJumpTimer();
		}
	}
	else {
		// Check for wall collision when thrown
		if(CheckThrownWallCollision(velocity.x * delta, collision)) {
			// Mark for destruction - GameScreen will spawn treat
			Defeat();
			return;
		}
	}

	// Apply movement with collision
	ResolveCollisionX(velocity.x * delta, collision);
	ResolveCollisionY(velocity.y * delta, collision);

	// Kill if falls off map
	float feetY = min(bounds.top, bounds.bottom);
	if(feetY < -collision.GetGridSize() * 2) {
		Kill();
	}
}

void EnemyJumper::Render(Draw& w, Player::CoordinateConverter& coords) {
	if(!alive) return;

	// Get world-space corners
	Point topLeft((int)bounds.left, (int)bounds.top);
	Point bottomRight((int)bounds.right, (int)bounds.bottom);

	// Convert to screen space (handles Y-flip)
	Point screenTopLeft = coords.WorldToScreen(topLeft);
	Point screenBottomRight = coords.WorldToScreen(bottomRight);

	// Normalize to get proper screen rect
	int screenX = min(screenTopLeft.x, screenBottomRight.x);
	int screenY = min(screenTopLeft.y, screenBottomRight.y);
	int width = abs(screenBottomRight.x - screenTopLeft.x);
	int height = abs(screenBottomRight.y - screenTopLeft.y);

	// Draw enemy as green rectangle (different from patroller's red)
	Color enemyColor = Color(50, 255, 50);
	w.DrawRect(screenX, screenY, width, height, enemyColor);

	// Draw facing direction indicator
	Color dirColor = Color(255, 255, 0);
	if(facing > 0) {
		w.DrawRect(screenX + width - 2, screenY + height/2 - 2, 4, 4, dirColor);
	} else {
		w.DrawRect(screenX - 2, screenY + height/2 - 2, 4, 4, dirColor);
	}
}
