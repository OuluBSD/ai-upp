# Task: Create Initial Source Files

## Goal
Create minimal implementation files that compile successfully.

## Files to Create

### 1. `uppsrc/ScriptIDE/PythonIDE.cpp`
```cpp
#include "ScriptIDE.h"

namespace Upp {

PythonIDE::PythonIDE()
{
    Title("ScriptIDE - Python IDE");
    Sizeable().Zoomable().CenterScreen();
    SetRect(0, 0, 1400, 900);

    AddFrame(menubar);
    AddFrame(toolbar);
    AddFrame(statusbar);

    // Layout will be initialized in DockInit()
}

void PythonIDE::DockInit()
{
    // Docking initialization happens here (after window opens)
    // TODO: Add docking layout in next phase

    // Load saved layout
    FileIn in(ConfigFile("docking-layout.bin"));
    if(in.IsOpen())
        SerializeWindow(in);
}

void PythonIDE::Close()
{
    // Save layout before closing
    FileOut out(ConfigFile("docking-layout.bin"));
    if(out.IsOpen())
        SerializeWindow(out);

    DockWindow::Close();
}

}
```

### 2. `uppsrc/ScriptIDE/Settings.cpp`
```cpp
#include "ScriptIDE.h"

namespace Upp {

void PythonIDESettings::Serialize(Stream& s)
{
    s % editor_font
      % console_font
      % show_line_numbers
      % show_whitespace
      % work_directory;
}

}
```

### 3. `uppsrc/ScriptIDE/VariableExplorer.cpp`
```cpp
#include "ScriptIDE.h"

namespace Upp {

// TODO: Implement in Track 4 Phase 1

}
```

### 4. `uppsrc/ScriptIDE/PythonConsole.cpp`
```cpp
#include "ScriptIDE.h"

namespace Upp {

// TODO: Implement in Track 3 Phase 1

}
```

### 5. `uppsrc/ScriptIDE/FileTree.cpp`
```cpp
#include "ScriptIDE.h"

namespace Upp {

// TODO: Implement in Track 4 Phase 2

}
```

### 6. `uppsrc/ScriptIDE/Main.cpp`
```cpp
#include "ScriptIDE.h"

using namespace Upp;

GUI_APP_MAIN
{
    PythonIDE ide;
    ide.Run();
}
```

## Files Created
- `uppsrc/ScriptIDE/PythonIDE.cpp`
- `uppsrc/ScriptIDE/Settings.cpp`
- `uppsrc/ScriptIDE/VariableExplorer.cpp`
- `uppsrc/ScriptIDE/PythonConsole.cpp`
- `uppsrc/ScriptIDE/FileTree.cpp`
- `uppsrc/ScriptIDE/Main.cpp`

## Testing
```bash
script/build.py -mc 1 -j 12 ScriptIDE
```

## Success Criteria
- Package builds successfully
- ScriptIDE binary runs and shows empty window
- No linker errors
