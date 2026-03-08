# Umbrella Game Component Tests

This directory contains tests for the Umbrella game components that were converted from RainbowGame.

## LevelLoader Test

**Directory**: `UmbrellaLevelLoaderTest`

**Purpose**: Test the LevelLoader component responsible for loading and saving game levels.

**Features tested**:
- Level loading and saving functionality using ShareDirFile for proper file paths
- Tile management system
- Spawn point and entity handling
- Tile placement and retrieval
- Loads levels from `~/Dev/ai-upp/share/mods/umbrella/levels/` directory structure

**Build command**: `script/build.py -j12 -mc 0 UmbrellaLevelLoaderTest`
**Run command**: `bin/UmbrellaLevelLoaderTest`

## AudioSystem Test

**Directory**: `UmbrellaAudioSystemTest`

**Purpose**: Test the AudioSystem component responsible for sound and music playback.

**Features tested**:
- Sound loading with different types (SFX, MUSIC, VOICE) using ShareDirFile for proper file paths
- Sound playback functionality
- Volume control for individual sounds and master volume
- Sound management and lookup
- Loads sounds from `~/Dev/ai-upp/share/mods/umbrella/sounds/` directory structure

**Build command**: `script/build.py -j12 -mc 0 UmbrellaAudioSystemTest`
**Run command**: `bin/UmbrellaAudioSystemTest`

## Implementation Notes

Both tests were implemented as console applications to avoid GUI dependencies. They use simplified versions of the actual components that are compatible with U++'s console application requirements.

The tests verify basic functionality of the components and ensure they can be integrated into the broader Umbrella game system. Both components now properly use ShareDirFile to locate assets in the correct shared directory structure (mods/umbrella/levels/ and mods/umbrella/sounds/).