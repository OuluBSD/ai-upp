#include "Core/Core.h"
using namespace Upp;
CONSOLE_APP_MAIN {
    Value v = ParseYAML("name: test");
    LOG("name: " << v["name"]);
}
