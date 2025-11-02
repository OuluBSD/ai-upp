#ifndef RAINBOWGAME_SAVESYSTEM_H
#define RAINBOWGAME_SAVESYSTEM_H

#include <Core/Core.h>

using namespace Upp;

class SaveSystem {
public:
    SaveSystem();
    ~SaveSystem();
    
    bool SaveGame(const String& slotName, const Value& data);
    Value LoadGame(const String& slotName);
    bool DeleteSave(const String& slotName);
    bool HasSave(const String& slotName);
    
    // Utility functions
    Vector<String> GetSaveSlots();
    bool ClearAllSaves();
    
private:
    String GetSavePath(const String& slotName);
    Vector<String> saveSlots;
};

#endif