#include "ScriptIDE.h"

using namespace Upp;

GUI_APP_MAIN
{
    PythonIDE ide;
    const Vector<String>& args = CommandLine();
    if(args.GetCount() > 0) {
        ide.LoadFile(args[0]);
        // If it's a gamestate, we might want to auto-run it
        if(GetFileExt(args[0]) == ".gamestate") {
            ide.OnRun();
        }
    }
    ide.Run();
}
