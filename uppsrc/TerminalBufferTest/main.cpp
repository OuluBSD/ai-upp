#include <Core/Core.h>
#include <Maestro/TerminalBuffer.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	
	TerminalBuffer buffer;
	buffer.RunTest();
}
