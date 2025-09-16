#include <Shell/Shell.h>


CONSOLE_APP_MAIN {
	using namespace Upp;
	Engine& eng = ShellMainEngine();
	eng.WhenUserInitialize << [](Engine& eng) {
		TODO
	};
	
	ShellMain(true);
}
