#ifndef CMD_COMMAND_H
#define CMD_COMMAND_H

#include "cli.h"

namespace Upp {

struct Argument {
    String name;
    String type;            // "string", "int", "bool", "enum", "path"
    Vector<String> enum_values; // For enum types
    bool required;
    String default_value;
    String description;

    Argument() : required(false) {}
};

struct Command {
    String name;
    String category;
    String description;
    String long_description;
    Vector<Argument> inputs;
    struct {
        String kind;        // "status-only", "list", "object", "stream"
        Vector<String> fields; // For structured outputs
    } output;
    struct {
        bool modifies_files;
        bool modifies_project;
        bool requires_open_project;
        bool requires_open_file;
    } side_effects;
    String context_notes;

    Command() : side_effects{false, false, false, false} {}
};

struct InvocationResult {
    int status_code;    // 0 = success
    String message;     // Result message or error details
};

}

#endif