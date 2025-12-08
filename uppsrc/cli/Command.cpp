#include "Command.h"

namespace Upp {

void Argument::operator=(const Argument& s) {
    if (this != &s) {
        name = s.name;
        type = s.type;
        // For Vector, clear and add elements individually
        enum_values.Clear();
        for(const String& value : s.enum_values) {
            enum_values.Add(value);
        }
        required = s.required;
        default_value = s.default_value;
        description = s.description;
    }
}

void Command::operator=(const Command& s) {
    if (this != &s) {
        name = s.name;
        category = s.category;
        description = s.description;
        long_description = s.long_description;
        // For Vector, clear and add elements individually
        inputs.Clear();
        for(const Argument& arg : s.inputs) {
            inputs.Add(arg);
        }

        // Field-by-field assignment for nested structures
        output.kind = s.output.kind;
        output.fields.Clear();
        for(const String& field : s.output.fields) {
            output.fields.Add(field);
        }

        side_effects = s.side_effects;
        context_notes = s.context_notes;
    }
}

InvocationResult::InvocationResult(int status_code, String message)
    : status_code(status_code), message(message) {
}

}