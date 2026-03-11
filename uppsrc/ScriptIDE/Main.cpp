#include "ScriptIDE.h"

using namespace Upp;

GUI_APP_MAIN
{
    const Vector<String>& args = CommandLine();
    String path;
    if(args.GetCount() > 0) {
        path = GetFullPath(args[0]);
        if(FileExists(path)) {
            SetCurrentDirectory(GetFileDirectory(path));
        }
    }

    PythonIDE ide;

    if(!path.IsEmpty() && FileExists(path)) {
        ide.LoadFile(path);
        if(GetFileExt(path) == ".gamestate")
            ide.OnRun();
    }

    ide.Run();
}
