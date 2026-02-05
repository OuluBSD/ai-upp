#include "Umbrella.h"
#include "Player.h"
#include "Tile.h"

using namespace Upp;

Player::Player(float x, float y, float width, float height) {
	bounds = Rectf(x, y, x + width, y + height);
	velocity = Pointf(0, 0);
	facing = 1;
	onGround = false;
	coyoteTimer = 0.0f;
	jumpBufferTimer = 0.0f;
	jumpHeld = false;
	jumpHoldQueued = false;
	lives = 3;
	score = 0;
	invincibleTimer = 0.0f;
	knockbackTimer = 0.0f;
}

void Player::Update(float delta, const InputState& input, CollisionHandler& collision) {
	// Update timers
	if(invincibleTimer > 0.0f) {
		invincibleTimer = invincibleTimer - delta;
		if(invincibleTimer < 0.0f) invincibleTimer = 0.0f;
	}

	bool recovering = knockbackTimer > 0.0f;
	if(recovering) {
		knockbackTimer = knockbackTimer - delta;
		if(knockbackTimer < 0.0f) knockbackTimer = 0.0f;
	}

	// Jump buffering
	if(!recovering && input.jumpPressed) {
		jumpBufferTimer = JUMP_BUFFER_TIME;
		if(!onGround) {
			jumpHoldQueued = true;
		}
	} else {
		jumpBufferTimer = jumpBufferTimer - delta;
		if(jumpBufferTimer < 0.0f) jumpBufferTimer = 0.0f;
	}

	if(!input.jumpHeld) {
		jumpHoldQueued = false;
	} else if(jumpHoldQueued && !onGround) {
		jumpBufferTimer = JUMP_BUFFER_TIME;
	}

	// Coyote time
	if(onGround) {
		coyoteTimer = COYOTE_TIME;
	} else {
		coyoteTimer = coyoteTimer - delta;
		if(coyoteTimer < 0.0f) coyoteTimer = 0.0f;
	}

	// Horizontal movement (instant velocity, no lerping)
	if(!recovering) {
		bool touchingLeftWall = IsTouchingWallOnLeft(collision);
		bool touchingRightWall = IsTouchingWallOnRight(collision);

		if(input.moveLeft && !input.moveRight) {
			if(!touchingLeftWall) {
				velocity.x = -MOVE_SPEED;
			}
		} else if(input.moveRight && !input.moveLeft) {
			if(!touchingRightWall) {
				velocity.x = MOVE_SPEED;
			}
		} else {
			velocity.x = 0.0f;
		}
	} else {
		velocity.x = 0.0f;
	}

	// Update facing direction
	if(velocity.x > 5.0f) {
		facing = 1;
	} else if(velocity.x < -5.0f) {
		facing = -1;
	}

	// Jump logic
	bool wantsJump = jumpBufferTimer > 0.0f && (onGround || coyoteTimer > 0.0f);

	// Apply gravity
	velocity.y += GRAVITY * delta;
	if(velocity.y < MAX_FALL_SPEED) velocity.y = MAX_FALL_SPEED;

	if(wantsJump) {
		velocity.y = JUMP_VELOCITY;
		onGround = false;
		coyoteTimer = 0.0f;
		jumpBufferTimer = 0.0f;
		jumpHoldQueued = false;
	}

	// Variable jump height (release jump early)
	bool jumpHeldNow = input.jumpHeld;
	if(!jumpHeldNow && velocity.y > MIN_JUMP_VELOCITY && jumpHeld) {
		velocity.y = MIN_JUMP_VELOCITY;
	}
	jumpHeld = jumpHeldNow;

	// Apply movement with collision detection
	ResolveCollisionX(velocity.x * delta, collision);
	ResolveCollisionY(velocity.y * delta, collision);
}

