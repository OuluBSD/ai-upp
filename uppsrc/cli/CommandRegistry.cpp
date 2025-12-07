#include "CommandRegistry.h"

namespace Upp {

CommandRegistry::CommandRegistry() {
    // Default constructor - initialize empty registry
}

CommandRegistry::CommandRegistry(const String& metadataPath) {
    LoadCommands(metadataPath);
}

void CommandRegistry::LoadCommands(const String& metadataPath) {
    commands = JsonLoader::LoadCommands(metadataPath);
    
    // Build the name to index mapping for fast lookup
    nameToIndex.Clear();
    for(int i = 0; i < commands.GetCount(); i++) {
        nameToIndex.GetAdd(commands[i].name, i);
    }
}

const Command* CommandRegistry::FindByName(const String& name) const {
    int index = nameToIndex.Find(name);
    if(index >= 0) {
        return &commands[nameToIndex.Get(index)];
    }
    return nullptr;
}

Vector<Command> CommandRegistry::ListCommands() const {
    return commands;
}

Vector<Command> CommandRegistry::ListByCategory(const String& category) const {
    Vector<Command> result;
    for(int i = 0; i < commands.GetCount(); i++) {
        if(commands[i].category == category) {
            result.Add(commands[i]);
        }
    }
    return result;
}

}