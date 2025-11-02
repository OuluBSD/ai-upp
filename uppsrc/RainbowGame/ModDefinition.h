#ifndef RAINBOWGAME_MODDEFINITION_H
#define RAINBOWGAME_MODDEFINITION_H

#include <Core/Core.h>

using namespace Upp;

class ModDefinition {
public:
    ModDefinition();
    ModDefinition(const String& id);
    ~ModDefinition();
    
    String GetId() const { return id; }
    void SetId(const String& id) { this->id = id; }
    
    String GetName() const { return name; }
    void SetName(const String& name) { this->name = name; }
    
    String GetDescription() const { return description; }
    void SetDescription(const String& description) { this->description = description; }
    
    String GetVersion() const { return version; }
    void SetVersion(const String& version) { this->version = version; }
    
    bool LoadMod(const String& modId);
    
private:
    String id;
    String name;
    String description;
    String version;
};

#endif