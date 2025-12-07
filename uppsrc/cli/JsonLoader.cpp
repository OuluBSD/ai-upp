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
    if (!json.IsObject() || !json.Get("commands").IsArray()) {
        LOG("Invalid JSON format: missing 'commands' array");
        return commands;
    }
    
    Value commandsArray = json.Get("commands");
    
    // Iterate through the commands
    for(int i = 0; i < commandsArray.GetArray().GetCount(); i++) {
        Value cmdValue = commandsArray[i];
        
        Command cmd;
        cmd.name = cmdValue.Get("name");
        cmd.category = cmdValue.Get("category");
        cmd.description = cmdValue.Get("description");
        cmd.long_description = cmdValue.Get("long_description");
        
        // Parse inputs (arguments)
        Value inputs = cmdValue.Get("inputs");
        if(inputs.IsArray()) {
            for(int j = 0; j < inputs.GetArray().GetCount(); j++) {
                Value argValue = inputs[j];
                Argument arg = ParseArgument(argValue);
                cmd.inputs.Add(arg);
            }
        }
        
        // Parse outputs
        Value outputs = cmdValue.Get("outputs");
        if(outputs.IsObject()) {
            cmd.output.kind = outputs.Get("kind");
            
            Value fields = outputs.Get("fields");
            if(fields.IsArray()) {
                for(int k = 0; k < fields.GetArray().GetCount(); k++) {
                    cmd.output.fields.Add(fields[k]);
                }
            }
        }
        
        // Parse side effects
        Value sideEffects = cmdValue.Get("side_effects");
        if(sideEffects.IsObject()) {
            cmd.side_effects.modifies_files = sideEffects.Get("modifies_files").GetBool();
            cmd.side_effects.modifies_project = sideEffects.Get("modifies_project").GetBool();
            cmd.side_effects.requires_open_project = sideEffects.Get("requires_open_project").GetBool();
            cmd.side_effects.requires_open_file = sideEffects.Get("requires_open_file").GetBool();
        }
        
        cmd.context_notes = cmdValue.Get("context_notes");
        
        commands.Add(cmd);
    }
    
    return commands;
}

Argument JsonLoader::ParseArgument(const Value& argValue) {
    Argument arg;
    arg.name = argValue.Get("name");
    arg.type = argValue.Get("type");
    arg.required = argValue.Get("required").GetBool();
    arg.default_value = argValue.Get("default");
    arg.description = argValue.Get("description");
    
    ParseArgumentEnumValues(arg, argValue);
    
    return arg;
}

void JsonLoader::ParseArgumentEnumValues(Argument& arg, const Value& argValue) {
    Value enumValues = argValue.Get("enum_values");
    if(enumValues.IsArray()) {
        for(int i = 0; i < enumValues.GetArray().GetCount(); i++) {
            arg.enum_values.Add(enumValues[i]);
        }
    }
}

}