#include "Core/Core.h"
using namespace Upp;
CONSOLE_APP_MAIN {
    String s = LoadFile("/tmp/simple.yaml");
    LOG("Input: " << s);
    Value v = ParseYAML(s);
    LOG("Result: " << v["name"]);
}