void Player::ResolveCollisionX(float deltaX, CollisionHandler& collision) {
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

void Player::ResolveCollisionY(float deltaY, CollisionHandler& collision) {
	if(deltaY == 0.0f) return;

	float step = deltaY > 0 ? COLLISION_STEP : -COLLISION_STEP;
	float remaining = deltaY;

	onGround = false;

	while(abs(remaining) > COLLISION_STEP) {
		bounds.top += step;
		bounds.bottom += step;

		// Check collision
		// NOTE: In Y-up coordinates:
		//  - bounds.top is the LOWER Y value (player's feet/bottom)
		//  - bounds.bottom is the HIGHER Y value (player's head/top)
		//  - deltaY < 0 means falling (gravity), deltaY > 0 means jumping
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
					// In Y-up: deltaY < 0 means moving down (falling), hitting floor
					if(deltaY < 0) {
						onGround = true;
					}
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

	// Apply remaining movement and final ground check
	bounds.top += remaining;
	bounds.bottom += remaining;

	// Check if standing on ground (one pixel below player's feet)
	// In Y-up, feet are at the LOWER Y value
	int gridSize = (int)collision.GetGridSize();
	int minCol = (int)(bounds.left / gridSize);
	int maxCol = (int)(bounds.right / gridSize);
	float feetY = min(bounds.top, bounds.bottom);
	int floorRow = (int)((feetY - 1.0f) / gridSize);  // Check one pixel below feet

	for(int col = minCol; col <= maxCol; col++) {
		if(collision.IsWallTile(col, floorRow) || collision.IsFullBlockTile(col, floorRow)) {
			onGround = true;
			break;
		}
	}
}

bool Player::IsTouchingWallOnLeft(CollisionHandler& collision) {
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

bool Player::IsTouchingWallOnRight(CollisionHandler& collision) {
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

void Player::Render(Draw& w, CoordinateConverter& coords) {
	// Get world-space corners
	Point topLeft((int)bounds.left, (int)bounds.top);
	Point bottomRight((int)bounds.right, (int)bounds.bottom);

	// Convert to screen space (handles Y-flip)
	Point screenTopLeft = coords.WorldToScreen(topLeft);
	Point screenBottomRight = coords.WorldToScreen(bottomRight);

	// Since Y is flipped, screenBottomRight.y will be < screenTopLeft.y
	// Normalize to get proper screen rect
	int screenX = min(screenTopLeft.x, screenBottomRight.x);
	int screenY = min(screenTopLeft.y, screenBottomRight.y);
	int width = abs(screenBottomRight.x - screenTopLeft.x);
	int height = abs(screenBottomRight.y - screenTopLeft.y);

	// Draw player as blue rectangle (placeholder)
	Color playerColor = Color(50, 150, 255);
	if(invincibleTimer > 0.0f) {
		// Flash while invincible
		int flash = ((int)(invincibleTimer * 10) % 2);
		if(flash) playerColor = Color(255, 255, 255);
	}

	w.DrawRect(screenX, screenY, width, height, playerColor);

	// Draw facing direction indicator
	Color dirColor = Color(255, 255, 0);
	if(facing > 0) {
		w.DrawRect(screenX + width - 2, screenY + height/2 - 2, 4, 4, dirColor);
	} else {
		w.DrawRect(screenX - 2, screenY + height/2 - 2, 4, 4, dirColor);
	}
}

void Player::Render(Draw& w, Point cameraOffset, float zoom) {
	// Deprecated version - does NOT handle Y-flip correctly
	// Calculate screen position
	Point screenPos;
	screenPos.x = (int)((bounds.left - cameraOffset.x) * zoom);
	screenPos.y = (int)((bounds.top - cameraOffset.y) * zoom);

	int width = (int)(bounds.Width() * zoom);
	int height = (int)(bounds.Height() * zoom);

	// Draw player as blue rectangle (placeholder)
	Color playerColor = Color(50, 150, 255);
	if(invincibleTimer > 0.0f) {
		// Flash while invincible
		int flash = ((int)(invincibleTimer * 10) % 2);
		if(flash) playerColor = Color(255, 255, 255);
	}

	w.DrawRect(screenPos.x, screenPos.y, width, height, playerColor);

	// Draw facing direction indicator
	Color dirColor = Color(255, 255, 0);
	if(facing > 0) {
		w.DrawRect(screenPos.x + width - 2, screenPos.y + height/2 - 2, 4, 4, dirColor);
	} else {
		w.DrawRect(screenPos.x - 2, screenPos.y + height/2 - 2, 4, 4, dirColor);
	}
}

void Player::TakeDamage(int amount) {
	if(invincibleTimer > 0.0f) return;

	lives -= amount;
	invincibleTimer = 2.0f;  // 2 seconds of invincibility
	knockbackTimer = KNOCKBACK_DURATION;
}

void Player::SetPosition(float x, float y) {
	float width = bounds.Width();
	float height = bounds.Height();
	bounds = Rectf(x, y, x + width, y + height);
}
