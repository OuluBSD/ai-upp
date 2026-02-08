#ifndef _Umbrella_GameSettings_h_
#define _Umbrella_GameSettings_h_

namespace Upp {

// Game configuration constants
struct GameSettings {
	// Thrown Enemy Capture Settings
	static constexpr float THROWN_ENEMY_SIZE_TOLERANCE = 1.75f;  // Target can be up to 75% larger than thrower

	// TODO: Add other game settings here as they're identified:
	// - Enemy AI parameters (speeds, jump heights, etc.)
	// - Player physics constants
	// - Combat parameters (damage, knockback, etc.)
	// - Scoring values
	// - Visual effects parameters
};

}

#endif
