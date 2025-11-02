#include "ModDefinition.h"

ModDefinition::ModDefinition() : id("default"), name("Default Mod"), 
                                description("Default game content"), version("1.0.0") {
    // Initialize with default values
}

ModDefinition::ModDefinition(const String& id) : id(id), name(id), 
                                                 description("Mod: " + id), version("1.0.0") {
    // Initialize with provided ID
}

ModDefinition::~ModDefinition() {
    // Cleanup
}

bool ModDefinition::LoadMod(const String& modId) {
    // In a real implementation, you would load mod configuration from files
    // For now, we'll just update the ID and set default values
    id = modId;
    name = modId;
    description = "Mod: " + modId;
    
    return true;
}