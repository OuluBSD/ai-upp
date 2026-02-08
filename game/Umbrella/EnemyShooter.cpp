#include "Umbrella.h"
#include "EnemyShooter.h"

using namespace Upp;

EnemyShooter::EnemyShooter(float x, float y)
	: Enemy(x, y, 14, 14, ENEMY_SHOOTER)
{
	velocity.x = 0.0f;  // Stationary
	velocity.y = 0.0f;
	facing = 1;  // Face right initially
	shootTimer = 0.0f;
}

EnemyShooter::~EnemyShooter() {
	for(int i = 0; i < projectiles.GetCount(); i++) {
		delete projectiles[i];
	}
	projectiles.Clear();
}

bool EnemyShooter::CanSeePlayer(const Player& player) {
	Pointf playerPos = player.GetPosition();
	Pointf enemyCenter;
	enemyCenter.x = (bounds.left + bounds.right) / 2.0f;
	enemyCenter.y = (bounds.top + bounds.bottom) / 2.0f;

	// Calculate distance
	float dx = playerPos.x - enemyCenter.x;
	float dy = playerPos.y - enemyCenter.y;
	float distSq = dx * dx + dy * dy;

	// Check if within range
	if(distSq > DETECTION_RANGE * DETECTION_RANGE) {
		return false;
	}

	// Update facing direction based on player position
	facing = (dx > 0) ? 1 : -1;

	return true;
}

void EnemyShooter::ShootAtPlayer(const Player& player) {
	// Spawn projectile at center of shooter
	float spawnX = (bounds.left + bounds.right) / 2.0f - 3.0f;
	float spawnY = (bounds.top + bounds.bottom) / 2.0f - 3.0f;

	projectiles.Add(new Projectile(spawnX, spawnY, facing));
}

void EnemyShooter::UpdateProjectiles(float delta, Player::CollisionHandler& collision) {
	// Update all projectiles
	for(int i = 0; i < projectiles.GetCount(); i++) {
		projectiles[i]->Update(delta, collision);
	}

	// Remove inactive projectiles
	for(int i = projectiles.GetCount() - 1; i >= 0; i--) {
		if(!projectiles[i]->IsActive()) {
			delete projectiles[i];
			projectiles.Remove(i);
		}
	}
}

void EnemyShooter::Update(float delta, const Player& player, Player::CollisionHandler& collision) {
	if(!alive || !active) return;

	// If thrown, purely horizontal movement (no gravity)
	if(thrown) {
		// Check for wall collision when thrown
		if(CheckThrownWallCollision(velocity.x * delta, collision)) {
			// Mark for destruction - GameScreen will spawn treat
			Defeat();
			return;
		}

		// No gravity - purely horizontal flight
		ResolveCollisionX(velocity.x * delta, collision);
		// No Y collision needed since no vertical movement
	}
	else {
		// Normal shooter behavior: stationary on platforms
		// No gravity, no movement

		// Update shoot timer
		shootTimer += delta;

		// Check if can see player and ready to shoot
		if(CanSeePlayer(player) && shootTimer >= SHOOT_COOLDOWN) {
			ShootAtPlayer(player);
			shootTimer = 0.0f;
		}
	}

	// Update projectiles
	UpdateProjectiles(delta, collision);
}

void EnemyShooter::Render(Draw& w, Player::CoordinateConverter& coords) {
	if(!alive) return;

	// Render projectiles first (behind enemy)
	for(int i = 0; i < projectiles.GetCount(); i++) {
		projectiles[i]->Render(w, coords);
	}

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

	// Draw enemy as yellow rectangle (stationary shooter)
	Color enemyColor = Color(255, 255, 50);
	w.DrawRect(screenX, screenY, width, height, enemyColor);

	// Draw facing direction indicator
	Color dirColor = Color(255, 100, 100);
	if(facing > 0) {
		w.DrawRect(screenX + width - 2, screenY + height/2 - 2, 4, 4, dirColor);
	} else {
		w.DrawRect(screenX - 2, screenY + height/2 - 2, 4, 4, dirColor);
	}
}
