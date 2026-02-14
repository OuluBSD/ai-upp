#include "Eon09.h"

#if defined(flagSDL2)
#include <SDL2/SDL.h>
#endif

using namespace Upp;

namespace {

struct TestCase {
	void (*runner)(int);
	const char* label;
	bool needs_sdl;
};

#if defined(flagSDL2)
bool EnsureSdlVideoAvailable(String& reason) {
	static bool checked = false;
	static bool available = false;
	static String last_error;
	if (!checked) {
		checked = true;
		SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
		if (SDL_Init(SDL_INIT_VIDEO) == 0) {
			available = true;
			SDL_Quit();
		}
		else {
			const char* err = SDL_GetError();
			last_error = err ? String(err) : String("unknown SDL error");
			SDL_ClearError();
		}
	}
	if (!available)
		reason = last_error;
	else
		reason.Clear();
	return available;
}
#else
bool EnsureSdlVideoAvailable(String& reason) {
	reason = "SDL2 backend not available in this build";
	return false;
}
#endif

void RunScenario(const TestCase& test, int method) {
	if (test.needs_sdl) {
		String reason;
		if (!EnsureSdlVideoAvailable(reason)) {
			String msg = Format("%s: skipping (SDL video backend unavailable%s)", test.label,
			                    reason.IsEmpty() ? String() : String(": ") + reason);
			Cout() << msg << '\n';
			return;
		}
	}
	(*test.runner)(method);
}

const TestCase kTests[] = {
	{ Run09aScene3DHeadless, "Run09aScene3DHeadless", false },
	{ Run09bScene3DSdlOgl, "Run09bScene3DSdlOgl", true },
};

constexpr int kTestCount = int(sizeof(kTests) / sizeof(kTests[0]));

void PrintTestCatalog() {
	Cout() << "Tests:\n";
	Cout() << "  -1 runs all tests.\n";
	for(int i = 0; i < kTestCount; i++)
		Cout() << "  " << i << ": " << kTests[i].label << '\n';
}

void RunSingleTest(int index, int method) {
	if (index < 0 || index >= kTestCount)
		throw Exc(String().Cat() << "invalid test index " << index);
	RunScenario(kTests[index], method);
}

void RunAllTests(int method) {
	for(int i = 0; i < kTestCount; i++)
		RunScenario(kTests[i], method);
}

} // namespace

CONSOLE_APP_MAIN {
	Cout() << "=== Eon09 starting ===" << EOL;
	SetCoutLog();
	CommandLineArguments cmd;
	cmd.AddPositional("test number", INT_V, -1);
	cmd.AddPositional("method number", INT_V, 0);
	cmd.AddArg('h', "Show usage information", false);
	if (!cmd.Parse() || cmd.IsArg('h')) {
		cmd.PrintHelp();
		PrintTestCatalog();
		return;
	}

	int test_number = (int)cmd.GetPositional(0);
	int method_number = (int)cmd.GetPositional(1);

	if (test_number < -1 || test_number >= kTestCount) {
		Cerr() << "Test number out of range: " << test_number << '\n';
		cmd.PrintHelp();
		PrintTestCatalog();
		return;
	}

	try {
		if (test_number == -1)
			RunAllTests(method_number);
		else
			RunSingleTest(test_number, method_number);
	}
	catch (Exc e) {
		LOG("error: " << e);
		SetExitCode(1);
	}
}
