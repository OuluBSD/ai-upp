#include "Umbrella.h"
#include "CommandLineArguments.h"
#include "MapEditor.h"
#include "MainMenuScreen.h"
#include "GameScreen.h"

#include <CtrlLib/CtrlLib.h>
#include <Draw/Draw.h>
#include <Core/Core.h>
#include <Sound/Sound.h>
#include <ByteVM/ByteVM.h>
#include <Ctrl/Automation/Automation.h>

using namespace Upp;

// Forward declaration
bool RunGameScreenTests();

// Automation test runner using ByteVM
int RunAutomationTest(const String& scriptPath) {
	// Verify script exists
	if(!FileExists(scriptPath)) {
		LOG("ERROR: Test script not found: " << scriptPath);
		return 1;
	}

	// Load script source
	String source = LoadFile(scriptPath);
	if(source.IsEmpty()) {
		LOG("ERROR: Failed to load test script: " << scriptPath);
		return 1;
	}

	LOG("Running automation test: " << scriptPath);

	// Initialize PyVM
	PyVM vm;

	// Register automation bindings (from Ctrl/Automation package)
	RegisterAutomationBindings(vm);

	try {
		// Tokenize Python source
		Tokenizer tokenizer;
		tokenizer.SkipPythonComments(true);
		if(!tokenizer.Process(source, scriptPath)) {
			LOG("ERROR: Failed to tokenize Python script");
			return 1;
		}
		tokenizer.CombineTokens();

		// Compile to IR
		PyCompiler compiler(tokenizer.GetTokens());
		Vector<PyIR> ir;
		compiler.Compile(ir);

		// Execute
		vm.SetIR(ir);
		vm.Run();

		LOG("Test script completed successfully");
		return 0;
	}
	catch(const Exc& e) {
		LOG("ERROR: Exception during test execution: " << e);
		return 1;
	}
}

GUI_APP_MAIN
{
    StdLogSetup(LOG_COUT|LOG_FILE);

    // Parse command line arguments
    UmbrellaArgs cmdArgs;
    cmdArgs.Parse(CommandLine());

    if(cmdArgs.testMode) {
        if(cmdArgs.testScript.IsEmpty()) {
            // Fall back to C++ tests if no script provided
            bool testsPassed = RunGameScreenTests();
            SetExitCode(testsPassed ? 0 : 1);
        } else {
            // Run Python automation test
            int exitCode = RunAutomationTest(cmdArgs.testScript);
            SetExitCode(exitCode);
        }
        return;
    }

    if(cmdArgs.editorMode) {
        if(!cmdArgs.levelPath.IsEmpty()) {
            MapEditorApp(cmdArgs.levelPath).Run();
        } else {
            MapEditorApp().Run();
        }
    } else if(cmdArgs.newGameMode) {
        // Start specified or first level directly
        String levelToLoad = cmdArgs.GetLevelPath();
        if(FileExists(levelToLoad)) {
            LOG("Starting level: " << levelToLoad);
            GameScreen(levelToLoad).Run();
        } else {
            Exclamation("Level not found: " + levelToLoad);
        }
    } else {
        MainMenuScreen().Run();
    }
}