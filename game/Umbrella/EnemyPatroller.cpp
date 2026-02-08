#include "Umbrella.h"
#include "EnemyPatroller.h"

using namespace Upp;

EnemyPatroller::EnemyPatroller(float x, float y)
	: Enemy(x, y, 12, 12, ENEMY_PATROLLER)
{
	velocity.x = -WALK_SPEED;  // Start moving left
	facing = -1;
}

void EnemyPatroller::Update(float delta, const Player& player, Player::CollisionHandler& collision) {
	// Update rotation for visual effects (works for dead enemies too)
	UpdateRotation(delta);

	// Dead enemies: apply physics until ground collision
	if(!alive) {
		velocity.y += GRAVITY * delta;
		if(velocity.y < MAX_FALL_SPEED) velocity.y = MAX_FALL_SPEED;

		ResolveCollisionX(velocity.x * delta, collision);
		ResolveCollisionY(velocity.y * delta, collision);

		// Check if hit ground - will be handled in GameScreen for treat spawning
		return;
	}

	// If carried by thrown enemy, just apply movement (no AI, no gravity)
	if(carriedByThrown) {
		ResolveCollisionX(velocity.x * delta, collision);
		return;
	}

	if(!active) return;

	// If thrown, skip both gravity and AI - purely horizontal movement
	if(!thrown) {
		// Apply gravity
		velocity.y += GRAVITY * delta;
		if(velocity.y < MAX_FALL_SPEED) velocity.y = MAX_FALL_SPEED;
		// Set horizontal velocity based on facing
		velocity.x = facing * WALK_SPEED;

		// Check for walls or edges - turn around
		bool hitWall = (facing < 0 && IsTouchingWallOnLeft(collision)) ||
		               (facing > 0 && IsTouchingWallOnRight(collision));

		bool noFloorAhead = !IsFloorAhead(collision) && IsOnGround(collision);

		if(hitWall || noFloorAhead) {
			facing = -facing;
			velocity.x = facing * WALK_SPEED;
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

void EnemyPatroller::Render(Draw& w, Player::CoordinateConverter& coords) {
	// Render dead enemies too (they fly through air with red tint)

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

	// Draw enemy with state-based tint
	Color baseColor = Color(255, 50, 50);  // Red base color
	Color tint = GetTintColor();

	// Apply tint (simple color modulation)
	int r = (baseColor.GetR() * tint.GetR()) / 255;
	int g = (baseColor.GetG() * tint.GetG()) / 255;
	int b = (baseColor.GetB() * tint.GetB()) / 255;
	Color enemyColor = Color(r, g, b);

	w.DrawRect(screenX, screenY, width, height, enemyColor);

	// Draw facing direction indicator
	Color dirColor = Color(255, 255, 0);
	if(facing > 0) {
		w.DrawRect(screenX + width - 2, screenY + height/2 - 2, 4, 4, dirColor);
	} else {
		w.DrawRect(screenX - 2, screenY + height/2 - 2, 4, 4, dirColor);
	}
}
