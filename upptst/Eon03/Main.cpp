#include "Eon03.h"

using namespace Upp;

namespace {

struct EngineGuard {
	Engine* eng = nullptr;
	~EngineGuard() {
		if (eng)
			Engine::Uninstall(true, eng);
	}
};

struct TestCase {
	void (*runner)(Engine&, int);
	const char* label;
};

void ConfigureEngine(Engine& eng, void (*runner)(Engine&, int), int method) {
	eng.ClearCallbacks();
	eng.WhenInitialize << callback(MachineEcsInit);
	eng.WhenPreFirstUpdate << callback(DefaultStartup);
	eng.WhenBoot << callback(DefaultSerialInitializerInternalEon);
	if (method == 4) {
		eng.WhenUserInitialize << [=](Engine& eng) {
			Ptr<Eon::ScriptLoader> script = eng.FindAdd<Eon::ScriptLoader>();
			if (script) {
				script->SetEagerChainBuild(true);
				runner(eng, method);
				// Process .eon strings first to initialize driver, then Python
				script->DoPostLoad();
				script->DoPostLoadPython();
			}
		};
	} else {
		eng.WhenUserInitialize << callback1(runner, method);
	}
}

void RunScenario(void (*runner)(Engine&, int), int method, const char* label) {
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;

	ConfigureEngine(eng, runner, method);

	ValueMap args;
	args.Add("MACHINE_TIME_LIMIT", 3);

	if (!eng.StartLoad("Shell", String(), args))
		throw Exc(String().Cat() << label << ": engine failed to start");

	eng.MainLoop();

	Engine::Uninstall(true, guard.eng);
	guard.eng = nullptr;
}

const TestCase kTests[] = {
	{ Run03aX11Video, "Run03aX11Video" },
	{ Run03bGlxVideo, "Run03bGlxVideo" },
	{ Run03cAudioFile, "Run03cAudioFile" },
	{ Run03dAudioFile2, "Run03dAudioFile2" },
	{ Run03eX11VideoSw3d, "Run03eX11VideoSw3d" },
	{ Run03fX11VideoOgl, "Run03fX11VideoOgl" },
	{ Run03gX11VideoSw3dLinked, "Run03gX11VideoSw3dLinked" },
	{ Run03hX11VideoOglLinked, "Run03hX11VideoOglLinked" },
	{ Run03iX11VideoSw3dBufferstages, "Run03iX11VideoSw3dBufferstages" },
	{ Run03jX11VideoOglBufferstages, "Run03jX11VideoOglBufferstages" },
	{ Run03kX11VideoSw3dStereo, "Run03kX11VideoSw3dStereo" },
	{ Run03lX11VideoOglStereo, "Run03lX11VideoOglStereo" },
	{ Run03mX11VideoOglPbr, "Run03mX11VideoOglPbr" },
	{ Run03nWin32Video, "Run03nWin32Video" },
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
	RunScenario(kTests[index].runner, method, kTests[index].label);
}

void RunAllTests(int method) {
	for(int i = 0; i < kTestCount; i++)
		RunScenario(kTests[i].runner, method, kTests[i].label);
}

void RunLoaderTest(int test_index) {
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;

	if (test_index < 0 || test_index >= kTestCount)
		throw Exc(String().Cat() << "invalid test index " << test_index);

	const char* label = kTests[test_index].label;

	// Initialize without running
	eng.ClearCallbacks();
	eng.WhenInitialize << callback(MachineEcsInit);
	eng.WhenPreFirstUpdate << callback(DefaultStartup);
	eng.WhenBoot << callback(DefaultSerialInitializerInternalEon);
	eng.WhenUserInitialize << callback1(kTests[test_index].runner, 0);

	ValueMap args;
	args.Add("MACHINE_TIME_LIMIT", 0); // Don't run the machine

	if (!eng.StartLoad("Shell", String(), args))
		throw Exc(String().Cat() << label << ": engine failed to start");

	// Get VFS tree dump (StartLoad already initialized the VFS tree)
	String tree = eng.val.GetRoot().GetTreeString();

	// Load expected output
	String test_name = String(label).Mid(3); // Remove "Run" prefix
	test_name = ToLower(test_name);
	String expected_file = ShareDirFile("eon/tests/" + test_name + "_expected.txt");

	if (!FileExists(expected_file)) {
		Cout() << "Warning: Expected output file not found: " << expected_file << '\n';
		Cout() << "VFS Tree:\n" << tree << '\n';
		Engine::Uninstall(true, guard.eng);
		guard.eng = nullptr;
		return;
	}

	String expected = LoadFile(expected_file);

	// Compare (ignoring pointer addresses in parentheses)
	Vector<String> tree_lines = Split(tree, '\n');
	Vector<String> expected_lines = Split(expected, '\n');

	bool match = true;
	int max_lines = max(tree_lines.GetCount(), expected_lines.GetCount());

	for (int i = 0; i < max_lines; i++) {
		String tree_line = i < tree_lines.GetCount() ? tree_lines[i] : "";
		String exp_line = i < expected_lines.GetCount() ? expected_lines[i] : "";

		// Remove addresses (0x...) for comparison - simple approach
		String tree_normalized = tree_line;
		String exp_normalized = exp_line;

		// Replace hex addresses with placeholder
		int pos;
		while ((pos = tree_normalized.Find("(0x")) >= 0) {
			int end_pos = tree_normalized.Find(")", pos);
			if (end_pos > pos) {
				tree_normalized = tree_normalized.Mid(0, pos) + "(ADDR)" + tree_normalized.Mid(end_pos + 1);
			} else break;
		}
		while ((pos = exp_normalized.Find("(0x")) >= 0) {
			int end_pos = exp_normalized.Find(")", pos);
			if (end_pos > pos) {
				exp_normalized = exp_normalized.Mid(0, pos) + "(ADDR)" + exp_normalized.Mid(end_pos + 1);
			} else break;
		}

		if (tree_normalized != exp_normalized) {
			if (match) {
				Cout() << "VFS tree mismatch at line " << (i + 1) << ":\n";
				match = false;
			}
			Cout() << "  Expected: " << exp_line << '\n';
			Cout() << "  Got:      " << tree_line << '\n';
		}
	}

	if (match) {
		Cout() << "Loader test PASSED for " << label << '\n';
	} else {
		throw Exc(String().Cat() << "Loader test FAILED for " << label);
	}

	Engine::Uninstall(true, guard.eng);
	guard.eng = nullptr;
}

} // namespace

CONSOLE_APP_MAIN {
	SetCoutLog();

	CommandLineArguments cmd;
	cmd.AddPositional("test number", INT_V, -1);
	cmd.AddPositional("method number", INT_V, 0);
	cmd.AddArg('h', "Show usage information", false);
	cmd.AddArg('l', "Run loader VFS tree validation test", false);
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
		if (cmd.IsArg('l')) {
			if (test_number == -1) {
				for (int i = 0; i < kTestCount; i++)
					RunLoaderTest(i);
			} else {
				RunLoaderTest(test_number);
			}
		}
		else if (test_number == -1)
			RunAllTests(method_number);
		else
			RunSingleTest(test_number, method_number);
	}
	catch (Exc e) {
		Cout() << "error: " << e << '\n';
		throw;
	}
}
