#include "CommandRegistry.h"
#include "JsonLoader.h"

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
        int commandIndex = nameToIndex[index];  // Get the value at the found index
        if(commandIndex >= 0 && commandIndex < commands.GetCount()) {
            return &commands[commandIndex];
        }
    }
    return nullptr;
}

Vector<Command> CommandRegistry::ListCommands() const {
    Vector<Command> result;
    for(int i = 0; i < commands.GetCount(); i++) {
        result.Add(commands[i]);
    }
    return result;
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

const Vector<Command>& CommandRegistry::GetAll() const {
    return commands;
}

}