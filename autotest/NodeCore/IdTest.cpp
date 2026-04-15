#include <Node/Core/Core.h>

using namespace Upp;
using namespace Upp::Node;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	
	ASSERT(IsValidEntityId("node1"));
	ASSERT(IsValidEntityId("NODE-1"));
	ASSERT(IsValidEntityId("node_1"));
	ASSERT(!IsValidEntityId(""));
	ASSERT(!IsValidEntityId(" "));
	ASSERT(!IsValidEntityId("node 1"));
	ASSERT(!IsValidEntityId("node.1"));
	ASSERT(!IsValidEntityId("node@1"));
	
	LOG("IdTest passed.");
}
