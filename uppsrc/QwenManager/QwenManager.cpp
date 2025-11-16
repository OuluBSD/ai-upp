#include "QwenManager.h"


#ifdef flagMAIN
GUI_APP_MAIN {
	using namespace Upp;
	
	// Load QwenManagerState
	QwenManagerState::Global().Load(); // default path
	
	// Initialize QwenManager and Run
	QwenManager m;
	m.Run();
	
	// Store QwenManagerState
	QwenManagerState::Global().Store(); // default path
}
#endif
