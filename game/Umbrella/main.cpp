#include "Umbrella.h"
#include "CommandLineArguments.h"
#include "MapEditor.h"
#include "MainMenuScreen.h"
#include "GameScreen.h"

#include <CtrlLib/CtrlLib.h>
#include <Draw/Draw.h>
#include <Core/Core.h>
#include <Sound/Sound.h>

using namespace Upp;

// Forward declaration
bool RunGameScreenTests();

GUI_APP_MAIN
{
    StdLogSetup(LOG_COUT|LOG_FILE);

    // Parse command line arguments
    UmbrellaArgs cmdArgs;
    cmdArgs.Parse(CommandLine());

    if(cmdArgs.testMode) {
        bool testsPassed = RunGameScreenTests();
        SetExitCode(testsPassed ? 0 : 1);
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