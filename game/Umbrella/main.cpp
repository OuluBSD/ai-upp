#include "Umbrella.h"
#include "MapEditor.h"
#include "MainMenuScreen.h"

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
    bool editorMode = false;
    bool testMode = false;
    String levelPath;

    const Vector<String>& args = CommandLine();
    for(int i = 0; i < args.GetCount(); i++) {
        const String& arg = args[i];
        if(arg == "--editor" || arg == "--editor-parastar") {
            editorMode = true;
        }
        else if(arg == "--test") {
            testMode = true;
        }
        // If argument doesn't start with --, treat it as a file path
        else if(!arg.StartsWith("--") && !arg.StartsWith("-")) {
            levelPath = arg;
        }
    }

    if(testMode) {
        bool testsPassed = RunGameScreenTests();
        SetExitCode(testsPassed ? 0 : 1);
        return;
    }

    if(editorMode) {
        if(!levelPath.IsEmpty()) {
            MapEditorApp(levelPath).Run();
        } else {
            MapEditorApp().Run();
        }
    } else {
        MainMenuScreen().Run();
    }
}