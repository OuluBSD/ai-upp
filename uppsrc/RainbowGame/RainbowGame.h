#ifndef RAINBOWGAME_RAINBOWGAME_H
#define RAINBOWGAME_RAINBOWGAME_H

#include <Core/Core.h>
#include <Gdx/Gdx.h>
#include "PlatformServices.h"
#include "RainbowGameLaunchOptions.h"
#include "Assets.h"
#include "Settings.h"
#include "SaveSystem.h"
#include "Strings.h"
#include "GameConfig.h"
#include "LevelManifest.h"
#include "ModDefinition.h"
#include "AudioSystem.h"

using namespace Upp;

class LanguageOption {
public:
    LanguageOption(const String& code, const String& displayName) 
        : code(code), displayName(displayName) {}
    
    String GetCode() const { return code; }
    String GetDisplayName() const { return displayName; }

private:
    String code;
    String displayName;
};

class RainbowGame : public Game {
public:
    RainbowGame(PlatformServices* platformServices);
    RainbowGame(PlatformServices* platformServices, RainbowGameLaunchOptions* launchOptions);
    virtual ~RainbowGame();
    
    virtual void Create() override;
    virtual void Render() override;
    virtual void Dispose() override;
    
    // Getters
    Assets* GetAssets() { return assets; }
    Settings* GetSettings() { return settings; }
    SaveSystem* GetSaveSystem() { return saveSystem; }
    Strings* GetStrings() { return strings; }
    GameConfig* GetGameConfig() { return gameConfig; }
    LevelManifest* GetLevelManifest() { return levelManifest; }
    ModDefinition* GetModDefinition() { return modDefinition; }
    PlatformServices* GetPlatformServices() { return platformServices; }
    RainbowGameLaunchOptions* GetLaunchOptions() { return launchOptions; }
    AudioSystem* GetAudioSystem() { return audioSystem; }
    
    void ShowScreen(Screen* screen) { SetScreen(screen); }
    
    // Language-related methods
    Vector<LanguageOption> GetLanguageOptions();
    void OnLanguageSelected(const String& code);
    void ApplyLanguageSelection(const String& code, bool restart);

private:
    static const bool EDITOR_FEATURE_ENABLED = true;
    
    // Game components
    PlatformServices* platformServices;
    RainbowGameLaunchOptions* launchOptions;
    Assets* assets;
    Settings* settings;
    SaveSystem* saveSystem;
    Strings* strings;
    GameConfig* gameConfig;
    LevelManifest* levelManifest;
    ModDefinition* modDefinition;
    AudioSystem* audioSystem;
    
    // Language options
    Vector<LanguageOption> languageOptions;
};

#endif