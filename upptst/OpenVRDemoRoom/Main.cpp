#include "OpenVRDemoRoom.h"
#include <plugin/png/png.h>

using namespace Upp;

namespace {

CommandLineArguments cmd;

struct EngineGuard {
	Engine* eng = nullptr;
	~EngineGuard() {
		if (eng)
			Engine::Uninstall(true, eng);
	}
};

void CheckScreenshot(Engine& eng) {
	if (eng.GetEonParams().Find("emulatedevice") < 0)
		return;
	
	static int frame_count = 0;
	frame_count++;
	
	if (frame_count == 100) {
		LOG("CheckScreenshot: 100 frames reached, capturing...");
		
		int viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		int w = viewport[2];
		int h = viewport[3];
		
		if (w <= 0 || h <= 0) {
			LOG("CheckScreenshot: Invalid viewport " << w << "x" << h << ", stopping.");
			eng.Stop();
			Exit(1);
			return;
		}
		
		// Read from Front Buffer
		glReadBuffer(GL_FRONT);
		
		ImageBuffer ib(w, h);
		glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, ib.Begin());
		
		// Flip vertically
		ImageBuffer ib_flipped(w, h);
		for(int r=0; r<h; r++) {
			RGBA* src = ib[h - 1 - r];
			RGBA* dst = ib_flipped[r];
			memcpy(dst, src, w * sizeof(RGBA));
			
			// Force alpha to 255 because screen capture shouldn't be transparent
			for(int i=0; i<w; i++) dst[i].a = 255;
		}
		
		String path = "capture.png";
		if (PNGEncoder().SaveFile(path, Image(ib_flipped))) {
			LOG("CheckScreenshot: Saved to " << path);
			_exit(0);
		} else {
			LOG("CheckScreenshot: Failed to save to " << path);
			_exit(1);
		}
	}
}

struct TestCase {
	void (*runner)(Engine&, int);
	const char* label;
};

void ConfigureEngine(Engine& eng, void (*runner)(Engine&, int), int method) {
	eng.ClearCallbacks();
	eng.WhenInitialize << callback(MachineEcsInit);
	eng.WhenPreFirstUpdate << callback(DefaultStartup);
	eng.WhenBoot << callback(DefaultSerialInitializerInternalEon);
	eng.WhenUserInitialize << callback1(runner, method);
}

void RunScenario(void (*runner)(Engine&, int), int method, bool emulate, bool verbose, const char* label) {
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;

	ConfigureEngine(eng, runner, method);
	
	eng.WhenEnterUpdate << [&eng] { CheckScreenshot(eng); };

	ValueMap args;
	args.Add("MACHINE_TIME_LIMIT", 3);
	args.Add("emulate", emulate ? "true" : "false");
	args.Add("verbose", verbose ? "true" : "false");
	if (emulate)
		args.Add("emulatedevice", true);
	if (verbose)
		args.Add("verbose", true);
	else
		args.Add("quiet", true);

	LOG("RunScenario: starting " << label);
	if (!eng.StartLoad("Shell", String(), args))
		throw Exc(String().Cat() << label << ": engine failed to start");

	eng.MainLoop();

	Engine::Uninstall(true, guard.eng);
	guard.eng = nullptr;
}

const TestCase kTests[] = {
	{ Run01OpenvrDemoroom, "Run01OpenvrDemoroom" },
};

constexpr int kTestCount = int(sizeof(kTests) / sizeof(kTests[0]));

void PrintTestCatalog() {
	Cout() << "Tests:\n";
	Cout() << "  -1 runs all tests.\n";
	for(int i = 0; i < kTestCount; i++)
		Cout() << "  " << i << ": " << kTests[i].label << '\n';
}

void RunSingleTest(int index, int method, bool emulate, bool verbose) {
	if (index < 0 || index >= kTestCount)
		throw Exc(String().Cat() << "invalid test index " << index);
	RunScenario(kTests[index].runner, method, emulate, verbose, kTests[index].label);
}

void RunAllTests(int method, bool emulate, bool verbose) {
	for(int i = 0; i < kTestCount; i++)
		RunScenario(kTests[i].runner, method, emulate, verbose, kTests[i].label);
}

} // namespace

CONSOLE_APP_MAIN {
	SetCoutLog();
	LOG("OpenVRDemoRoom starting...");
	cmd.AddPositional("test number", INT_V, -1);
	cmd.AddPositional("method number", INT_V, 0);
	cmd.AddPositional("emulate", BOOL_V, false);
	cmd.AddPositional("verbose", BOOL_V, false);
	cmd.AddArg('h', "Show usage information", false);
	
	if (!cmd.Parse() || cmd.IsArg('h')) {
		cmd.PrintHelp();
		PrintTestCatalog();
		return;
	}

	int test_number = (int)cmd.GetPositional(0);
	int method_number = (int)cmd.GetPositional(1);
	bool emulate = (bool)cmd.GetPositional(2);
	bool verbose = (bool)cmd.GetPositional(3);

	Cout() << "Test: " << test_number << ", Method: " << method_number << ", Emulate: " << emulate << ", Verbose: " << verbose << "\n";

	if (test_number < -1 || test_number >= kTestCount) {
		Cerr() << "Test number out of range: " << test_number << '\n';
		cmd.PrintHelp();
		PrintTestCatalog();
		return;
	}

	try {
		if (test_number == -1)
			RunAllTests(method_number, emulate, verbose);
		else
			RunSingleTest(test_number, method_number, emulate, verbose);
	}
	catch (Exc e) {
		Cout() << "error: " << e << '\n';
		throw;
	}
}
