#ifndef CMD_COMMAND_H
#define CMD_COMMAND_H

#include <Core/Core.h>

namespace Upp {

struct Argument : Moveable<Argument> {
    String name;
    String type;            // "string", "int", "bool", "enum", "path"
    Vector<String> enum_values; // For enum types
    bool required;
    String default_value;
    String description;

    Argument() : required(false) {}
    // Copy constructor using assignment operator
    Argument(const Argument& s) : Moveable<Argument>() {
        *this = s;
    }
    void operator=(const Argument& s);

    void Jsonize(JsonIO& json) {
        json("name", name)("type", type)("enum_values", enum_values)
           ("required", required)("default_value", default_value)("description", description);
    }

    typedef Argument CLASSNAME;
};

struct Command : Moveable<Command> {
    String name;
    String category;
    String description;
    String long_description;
    Vector<Argument> inputs;
    struct Output {
        String kind;        // "status-only", "list", "object", "stream"
        Vector<String> fields; // For structured outputs

        Output() {}

        void Jsonize(JsonIO& json) {
            json("kind", kind)("fields", fields);
        }

        typedef Output CLASSNAME;
    } output;
    struct SideEffects {
        bool modifies_files;
        bool modifies_project;
        bool requires_open_project;
        bool requires_open_file;

        SideEffects() : modifies_files(false), modifies_project(false),
                       requires_open_project(false), requires_open_file(false) {}

        void Jsonize(JsonIO& json) {
            json("modifies_files", modifies_files)("modifies_project", modifies_project)
               ("requires_open_project", requires_open_project)("requires_open_file", requires_open_file);
        }

        typedef SideEffects CLASSNAME;
    } side_effects;
    String context_notes;

    Command() {
        side_effects.modifies_files = false;
        side_effects.modifies_project = false;
        side_effects.requires_open_project = false;
        side_effects.requires_open_file = false;
    }
    // Copy constructor using assignment operator
    Command(const Command& s) : Moveable<Command>() {
        *this = s;
    }
    void operator=(const Command& s);

    void Jsonize(JsonIO& json) {
        json("name", name)("category", category)("description", description)
           ("long_description", long_description)("inputs", inputs)
           ("output", output)("side_effects", side_effects)("context_notes", context_notes);
    }

    typedef Command CLASSNAME;
};


struct InvocationResult : Moveable<InvocationResult> {
    int status_code;    // 0 = success
    String message;     // Result message or error details
    Value payload;      // Optional structured data for machine consumption

    InvocationResult() : status_code(0) {}
    InvocationResult(int status_code, String message);

    void Jsonize(JsonIO& json) {
        json("status_code", status_code)("message", message)("payload", payload);
    }

    typedef InvocationResult CLASSNAME;
};

}

#endif