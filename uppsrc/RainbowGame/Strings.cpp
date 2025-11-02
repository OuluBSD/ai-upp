#include "Strings.h"

Strings::Strings() : languageCode("en") {
    // Initialize with default English strings
    LoadStrings("en");
}

Strings::~Strings() {
    // Cleanup
}

String Strings::Get(const String& key) const {
    int i = strings.Find(key);
    if (i >= 0) {
        return strings.Get(key);
    }
    return key;  // Return the key if no translation is found
}

String Strings::GetWithParams(const String& key, const Vector<Value>& params) const {
    String result = Get(key);
    
    // Simple parameter replacement (e.g., {0}, {1}, etc.)
    for (int i = 0; i < params.GetCount(); ++i) {
        String paramStr = AsString(params[i]);
        result.Replace("{%d}" % i, paramStr);
    }
    
    return result;
}

void Strings::SetLanguage(const String& languageCode) {
    if (this->languageCode != languageCode) {
        this->languageCode = languageCode;
        LoadStrings(languageCode);
    }
}

String Strings::GetLanguage() const {
    return languageCode;
}

void Strings::LoadStrings(const String& languageCode) {
    // Clear existing strings
    strings.Clear();
    
    // Load strings based on language code
    if (languageCode == "en") {
        // English strings
        strings.Add("game_title", "Rainbow Game");
        strings.Add("start_game", "Start Game");
        strings.Add("settings", "Settings");
        strings.Add("quit", "Quit");
        strings.Add("back", "Back");
        strings.Add("player_lives", "Lives: {0}");
        strings.Add("score", "Score: {0}");
        // Add more English strings as needed
    } else if (languageCode == "fi") {
        // Finnish strings
        strings.Add("game_title", "Sateenkaari Peli");
        strings.Add("start_game", "Aloita Peli");
        strings.Add("settings", "Asetukset");
        strings.Add("quit", "Lopeta");
        strings.Add("back", "Takaisin");
        strings.Add("player_lives", "Elämät: {0}");
        strings.Add("score", "Pisteet: {0}");
        // Add more Finnish strings as needed
    }
    
    // In a real implementation, you would load from resource files
}