#include "Adventure.h"

GUI_APP_MAIN {
	//MemoryBreakpoint(37975);

	using namespace Adventure;

	ProgramApp a;
	if (a.Init()) {
		a.MarkInitialized();  // Mark initialization complete
		a.Show();  // Show window after successful initialization
		a.Run();
	}

}
