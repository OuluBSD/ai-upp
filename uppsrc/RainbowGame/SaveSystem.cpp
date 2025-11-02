#include "SaveSystem.h"
#include <File.h>

SaveSystem::SaveSystem() {
    // Initialize the save system
}

SaveSystem::~SaveSystem() {
    // Cleanup
}

bool SaveSystem::SaveGame(const String& slotName, const Value& data) {
    String path = GetSavePath(slotName);
    
    // Create directory if it doesn't exist
    String dir = GetFileDirectory(path);
    if (!DirectoryExists(dir)) {
        if (!ForceDirectory(dir)) {
            LOG("Failed to create save directory: " + dir);
            return false;
        }
    }
    
    // Write the save data to file
    try {
        FileOut out(path);
        if (out.IsOpen()) {
            // For now, we'll save as a simple text format
            // In a real implementation, you might want to use JSON, XML, or binary format
            out <<AsString(data);
            out.Close();
            
            // Add to save slots if it's a new one
            if (saveSlots.Find(slotName) < 0) {
                saveSlots.Add(slotName);
            }
            
            return true;
        }
    } catch (Exception& e) {
        LOG("Error saving game: " << e);
    }
    
    return false;
}

Value SaveSystem::LoadGame(const String& slotName) {
    String path = GetSavePath(slotName);
    
    if (!FileExists(path)) {
        LOG("Save file does not exist: " + path);
        return Value();  // Return null value
    }
    
    try {
        FileIn in(path);
        if (in.IsOpen()) {
            String content = ReadString(in);
            in.Close();
            
            // For now, just return the raw content as a string value
            // In a real implementation, you'd parse the appropriate format
            return content;
        }
    } catch (Exception& e) {
        LOG("Error loading game: " << e);
    }
    
    return Value();  // Return null value
}

bool SaveSystem::DeleteSave(const String& slotName) {
    String path = GetSavePath(slotName);
    
    if (FileExists(path)) {
        bool result = DeleteFile(path);
        if (result) {
            int index = saveSlots.Find(slotName);
            if (index >= 0) {
                saveSlots.Remove(index);
            }
        }
        return result;
    }
    
    return false;  // File doesn't exist
}

bool SaveSystem::HasSave(const String& slotName) {
    String path = GetSavePath(slotName);
    return FileExists(path);
}

Vector<String> SaveSystem::GetSaveSlots() {
    return saveSlots;
}

bool SaveSystem::ClearAllSaves() {
    for (const String& slot : saveSlots) {
        DeleteSave(slot);
    }
    saveSlots.Clear();
    return true;
}

String SaveSystem::GetSavePath(const String& slotName) {
    // In a real implementation, you might want to use user data directories
    // For now, we'll save to a local saves directory
    return AppendFileName("saves", slotName + ".sav");
}