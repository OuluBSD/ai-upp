#ifndef _Umbrella_GameSettings_h_
#define _Umbrella_GameSettings_h_

namespace Upp {

// Game configuration constants
struct GameSettings {
	// Thrown Enemy Capture Settings
	static constexpr float THROWN_ENEMY_SIZE_TOLERANCE = 1.75f;  // Target can be up to 75% larger than thrower

	// Enemy Visual Effects
	static constexpr float ENEMY_ROTATION_SPEED = 720.0f;  // Degrees per second (2 full rotations)

	// Enemy State Colors (RGB)
	static constexpr int DEACTIVATED_TINT_R = 0;    // Green tint for captured/thrown enemies
	static constexpr int DEACTIVATED_TINT_G = 255;
	static constexpr int DEACTIVATED_TINT_B = 100;

	static constexpr int DEAD_TINT_R = 255;  // Red tint for dead enemies
	static constexpr int DEAD_TINT_G = 50;
	static constexpr int DEAD_TINT_B = 50;

	static constexpr int NORMAL_TINT_R = 255;  // No tint for normal enemies
	static constexpr int NORMAL_TINT_G = 255;
	static constexpr int NORMAL_TINT_B = 255;

	// TODO: Add other game settings here as they're identified:
	// - Enemy AI parameters (speeds, jump heights, etc.)
	// - Player physics constants
	// - Combat parameters (damage, knockback, etc.)
	// - Scoring values
};

}

#endif
