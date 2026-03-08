# RainbowGame.java Conversion Task

## Overview
Convert the main RainbowGame.java file to U++ equivalent as part of the Umbrella project.

## Shell Command to View File
```bash
head -20 /common/active/sblo/Dev/RainbowGame/trash/core/src/main/java/com/rainbowgame/RainbowGame.java
```

## Original File
- Location: `/common/active/sblo/Dev/RainbowGame/trash/core/src/main/java/com/rainbowgame/RainbowGame.java`

## Purpose
This is the main game class that extends ApplicationAdapter and manages the core game loop and screens.

## Key Responsibilities
1. Manages the main game loop
2. Handles screen transitions (main menu, gameplay, editor, etc.)
3. Manages game state and resources
4. Coordinates with platform services
5. Handles application lifecycle events

## Conversion Strategy
1. Create a U++ equivalent class that manages game state
2. Implement the game loop using U++ timer mechanisms
3. Replace libGDX ApplicationAdapter with U++ equivalent
4. Implement screen management system
5. Integrate with U++ resource management

## Implementation Plan
1. Create UmbrellaGame class as U++ equivalent to RainbowGame
2. Implement game state management
3. Create screen transition system
4. Integrate with main.cpp application flow
5. Connect to platform services equivalent

## Dependencies
- Depends on various other game systems (camera, input, levels, etc.)
- Works with platform services
- Interacts with UI screens

## Testing Approach
1. Verify game loop functionality
2. Test screen transitions
3. Validate resource management
4. Confirm platform service integration