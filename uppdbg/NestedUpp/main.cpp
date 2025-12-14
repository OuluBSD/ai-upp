// Error in this file is unintuitive. It hints that the header is not included, but it is. The fix is complex and requires looking for primary header's inclusions.
#include "NestedUpp.h"

CONSOLE_APP_MAIN {
	using namespace Upp;
	A a;
}
