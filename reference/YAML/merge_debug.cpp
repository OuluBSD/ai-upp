#include "Core/Core.h"
using namespace Upp;

CONSOLE_APP_MAIN {
    String s = LoadFile("/tmp/test_merge.yaml");
    Value v = ParseYAML(s);
    LOG("production.name: " << v["production"]["name"]);
    LOG("production.timeout: " << v["production"]["timeout"]);
    LOG("defaults.name: " << v["defaults"]["name"]);
}
