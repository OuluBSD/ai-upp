# Java Files Conversion Tracking Task

## Overview
Track the conversion of all Java files from RainbowGame to U++ equivalents in the Umbrella project.

## Shell Command to Explore Files
```bash
find /common/active/sblo/Dev/RainbowGame/trash/core/src/main/java -name "*.java"
```

## Files to Convert

### Assets Module
- [ ] Assets.java - Asset management system
- [ ] AudioSystem.java - Audio playback system (will use uppsrc/Sound)

### Camera Module
- [ ] CameraSystem.java - Camera control and management

### Config Module
- [ ] GameConfig.java - Game configuration settings
- [ ] RainbowGameLaunchOptions.java - Application launch options

### Debug Module
- [ ] DebugOverlay.java - Debug information display

### Editor Module
- [ ] MapEditorScreen.java - Main map editor interface (high priority)
- [ ] MapPlaytestScreen.java - Map playtesting interface
- [ ] EditorBuildConfig.java - Editor build configuration
- [ ] TextureListPanel.java - Texture panel UI component

### Editor Submodules
- [ ] annotation/ - Annotation and metadata systems (11 files)
- [ ] entity/ - Entity management and serialization (14 files)
- [ ] importer/ - Map import functionality (3 files)
- [ ] options/ - Editor options and preferences (4 files)
- [ ] persistable/ - Persistence systems (6 files)
- [ ] save/ - Save/load functionality (2 files)

### Gameplay Module
- [ ] components/ - Game entity components (7 files)
- [ ] enemies/ - Enemy systems and behaviors (25 files)
- [ ] rewards/ - Reward systems (4 files)
- [ ] water/ - Water physics and droplet systems (11 files)

### I18n Module
- [ ] LanguageOption.java - Language selection
- [ ] Strings.java - Localization system

### Input Module
- [ ] InputController.java - Input handling system

### Levels Module
- [ ] LevelLoader.java - Level loading system
- [ ] LevelManifest.java - Level manifest management
- [ ] LevelSyncManager.java - Level synchronization
- [ ] LiveLevelHandle.java - Live level editing
- [ ] LiveLevelRegistry.java - Level registry

### Mod Module
- [ ] ModDefinition.java - Modding support

### Platform Module
- [ ] AdBridge.java - Advertising bridge
- [ ] AdBridgeNoop.java - No-op advertising bridge
- [ ] AdsConfig.java - Advertising configuration
- [ ] PlatformServices.java - Platform services
- [ ] PlatformType.java - Platform type enumeration

### Save Module
- [ ] SaveSystem.java - Game save system

### Settings Module
- [ ] Settings.java - Game settings

### UI Module
- [ ] HUD.java - Heads-up display

### UI Screens Module
- [ ] BootScreen.java - Boot screen
- [ ] GameOverScreen.java - Game over screen
- [ ] GameScreen.java - Main game screen
- [ ] LanguageSelectScreen.java - Language selection
- [ ] MainMenuScreen.java - Main menu
- [ ] PauseScreen.java - Pause screen
- [ ] PlaytestStartResolver.java - Playtest resolver
- [ ] SettingsScreen.java - Settings screen
- [ ] WorldSelectScreen.java - World selection

### Util Module
- [ ] FixedTimeStepRunner.java - Fixed timestep management

### Main Class
- [ ] RainbowGame.java - Main application class

## Conversion Priority
1. High Priority: MapEditorScreen.java (complex UI with special focus)
2. Medium Priority: Core systems (Assets, Input, Game main class)
3. Low Priority: Individual gameplay components

## Conversion Strategy
1. Create U++ equivalents for each Java class
2. Replace libGDX dependencies with U++ equivalents
3. Implement U++ drawing using Ctrl::Paint(Draw&) override
4. Implement U++ input using Ctrl::Key(...) override
5. Implement U++ sound using uppsrc/Sound

## Progress Tracking
- [ ] Create individual U++ header and implementation files
- [ ] Implement basic functionality
- [ ] Replace libGDX dependencies
- [ ] Test individual components
- [ ] Integrate with main application

## Dependencies
- Many files depend on libGDX framework
- Some files depend on each other
- Need to identify and handle dependencies during conversion