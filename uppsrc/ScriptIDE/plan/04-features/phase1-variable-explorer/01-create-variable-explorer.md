# Task: Create Variable Explorer ArrayCtrl

## Goal
Build Variable Explorer showing stack variables with type-based coloring.

## Create VariableExplorer class

File: `uppsrc/ScriptIDE/VariableExplorer.h`
```cpp
#ifndef _ScriptIDE_VariableExplorer_h_
#define _ScriptIDE_VariableExplorer_h_

class VariableExplorer : public ArrayCtrl {
public:
    typedef VariableExplorer CLASSNAME;

    VariableExplorer();

    void SetVariables(const VectorMap<PyValue, PyValue>& vars);
    void Clear();

private:
    struct VarDisplay : Display {
        virtual void Paint(Draw& w, const Rect& r, const Value& q,
                          Color ink, Color paper, dword style) const override;
    };

    static Color TypeColor(const String& type);
};

#endif
```

File: `uppsrc/ScriptIDE/VariableExplorer.cpp`
```cpp
#include "ScriptIDE.h"

VariableExplorer::VariableExplorer()
{
    AddIndex("IDX");
    AddColumn("Name", 150).SetDisplay(StdDisplay());
    AddColumn("Type", 100).SetDisplay(StdDisplay());
    AddColumn("Size", 80).SetDisplay(StdRight());
    AddColumn("Value", 300).SetDisplay(Single<VarDisplay>());

    ColumnSort(1);  // Sort by Name (column 1)
    Sorting();
    EvenRowColor();
}

void VariableExplorer::SetVariables(const VectorMap<PyValue, PyValue>& vars)
{
    Clear();

    int idx = 0;
    for(const auto& kv : vars) {
        String name = kv.key.ToString();
        String type = PyTypeName(kv.value.GetType());
        String size;
        String value;

        // Calculate size based on type
        switch(kv.value.GetType()) {
            case PY_LIST:
            case PY_TUPLE:
                size = AsString(kv.value.GetList().GetCount());
                break;
            case PY_DICT:
                size = AsString(kv.value.GetDict().GetCount());
                break;
            case PY_STR:
                size = AsString(kv.value.GetStr().GetCount());
                break;
        }

        // Get string representation
        value = kv.value.ToString();
        if(value.GetCount() > 100)
            value = value.Mid(0, 97) + "...";

        Add(idx++, name, type, size,
            AttrText(value).Paper(TypeColor(type)));
    }
}

Color VariableExplorer::TypeColor(const String& type)
{
    // Hash type name to hue (0-360)
    unsigned hash = 0;
    for(char c : type)
        hash = hash * 31 + c;

    int hue = hash % 360;
    return HsvColorf(hue / 360.0, 0.15, 1.0);  // Light pastel colors
}

void VariableExplorer::VarDisplay::Paint(Draw& w, const Rect& r,
                                         const Value& q, Color ink,
                                         Color paper, dword style) const
{
    // Use AttrText paper color for type-based background
    AttrText at = q;
    w.DrawRect(r, at.paper);
    DrawSmartText(w, r.left + 2, r.top, r.Width() - 4, at.text,
                  StdFont(), at.ink);
}
```

## Integration in PythonIDE

In `PythonIDE.h`:
```cpp
class PythonIDE : public TopWindow {
    // ...
    TabCtrl right_top_tabs;
    VariableExplorer var_explorer;

    void InitRightTopTabs();
    void UpdateVariableExplorer();
};
```

In `PythonIDE.cpp`:
```cpp
void PythonIDE::InitRightTopTabs()
{
    right_top_tabs.Add(var_explorer.SizePos(), "Variable Explorer");
    right_top_tabs.Add(/* Help */, "Help");
    right_top_tabs.Add(/* Plots */, "Plots");
    right_top_tabs.Add(/* Files */, "Files");

    right_top.Add(right_top_tabs.SizePos());
}

void PythonIDE::UpdateVariableExplorer()
{
    if(vm.IsRunning() && vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
        // Get variables from current frame
        const auto& stack = vm.GetCallStack();
        if(stack.GetCount() > 0) {
            const auto& locals = vm.GetLocals(stack[0].frame_index);
            var_explorer.SetVariables(locals);
        }
    }
}
```

## Files Modified
- `uppsrc/ScriptIDE/VariableExplorer.h`
- `uppsrc/ScriptIDE/VariableExplorer.cpp`
- `uppsrc/ScriptIDE/PythonIDE.h`
- `uppsrc/ScriptIDE/PythonIDE.cpp`

## Testing
1. Run Python code with debugger
2. Pause at breakpoint
3. Verify Variable Explorer shows:
   - Variable names
   - Types with color-coded backgrounds
   - Sizes (for collections)
   - Values (truncated if needed)
4. Verify sorting by name works

## Success Criteria
- Variables displayed in ArrayCtrl
- Type-based background colors (hash->hue)
- Sorted by Name column
- Values show appropriate representations
- Updates when debugger pauses
