#include "Umbrella.h"
#include "MapEditor.h"
#include "MainMenuScreen.h"

#include <CtrlLib/CtrlLib.h>
#include <Draw/Draw.h>
#include <Core/Core.h>
#include <Sound/Sound.h>

using namespace Upp;

GUI_APP_MAIN
{
    // Parse command line arguments
    bool editorMode = false;
    String levelPath;

    const Vector<String>& args = CommandLine();
    for(int i = 0; i < args.GetCount(); i++) {
        const String& arg = args[i];
        if(arg == "--editor" || arg == "--editor-parastar") {
            editorMode = true;
        }
        // If argument doesn't start with --, treat it as a file path
        else if(!arg.StartsWith("--") && !arg.StartsWith("-")) {
            levelPath = arg;
        }
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