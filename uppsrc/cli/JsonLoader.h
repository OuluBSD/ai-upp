#ifndef CMD_JSONLOADER_H
#define CMD_JSONLOADER_H

#include "cli.h"

namespace Upp {

class JsonLoader {
public:
    static Vector<Command> LoadCommands(const String& filepath);

private:
    static Argument ParseArgument(const Value& argValue);
    static void ParseArgumentEnumValues(Argument& arg, const Value& argValue);
};

}

#endif