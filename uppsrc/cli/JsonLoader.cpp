#include "JsonLoader.h"
#include <plugin/json/json.h>

namespace Upp {

Vector<Command> JsonLoader::LoadCommands(const String& filepath) {
    Vector<Command> commands;
    
    // Load the JSON file
    String jsonContent = LoadFile(filepath);
    if (jsonContent.IsEmpty()) {
        LOG("Failed to load JSON file: " << filepath);
        return commands;
    }
    
    // Parse the JSON
    Value json = ParseJSON(jsonContent);
    if (!json || !json("commands").Is<ValueArray>()) {
        LOG("Invalid JSON format: missing 'commands' array");
        return commands;
    }
    
    Value commandsArray = json("commands");
    
    // Iterate through the commands
    for(int i = 0; i < commandsArray.Get<ValueArray>().GetCount(); i++) {
        Value cmdValue = commandsArray[i];
        
        Command cmd;
        cmd.name = cmdValue("name");
        cmd.category = cmdValue("category");
        cmd.description = cmdValue("description");
        cmd.long_description = cmdValue("long_description");
        
        // Parse inputs (arguments)
        Value inputs = cmdValue("inputs");
        if(inputs.Is<ValueArray>()) {
            for(int j = 0; j < inputs.Get<ValueArray>().GetCount(); j++) {
                Value argValue = inputs[j];
                Argument arg = ParseArgument(argValue);
                cmd.inputs.Add(arg);
            }
        }
        
        // Parse outputs
        Value outputs = cmdValue("outputs");
        if(outputs) {
            cmd.output.kind = outputs("kind");
            
            Value fields = outputs("fields");
            if(fields.Is<ValueArray>()) {
                for(int k = 0; k < fields.Get<ValueArray>().GetCount(); k++) {
                    cmd.output.fields.Add(fields[k]);
                }
            }
        }
        
        // Parse side effects
        Value sideEffects = cmdValue("side_effects");
        if(sideEffects) {
            cmd.side_effects.modifies_files = (bool)sideEffects("modifies_files");
            cmd.side_effects.modifies_project = (bool)sideEffects("modifies_project");
            cmd.side_effects.requires_open_project = (bool)sideEffects("requires_open_project");
            cmd.side_effects.requires_open_file = (bool)sideEffects("requires_open_file");
        }
        
        cmd.context_notes = cmdValue("context_notes");
        
        commands.Add(cmd);
    }
    
    return commands;
}

Argument JsonLoader::ParseArgument(Value argValue) {
    Argument arg;
    arg.name = argValue("name");
    arg.type = argValue("type");
    arg.required = (bool)argValue("required");
    arg.default_value = argValue("default");
    arg.description = argValue("description");
    
    ParseArgumentEnumValues(arg, argValue);
    
    return arg;
}

void JsonLoader::ParseArgumentEnumValues(Argument& arg, Value argValue) {
    Value enumValues = argValue("enum_values");
    if(enumValues.Is<ValueArray>()) {
        for(int i = 0; i < enumValues.Get<ValueArray>().GetCount(); i++) {
            arg.enum_values.Add(enumValues[i]);
        }
    }
}

}