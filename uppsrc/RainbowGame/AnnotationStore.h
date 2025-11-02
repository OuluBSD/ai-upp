#ifndef _RainbowGame_AnnotationStore_h_
#define _RainbowGame_AnnotationStore_h_

#include <Core/Core.h>
#include <Json/Json.h>
#include <File/File.h>
#include <Array/Array.h>

using namespace Upp;

class GridAnnotationMask;  // Forward declaration
class EnemySpawnMap;       // Forward declaration
class DropletSpawnMap;     // Forward declaration
class LevelWaterConfig;    // Forward declaration
class EditorComment;       // Forward declaration

// Structure for storing annotation data
struct AnnotationRecord {
    int columns = 0;
    int rows = 0;
    int gridSize = 0;
    Vector<int> walls;
    Vector<int> background;
    Vector<int> fullBlocks;
    Vector<Index<String>> textures;  // Texture IDs
    Vector<Index<int>> spawns;       // Enemy spawn points
    Vector<Value> droplets;          // Droplet spawn data
    Value water;                     // Water configuration
    Vector<Value> comments;          // Editor comments
    int mapOriginCol = 0;
    int mapOriginRow = 0;
    int mapCols = 0;
    int mapRows = 0;

    AnnotationRecord() {}
};

// Structure for annotation snapshot
class AnnotationSnapshot {
private:
    One<GridAnnotationMask> mask;
    One<EnemySpawnMap> enemySpawnMap;
    One<DropletSpawnMap> dropletSpawnMap;
    One<LevelWaterConfig> waterConfig;
    Vector<One<EditorComment>> comments;
    int mapOriginCol;
    int mapOriginRow;
    int mapCols;
    int mapRows;

public:
    AnnotationSnapshot();
    AnnotationSnapshot(GridAnnotationMask* mask, EnemySpawnMap* enemySpawnMap,
                      DropletSpawnMap* dropletSpawnMap, LevelWaterConfig* waterConfig,
                      const Vector<One<EditorComment>>& comments, 
                      int mapOriginCol, int mapOriginRow, int mapCols, int mapRows);
    
    static AnnotationSnapshot Empty();
    
    GridAnnotationMask* GetMask() const { return mask; }
    EnemySpawnMap* GetSpawnMap() const { return enemySpawnMap; }
    DropletSpawnMap* GetDropletSpawnMap() const { return dropletSpawnMap; }
    LevelWaterConfig* GetWaterConfig() const { return waterConfig; }
    const Vector<One<EditorComment>>& GetComments() const { return comments; }
    
    int GetMapOriginCol() const { return mapOriginCol; }
    int GetMapOriginRow() const { return mapOriginRow; }
    int GetMapCols() const { return mapCols; }
    int GetMapRows() const { return mapRows; }
    
    bool HasMapArea() const { return mapCols > 0 && mapRows > 0; }
    bool IsEmpty() const;
};

class AnnotationStore {
private:
    String baseDir;
    Json json;
    JsonReader jsonReader;

public:
    AnnotationStore(const String& baseDir);
    
    AnnotationSnapshot Load(const String& mapId);
    void Save(const String& mapId, GridAnnotationMask* mask, EnemySpawnMap* enemySpawnMap,
              DropletSpawnMap* dropletSpawnMap, LevelWaterConfig* waterConfig,
              const Vector<One<EditorComment>>& comments, int mapOriginCol, int mapOriginRow, 
              int mapCols, int mapRows);
    
private:
    String GetFilePath(const String& mapId);
    AnnotationRecord From(GridAnnotationMask* mask, EnemySpawnMap* enemySpawnMap,
                         DropletSpawnMap* dropletSpawnMap, LevelWaterConfig* waterConfig,
                         const Vector<One<EditorComment>>& commentList, 
                         int mapOriginCol, int mapOriginRow, int mapCols, int mapRows);
    AnnotationSnapshot ToSnapshot(const AnnotationRecord& record);
    Vector<int> ExtractIndexes(const BitSet& bits);
    Vector<One<EditorComment>> BuildComments(const Vector<Value>& comments);
};

#endif