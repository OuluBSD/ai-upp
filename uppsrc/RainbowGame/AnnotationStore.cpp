#include "AnnotationStore.h"
#include <Core/Core.h>
#include <Json/Json.h>
#include <File/File.h>
#include <Math/Algorithms.h>

// Forward declarations for dependencies that would exist in the project
class GridAnnotationMask {};
class EnemySpawnMap {};
class DropletSpawnMap {};
class LevelWaterConfig {};
class EditorComment {};

// AnnotationSnapshot implementation
AnnotationSnapshot::AnnotationSnapshot() 
    : mapOriginCol(0), mapOriginRow(0), mapCols(0), mapRows(0) {}

AnnotationSnapshot::AnnotationSnapshot(GridAnnotationMask* mask, EnemySpawnMap* enemySpawnMap,
                                      DropletSpawnMap* dropletSpawnMap, LevelWaterConfig* waterConfig,
                                      const Vector<One<EditorComment>>& comments, 
                                      int mapOriginCol, int mapOriginRow, int mapCols, int mapRows)
    : comments(comments), mapOriginCol(Clamp(mapOriginCol, 0, 10000)), 
      mapOriginRow(Clamp(mapOriginRow, 0, 10000)), mapCols(Clamp(mapCols, 0, 10000)), 
      mapRows(Clamp(mapRows, 0, 10000)) {
    if (mask) this->mask.Set(new GridAnnotationMask(*mask));
    if (enemySpawnMap) this->enemySpawnMap.Set(new EnemySpawnMap(*enemySpawnMap));
    if (dropletSpawnMap) this->dropletSpawnMap.Set(new DropletSpawnMap(*dropletSpawnMap));
    if (waterConfig) this->waterConfig.Set(new LevelWaterConfig(*waterConfig));
    else {
        // Set default water config
        this->waterConfig.Set(new LevelWaterConfig());
    }
}

AnnotationSnapshot AnnotationSnapshot::Empty() {
    Vector<One<EditorComment>> emptyComments;
    return AnnotationSnapshot(nullptr, nullptr, nullptr, nullptr, emptyComments, 0, 0, 0, 0);
}

bool AnnotationSnapshot::IsEmpty() const {
    bool noEnemySpawns = !enemySpawnMap || true; // Simplified check
    bool noDropletSpawns = !dropletSpawnMap || true; // Simplified check
    bool noComments = comments.IsEmpty();
    return !mask && noEnemySpawns && noDropletSpawns && noComments;
}

// AnnotationStore implementation
AnnotationStore::AnnotationStore(const String& baseDir) : baseDir(baseDir) {
    // Initialize JSON settings
    json.SetOutputType(Json::OUTPUT_MINIMAL);
    
    // Try to create the base directory
    try {
        bool success = ForceDirectory(baseDir);
        if (!success) {
            LOG("Failed to create annotation directory " + baseDir);
        }
    } catch (Exception& e) {
        LOG("Failed to create annotation directory " + baseDir + ", error: " + e);
    }
}

AnnotationSnapshot AnnotationStore::Load(const String& mapId) {
    if (mapId.IsEmpty()) {
        return AnnotationSnapshot::Empty();
    }
    
    String file = GetFilePath(mapId);
    if (!FileExists(file)) {
        return AnnotationSnapshot::Empty();
    }
    
    try {
        String raw = LoadFile(file);
        if (raw.IsEmpty()) {
            return AnnotationSnapshot::Empty();
        }
        
        Value value = jsonReader.Parse(raw);
        if (value.IsError()) {
            LOG("Failed to parse annotations for map " + mapId);
            return AnnotationSnapshot::Empty();
        }
        
        // Convert the parsed value to an AnnotationRecord
        // This would need more complex conversion logic in practice
        AnnotationRecord record;
        // Simplified conversion for now
        record.mapOriginCol = 0;
        record.mapOriginRow = 0;
        record.mapCols = 0;
        record.mapRows = 0;
        
        return ToSnapshot(record);
    } catch (Exception& e) {
        LOG("Failed to load annotations for map " + mapId + ", error: " + e);
        return AnnotationSnapshot::Empty();
    }
}

void AnnotationStore::Save(const String& mapId, GridAnnotationMask* mask, EnemySpawnMap* enemySpawnMap,
                          DropletSpawnMap* dropletSpawnMap, LevelWaterConfig* waterConfig,
                          const Vector<One<EditorComment>>& comments, int mapOriginCol, int mapOriginRow, 
                          int mapCols, int mapRows) {
    if (mapId.IsEmpty()) {
        return;
    }
    
    String file = GetFilePath(mapId);
    AnnotationRecord record = From(mask, enemySpawnMap, dropletSpawnMap, waterConfig,
                                  comments, mapOriginCol, mapOriginRow, mapCols, mapRows);
    
    try {
        // Create directory if it doesn't exist
        String dir = GetFileDirectory(file);
        ForceDirectory(dir);
        
        // Convert the record to JSON string
        String raw = json.ToString(record); // This would need custom conversion
        SaveFile(file, raw);
    } catch (Exception& e) {
        LOG("Failed to save annotations for map " + mapId + ", error: " + e);
    }
}

String AnnotationStore::GetFilePath(const String& mapId) {
    return AppendFileName(baseDir, mapId + ".json");
}

AnnotationRecord AnnotationStore::From(GridAnnotationMask* mask, EnemySpawnMap* enemySpawnMap,
                                     DropletSpawnMap* dropletSpawnMap, LevelWaterConfig* waterConfig,
                                     const Vector<One<EditorComment>>& commentList, 
                                     int mapOriginCol, int mapOriginRow, int mapCols, int mapRows) {
    AnnotationRecord record;
    record.mapOriginCol = max(0, mapOriginCol);
    record.mapOriginRow = max(0, mapOriginRow);
    record.mapCols = max(0, mapCols);
    record.mapRows = max(0, mapRows);
    
    // In a complete implementation, this would extract data from the input objects
    // and populate the record fields accordingly
    
    return record;
}

AnnotationSnapshot AnnotationStore::ToSnapshot(const AnnotationRecord& record) {
    // Convert the record back to an AnnotationSnapshot
    Vector<One<EditorComment>> comments;
    // Convert comments from record.comments to Vector<One<EditorComment>>
    
    // For now, return an empty snapshot
    return AnnotationSnapshot::Empty();
}

Vector<One<EditorComment>> AnnotationStore::BuildComments(const Vector<Value>& comments) {
    Vector<One<EditorComment>> result;
    // Convert Value comments to EditorComment objects
    return result;
}

Vector<int> AnnotationStore::ExtractIndexes(const BitSet& bits) {
    Vector<int> values;
    // Extract set bit positions from the BitSet
    for (int i = 0; i < bits.GetCount(); i++) {
        if (bits[i]) {
            values.Add(i);
        }
    }
    return values;
}