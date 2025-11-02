#include "WorldEnemyRoster.h"
#include <Core/Core.h>

WorldEnemyRoster::WorldEnemyRoster(WorldType worldType)
    : worldType(worldType) {
    InitializeWorldEnemies();
}

void WorldEnemyRoster::InitializeWorldEnemies() {
    switch (worldType) {
        case WorldType::MUSIC_STAR:
            // Music Star enemies (instruments and music-themed)
            // Assuming these EnemyData objects exist and are defined elsewhere
            // enemyTypes.Add("Casta-kun", One<EnemyData>(new EnemyData(/* parameters for CASTA_KUN */)));
            // enemyTypes.Add("Pet-kun", One<EnemyData>(new EnemyData(/* parameters for PET_KUN */)));
            // enemyTypes.Add("Pianon", One<EnemyData>(new EnemyData(/* parameters for PIANON */)));
            defaultEnemies.Add("Casta-kun");
            defaultEnemies.Add("Pet-kun");
            bossEnemies.Add("Pianon");
            break;
            
        case WorldType::WOODS_STAR:
            // Woods Star enemies (woodland creatures, plants)
            // enemyTypes.Add("Vio-kun", One<EnemyData>(new EnemyData(/* parameters for VIO_KUN */)));
            // enemyTypes.Add("Apple-Head", One<EnemyData>(new EnemyData(/* parameters for APPLE_HEAD */)));
            defaultEnemies.Add("Vio-kun");
            defaultEnemies.Add("Apple-Head");
            bossEnemies.Add("Vio-kun");
            break;
            
        // For other worlds, we'll add them as we implement more enemy types
        default:
            // For now, add just basic enemies to all worlds
            // enemyTypes.Add("Casta-kun", One<EnemyData>(new EnemyData(/* parameters for CASTA_KUN */)));
            defaultEnemies.Add("Casta-kun");
            break;
    }
}

const EnemyData* WorldEnemyRoster::GetEnemyData(const String& enemyTypeName) const {
    int index = enemyTypes.Find(enemyTypeName);
    if (index >= 0) {
        return enemyTypes[index];
    }
    return nullptr;
}

bool WorldEnemyRoster::ContainsEnemyType(const String& enemyTypeName) const {
    return enemyTypes.Find(enemyTypeName) >= 0;
}

const Vector<String> WorldEnemyRoster::GetEnemyTypeNames() const {
    Vector<String> names;
    for (int i = 0; i < enemyTypes.GetCount(); i++) {
        names.Add(enemyTypes.GetKey(i));
    }
    return names;
}