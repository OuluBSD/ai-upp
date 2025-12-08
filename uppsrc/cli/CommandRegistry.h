#ifndef CMD_COMMANDREGISTRY_H
#define CMD_COMMANDREGISTRY_H

#include <Core/Core.h>
#include "Command.h"

namespace Upp {

class CommandRegistry {
public:
    CommandRegistry();
    CommandRegistry(const String& metadataPath);

    void LoadCommands(const String& metadataPath);

    const Command* FindByName(const String& name) const;
    const Vector<Command>& GetAll() const;
    Vector<Command> ListCommands() const;
    Vector<Command> ListByCategory(const String& category) const;

private:
    Vector<Command> commands;
    VectorMap<String, int> nameToIndex;  // For fast lookup
};

}

#endif