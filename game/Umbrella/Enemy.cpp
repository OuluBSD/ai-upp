#include "Umbrella.h"
#include "Enemy.h"

using namespace Upp;

Enemy::Enemy(float x, float y, float width, float height, EnemyType t) {
	bounds = Rectf(x, y, x + width, y + height);
	velocity = Pointf(0, 0);
	health = 1;
	alive = true;
	active = true;
	captured = false;
	thrown = false;
	stateTimer = 0.0f;
	facing = -1;  // Start facing left
	type = t;
	carryWeight = 1.0f;  // Default weight
}

void Enemy::TakeDamage(int amount) {
	health -= amount;
	if(health <= 0) {
		Kill();
	}
}

void Enemy::Kill() {
	alive = false;
	active = false;
}

void Enemy::Defeat() {
	// Killed by player parasol attack
	// Mark as dead (will trigger reward spawning in GameScreen)
	alive = false;
	active = false;
	captured = false;
	// TODO: Play defeat sound
	// TODO: Spawn particle effect
}

void Enemy::Capture() {
	// Enemy captured by player umbrella
	captured = true;
	active = false;
	velocity = Pointf(0, 0);  // Stop movement
}

void Enemy::ThrowFrom(float x, float y, float vx, float vy) {
	// Thrown by player - reactivate with new position and velocity
	bounds.left = x;
	bounds.right = x + bounds.Width();
	bounds.top = y;
	bounds.bottom = y + bounds.Height();
	velocity.x = vx;
	velocity.y = 0.0f;  // Thrown enemies move purely horizontal (gravity will pull down)
	captured = false;
	thrown = true;
	active = true;
}

bool Enemy::IsTouchingWallOnLeft(Player::CollisionHandler& collision) {
	int gridSize = (int)collision.GetGridSize();
	int checkCol = (int)((bounds.left - 1.0f) / gridSize);

	// Get Y range - remember bounds naming is backwards for Y-up!
	float minY = min(bounds.top, bounds.bottom);
	float maxY = max(bounds.top, bounds.bottom);
	int minRow = (int)(minY / gridSize);
	int maxRow = (int)(maxY / gridSize);

	for(int row = minRow; row <= maxRow; row++) {
		if(collision.IsWallTile(checkCol, row) || collision.IsFullBlockTile(checkCol, row)) {
			return true;
		}
	}
	return false;
}

bool Enemy::IsTouchingWallOnRight(Player::CollisionHandler& collision) {
	int gridSize = (int)collision.GetGridSize();
	int checkCol = (int)((bounds.right + 1.0f) / gridSize);

	// Get Y range - remember bounds naming is backwards for Y-up!
	float minY = min(bounds.top, bounds.bottom);
	float maxY = max(bounds.top, bounds.bottom);
	int minRow = (int)(minY / gridSize);
	int maxRow = (int)(maxY / gridSize);

	for(int row = minRow; row <= maxRow; row++) {
		if(collision.IsWallTile(checkCol, row) || collision.IsFullBlockTile(checkCol, row)) {
			return true;
		}
	}
	return false;
}

bool Enemy::IsOnGround(Player::CollisionHandler& collision) {
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

bool Enemy::IsFloorAhead(Player::CollisionHandler& collision) {
	// Check if there's floor ahead in the direction enemy is facing
	int gridSize = (int)collision.GetGridSize();

	// Check one tile ahead
	int checkCol = facing < 0 ? (int)((bounds.left - gridSize) / gridSize)
	                           : (int)((bounds.right + gridSize) / gridSize);

	// Check at feet level
	float feetY = min(bounds.top, bounds.bottom);
	int floorRow = (int)((feetY - 1.0f) / gridSize);

	return collision.IsFloorTile(checkCol, floorRow);
}

void Enemy::ResolveCollisionX(float deltaX, Player::CollisionHandler& collision) {
	if(deltaX == 0.0f) return;

	float step = deltaX > 0 ? COLLISION_STEP : -COLLISION_STEP;
	float remaining = deltaX;

	while(abs(remaining) > COLLISION_STEP) {
		bounds.left += step;
		bounds.right += step;

		// Check collision
		int gridSize = (int)collision.GetGridSize();
		int minCol = (int)(bounds.left / gridSize);
		int maxCol = (int)(bounds.right / gridSize);

		// Get Y range - remember bounds naming is backwards for Y-up!
		float minY = min(bounds.top, bounds.bottom);
		float maxY = max(bounds.top, bounds.bottom);
		int minRow = (int)(minY / gridSize);
		int maxRow = (int)(maxY / gridSize);

		bool collided = false;
		for(int row = minRow; row <= maxRow; row++) {
			for(int col = minCol; col <= maxCol; col++) {
				if(collision.IsWallTile(col, row) || collision.IsFullBlockTile(col, row)) {
					collided = true;
					break;
				}
			}
			if(collided) break;
		}

		if(collided) {
			bounds.left -= step;
			bounds.right -= step;
			velocity.x = 0.0f;
			return;
		}

		remaining -= step;
	}

	// Apply remaining movement
	bounds.left += remaining;
	bounds.right += remaining;
}

void Enemy::ResolveCollisionY(float deltaY, Player::CollisionHandler& collision) {
	if(deltaY == 0.0f) return;

	float step = deltaY > 0 ? COLLISION_STEP : -COLLISION_STEP;
	float remaining = deltaY;

	while(abs(remaining) > COLLISION_STEP) {
		bounds.top += step;
		bounds.bottom += step;

		// Check collision
		int gridSize = (int)collision.GetGridSize();
		int minCol = (int)(bounds.left / gridSize);
		int maxCol = (int)(bounds.right / gridSize);

		// Get Y range - remember bounds naming is backwards for Y-up!
		float minY = min(bounds.top, bounds.bottom);
		float maxY = max(bounds.top, bounds.bottom);
		int minRow = (int)(minY / gridSize);
		int maxRow = (int)(maxY / gridSize);

		bool collided = false;
		for(int row = minRow; row <= maxRow; row++) {
			for(int col = minCol; col <= maxCol; col++) {
				if(collision.IsWallTile(col, row) || collision.IsFullBlockTile(col, row)) {
					collided = true;
					break;
				}
			}
			if(collided) break;
		}

		if(collided) {
			bounds.top -= step;
			bounds.bottom -= step;
			velocity.y = 0.0f;
			return;
		}

		remaining -= step;
	}

	// Apply remaining movement
	bounds.top += remaining;
	bounds.bottom += remaining;
}

bool Enemy::CheckThrownWallCollision(float deltaX, Player::CollisionHandler& collision) {
	// Only check if thrown enemy is moving horizontally
	if(!thrown || deltaX == 0.0f) return false;

	// Check if enemy will hit wall in movement direction
	int gridSize = (int)collision.GetGridSize();

	// Get Y range
	float minY = min(bounds.top, bounds.bottom);
	float maxY = max(bounds.top, bounds.bottom);
	int minRow = (int)(minY / gridSize);
	int maxRow = (int)(maxY / gridSize);

	// Check column ahead of movement
	int checkCol = deltaX > 0 ? (int)((bounds.right + abs(deltaX)) / gridSize)
	                           : (int)((bounds.left + deltaX) / gridSize);

	// Check if wall exists in path
	for(int row = minRow; row <= maxRow; row++) {
		if(collision.IsWallTile(checkCol, row) || collision.IsFullBlockTile(checkCol, row)) {
			// Wall collision detected - mark enemy for destruction
			return true;
		}
	}

	return false;
}
