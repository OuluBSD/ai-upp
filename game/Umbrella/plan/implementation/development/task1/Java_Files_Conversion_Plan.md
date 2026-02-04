# Java Files Conversion Plan

## Overview
Create individual tasks for converting each Java file in the RainbowGame project to U++ equivalents.

## Core Package Files
Located in `/common/active/sblo/Dev/RainbowGame/trash/core/src/main/java/com/rainbowgame/`

### Assets Module
- Assets.java - Asset management system
- AudioSystem.java - Audio playback system

### Camera Module  
- CameraSystem.java - Camera control and management

### Config Module
- GameConfig.java - Game configuration settings
- RainbowGameLaunchOptions.java - Application launch options

### Debug Module
- DebugOverlay.java - Debug information display

### Editor Module
- MapEditorScreen.java - Main map editor interface (special focus)
- MapPlaytestScreen.java - Map playtesting interface
- EditorBuildConfig.java - Editor build configuration
- EntityEditorScreen.java - Entity editing interface

### Editor Submodules
- annotation/ - Annotation and metadata systems
- entity/ - Entity management and serialization
- importer/ - Map import functionality
- options/ - Editor options and preferences
- persistable/ - Persistence systems
- save/ - Save/load functionality

### Gameplay Module
- components/ - Game entity components
- enemies/ - Enemy systems and behaviors
- rewards/ - Reward systems
- water/ - Water physics and droplet systems

### I18n Module
- LanguageOption.java - Language selection
- Strings.java - Localization system

### Input Module
- InputController.java - Input handling system

### Levels Module
- LevelLoader.java - Level loading system
- LevelManifest.java - Level manifest management
- LevelSyncManager.java - Level synchronization
- LiveLevelHandle.java - Live level editing
- LiveLevelRegistry.java - Level registry

### Mod Module
- ModDefinition.java - Modding support

### Platform Module
- AdBridge.java - Advertising bridge
- AdBridgeNoop.java - No-op advertising bridge
- AdsConfig.java - Advertising configuration
- PlatformServices.java - Platform services
- PlatformType.java - Platform type enumeration

### Save Module
- SaveSystem.java - Game save system

### Settings Module
- Settings.java - Game settings

### UI Module
- HUD.java - Heads-up display

### UI Screens Module
- BootScreen.java - Boot screen
- GameOverScreen.java - Game over screen
- GameScreen.java - Main game screen
- LanguageSelectScreen.java - Language selection
- MainMenuScreen.java - Main menu
- PauseScreen.java - Pause screen
- PlaytestStartResolver.java - Playtest resolver
- SettingsScreen.java - Settings screen
- WorldSelectScreen.java - World selection

### Util Module
- FixedTimeStepRunner.java - Fixed timestep management

### Main Class
- RainbowGame.java - Main application class

## Desktop Launcher
Located in `/common/active/sblo/Dev/RainbowGame/trash/desktop/src/main/java/com/rainbowgame/desktop/`
- DesktopLauncher.java - Desktop application entry point

## Conversion Approach
1. Create individual tasks for each Java file conversion
2. Prioritize files based on dependencies and importance
3. Focus special attention on MapEditorScreen.java due to its complexity
4. Convert libGDX-dependent code to U++ equivalents
5. Implement U++ drawing using Ctrl::Paint(Draw&) override
6. Implement U++ input using Ctrl::Key(...) override
7. Implement U++ sound using uppsrc/Sound

## Dependencies
- Identify interdependencies between Java files
- Convert in dependency order
- Maintain functionality during conversion

## Success Criteria
- All Java files converted to U++ equivalents
- Same functionality preserved
- Proper integration with U++ framework
- Clean, maintainable code following U++ conventions