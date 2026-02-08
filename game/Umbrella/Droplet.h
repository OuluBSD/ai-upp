#ifndef _Umbrella_Droplet_h_
#define _Umbrella_Droplet_h_

#include <Core/Core.h>
#include "Player.h"

using namespace Upp;

enum DropletType {
	DROPLET_RAINBOW,  // All colors cycling
	DROPLET_ICE,      // Cyan/blue
	DROPLET_FIRE      // Orange/red
};

// Spawn point for droplets (from editor)
struct DropletSpawnPoint {
	int col;
	int row;
	DropletType mode;
	int direction;        // -1 left, 1 right (horizontal velocity)
	int intervalMs;       // Spawn interval in milliseconds (0 for one-shot)
	bool enabled;
	float timer;          // Internal timer for spawning

	DropletSpawnPoint() : col(0), row(0), mode(DROPLET_RAINBOW),
	                      direction(1), intervalMs(2000), enabled(true), timer(0.0f) {}
};

class Droplet {
private:
	Pointf position;
	Pointf velocity;
	DropletType type;
	bool active;
	bool collected;       // True if collected and orbiting player
	float orbitAngle;     // Angle in orbit around player (radians)
	float rotation;       // Rotation angle in degrees
	float rotationSpeed;  // Degrees per second
	float size;           // Droplet radius

	// Physics constants
	static constexpr float GRAVITY = -490.0f;  // Match player gravity
	static constexpr float MAX_FALL_SPEED = -400.0f;
	static constexpr float BOUNCE_DAMPING = 0.6f;  // Velocity multiplier on bounce
	static constexpr float MIN_BOUNCE_VY = 50.0f;  // Minimum vertical velocity to bounce
	static constexpr float DROPLET_SIZE = 6.0f;  // Default droplet radius

	// Collision helpers
	bool CheckGroundCollision(Player::CollisionHandler& collision);
	bool CheckWallCollision(Player::CollisionHandler& collision);

public:
	Droplet(float x, float y, DropletType t);

	void Update(float delta, Player::CollisionHandler& collision);
	void UpdateOrbit(float delta, Pointf playerPos, int dropletIndex, int totalDroplets);
	void Render(Draw& w, Player::CoordinateConverter& coords);

	bool IsActive() const { return active; }
	bool IsCollected() const { return collected; }
	void Collect(float angle) {
		collected = true;
		orbitAngle = angle;
	}
	void Deactivate() { active = false; }
	Pointf GetPosition() const { return position; }
	Pointf GetVelocity() const { return velocity; }
	void SetVelocity(Pointf vel) { velocity = vel; }
	float GetSize() const { return size; }
	DropletType GetType() const { return type; }

	// Collision bounds
	Rectf GetBounds() const {
		return Rectf(position.x - size, position.y - size,
		             position.x + size, position.y + size);
	}
};

#endif
