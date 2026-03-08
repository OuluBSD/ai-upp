# Task: Create Initial Header Files

## Goal
Create the main header files for ScriptIDE package.

## Files to Create

### 1. `uppsrc/ScriptIDE/ScriptIDE.h`
Main package header that includes all other headers:
```cpp
#ifndef _ScriptIDE_ScriptIDE_h_
#define _ScriptIDE_ScriptIDE_h_

#include <CtrlLib/CtrlLib.h>
#include <CodeEditor/CodeEditor.h>
#include <TabBar/TabBar.h>
#include <ByteVM/ByteVM.h>
#include <RichEdit/RichEdit.h>
#include <Docking/Docking.h>

namespace Upp {

#define LAYOUTFILE <ScriptIDE/ScriptIDE.lay>
#include <CtrlCore/lay.h>

#define IMAGECLASS ScriptIDEImg
#define IMAGEFILE  <ScriptIDE/ScriptIDE.iml>
#include <Draw/iml_header.h>

#include "PythonIDE.h"
#include "Settings.h"
#include "VariableExplorer.h"
#include "PythonConsole.h"
#include "FileTree.h"

}

#endif
```

### 2. `uppsrc/ScriptIDE/PythonIDE.h`
Main IDE window class (skeleton):
```cpp
#ifndef _ScriptIDE_PythonIDE_h_
#define _ScriptIDE_PythonIDE_h_

namespace Upp {

class PythonIDE : public DockWindow {
public:
    typedef PythonIDE CLASSNAME;

    PythonIDE();
    virtual void DockInit() override;
    virtual void Close() override;

private:
    // Will be filled in later phases
    MenuBar menubar;
    ToolBar toolbar;
    StatusBar statusbar;
};

}

#endif
```

### 3. `uppsrc/ScriptIDE/Settings.h`
Settings management (skeleton):
```cpp
#ifndef _ScriptIDE_Settings_h_
#define _ScriptIDE_Settings_h_

namespace Upp {

struct PythonIDESettings {
    Font editor_font;
    Font console_font;
    bool show_line_numbers = true;
    bool show_whitespace = false;
    String work_directory;

    void Serialize(Stream& s);
};

}

#endif
```

### 4-7. Other skeleton headers
- `VariableExplorer.h` - Variable inspector ArrayCtrl
- `PythonConsole.h` - IPython-like console
- `FileTree.h` - File navigation tree
(Skeleton classes only)

## Files Created
- `uppsrc/ScriptIDE/ScriptIDE.h`
- `uppsrc/ScriptIDE/PythonIDE.h`
- `uppsrc/ScriptIDE/Settings.h`
- `uppsrc/ScriptIDE/VariableExplorer.h`
- `uppsrc/ScriptIDE/PythonConsole.h`
- `uppsrc/ScriptIDE/FileTree.h`

## Success Criteria
- All headers compile
- No syntax errors
- Headers can be included
