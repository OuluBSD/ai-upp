# Task: Implement Python Console UI

## Goal
Create a terminal-like console window for Python interaction.

## Implementation in PythonConsole.h

```cpp
#ifndef _ScriptIDE_PythonConsole_h_
#define _ScriptIDE_PythonConsole_h_

namespace Upp {

class PythonConsole : public ParentCtrl {
public:
    typedef PythonConsole CLASSNAME;
    PythonConsole();

    void Write(const String& s);
    void WriteError(const String& s);
    void Clear();

private:
    LineEdit output;
    EditField input;
    
    void OnInput();
};

}

#endif
```

## Implementation in PythonConsole.cpp

```cpp
#include "ScriptIDE.h"

namespace Upp {

PythonConsole::PythonConsole()
{
	output.ReadOnly();
	output.SetFont(Courier(14));
	
	input.SetFont(Courier(14));
	input.WhenAction = [=] { OnInput(); };
	
	Add(output.VSizePos(0, 30));
	Add(input.BottomPos(0, 30).HSizePos());
}

void PythonConsole::Write(const String& s)
{
	output.Append(s.ToWString());
	output.ScrollEnd();
}

void PythonConsole::WriteError(const String& s)
{
	// TODO: Colorized error output
	Write(s);
}

void PythonConsole::Clear()
{
	output.Clear();
}

void PythonConsole::OnInput()
{
	String cmd = input.GetText();
	if(cmd.IsEmpty()) return;
	
	Write(">>> " + cmd + "
");
	input.Clear();
	
	// TODO: Send to ByteVM
}

}
```

## Integration in PythonIDE

Update `PythonIDE::InitRightPanels` to use the real `PythonConsole`.

## Files Modified
- `uppsrc/ScriptIDE/PythonConsole.h`
- `uppsrc/ScriptIDE/PythonConsole.cpp`
- `uppsrc/ScriptIDE/PythonIDE.h`
- `uppsrc/ScriptIDE/PythonIDE.cpp`

## Success Criteria
- Console panel shows output area and input field
- Typing in input field and pressing Enter adds text to output
- Input field clears after Enter
