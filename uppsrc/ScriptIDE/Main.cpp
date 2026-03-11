#include "ScriptIDE.h"

using namespace Upp;

GUI_APP_MAIN
{
    const Vector<String>& args = CommandLine();
    if(args.GetCount() > 0) {
        String path = GetFullPath(args[0]);
        if(FileExists(path)) {
            SetCurrentDirectory(GetFileDirectory(path));
        }
    }

    PythonIDE ide;
    
    LoadFromFile(ide, ConfigFile("ide_session.bin"));
    
    if(args.GetCount() > 0) {
        String path = GetFullPath(args[0]);
        if(FileExists(path)) {
            ide.LoadFile(path);
            if(GetFileExt(path) == ".gamestate") {
                ide.OnRun();
            }
        }
    }
    
    ide.Run();
    StoreToFile(ide, ConfigFile("ide_session.bin"));
}
