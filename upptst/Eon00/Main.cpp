#include "Eon00.h"

using namespace Upp;

namespace {

struct EngineGuard {
	Engine* eng = nullptr;
	~EngineGuard() {
		if (eng)
			Engine::Uninstall(true, eng);
	}
};

void ConfigureEngine(Engine& eng, void (*runner)(Engine&, int), int method) {
	eng.WhenEnterUpdate.Clear();
	eng.WhenLeaveUpdate.Clear();
	eng.WhenEnterSystemUpdate.Clear();
	eng.WhenLeaveSystemUpdate.Clear();
	eng.WhenGuiProgram.Clear();
	eng.WhenUserProgram.Clear();
	eng.WhenInitialize.Clear();
	eng.WhenPreFirstUpdate.Clear();
	eng.WhenPostInitialize.Clear();
	eng.WhenBoot.Clear();
	eng.WhenUserInitialize.Clear();
	
	eng.WhenInitialize << callback(MachineEcsInit);
	eng.WhenPreFirstUpdate << callback(DefaultStartup);
	eng.WhenBoot << callback(DefaultSerialInitializerInternalEon);
	eng.WhenUserInitialize << callback1(runner, method);
}

void RunScenario(void (*runner)(Engine&, int), int method, const char* label) {
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;
	
	ConfigureEngine(eng, runner, method);
	
	VectorMap<String, Value> args;
	args.Add("MACHINE_TIME_LIMIT", 3);
	
	if (!eng.Start("Shell", String(), &args))
		throw Exc(String().Cat() << label << ": engine failed to start");
	
	eng.MainLoop();
	
	Engine::Uninstall(true, guard.eng);
	guard.eng = nullptr;
}

} // namespace

CONSOLE_APP_MAIN {
	try {
		RunScenario(Run00aAudioGen, 0, "Run00aAudioGen");
		RunScenario(Run00bAudioGen, 0, "Run00bAudioGen");
		RunScenario(Run00cAudioGen, 0, "Run00cAudioGen");
	}
	catch (Exc e) {
		Cout() << "error: " << e << '\n';
		throw;
	}
}
