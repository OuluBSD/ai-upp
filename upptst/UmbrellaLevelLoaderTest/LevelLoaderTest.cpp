#include "LevelLoaderTest.h"

CONSOLE_APP_MAIN
{
    Cout() << "Testing LevelLoader functionality..." << EOL;

    // Get the global level loader instance
    LevelLoader& loader = GetLevelLoader();

    // Test loading a level
    LevelData level;
    bool loaded = loader.LoadLevel("test_level.dat", level);

    if(loaded) {
        Cout() << "Level loaded successfully!" << EOL;
        Cout() << "Level name: " << level.name << EOL;
        Cout() << "Level size: " << level.width << "x" << level.height << EOL;
        Cout() << "Number of spawn points: " << level.spawnPoints.GetCount() << EOL;
        Cout() << "Number of entities: " << level.entities.GetCount() << EOL;

        // Test tile operations
        if(level.width > 0 && level.height > 0) {
            int tileId = loader.GetTileAt(level, 5, 5);
            Cout() << "Tile at (5,5): " << tileId << EOL;

            bool setSuccess = loader.SetTileAt(level, 5, 5, 99);
            if(setSuccess) {
                Cout() << "Successfully set tile at (5,5) to 99" << EOL;
                int newTileId = loader.GetTileAt(level, 5, 5);
                Cout() << "New tile at (5,5): " << newTileId << EOL;
            }
        }

        // Test tile set
        const Vector<int>& tileIds = loader.GetTileIds();
        const Vector<String>& tileNames = loader.GetTileNames();
        Cout() << "Number of tile types: " << tileIds.GetCount() << EOL;

        // Print first few tile names
        for(int i = 0; i < min(5, tileIds.GetCount()); i++) {
            Cout() << "Tile " << tileIds[i] << ": " << tileNames[i] << EOL;
        }
    } else {
        Cout() << "Failed to load level!" << EOL;
    }

    // Test saving a level
    bool saved = loader.SaveLevel("output_level.dat", level);
    if(saved) {
        Cout() << "Level saved successfully!" << EOL;
    } else {
        Cout() << "Failed to save level!" << EOL;
    }

    Cout() << "LevelLoader test completed." << EOL;
}