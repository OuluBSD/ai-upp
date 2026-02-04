#ifndef _LevelLoader_Console_h_
#define _LevelLoader_Console_h_

#include <Core/Core.h>

using namespace Upp;

// Basic LevelData structure
struct LevelData {
    String name;
    int width;
    int height;
    Vector<Vector<int>> tiles;  // 2D array of tile IDs
    Vector<Point> spawnPoints;  // Player/enemy spawn points
    Vector<String> entities;    // Entities in the level
    
    LevelData() : width(0), height(0) {}
};

// Level Loader Class - Basic Implementation
class LevelLoader {
private:
    // Use a simpler approach with just basic types that U++ handles well
    Vector<int> tileIds;
    Vector<String> tileNames;
    String currentLevelPath;
    
public:
    LevelLoader();
    ~LevelLoader();
    
    // Load a level from file
    bool LoadLevel(const String& filePath, LevelData& levelData);
    
    // Save a level to file
    bool SaveLevel(const String& filePath, const LevelData& levelData);
    
    // Load tileset
    bool LoadTileSet(const String& tileSetPath);
    
    // Get tile info by ID
    String GetTileName(int tileId) const;
    bool IsValidTileId(int tileId) const;
    
    // Utility functions
    bool IsValidPosition(const LevelData& level, int x, int y) const;
    int GetTileAt(const LevelData& level, int x, int y) const;
    bool SetTileAt(LevelData& level, int x, int y, int tileId);
    
    // Getters
    const Vector<int>& GetTileIds() const { return tileIds; }
    const Vector<String>& GetTileNames() const { return tileNames; }
    const String& GetCurrentLevelPath() const { return currentLevelPath; }
    
    // Initialize
    bool Initialize();
};

// Global instance for easy access
LevelLoader& GetLevelLoader();

#endif