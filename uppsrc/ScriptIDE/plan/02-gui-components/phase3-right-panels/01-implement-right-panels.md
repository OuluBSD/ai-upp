# Task: Implement Right Side Panels (Top/Bottom)

## Goal
Implement the two TabCtrls on the right side with bottom-aligned tabs.

## Implementation in PythonIDE.h

```cpp
class PythonIDE : public DockWindow {
    // ...
    TabCtrl right_top_tabs;
    TabCtrl right_bottom_tabs;

    ParentCtrl help_panel;
    ParentCtrl plots_panel;
    ParentCtrl files_panel;
    
    // var_explorer is already defined
    // python_console is to be defined later

    void InitRightPanels();
};
```

## Implementation in PythonIDE.cpp

```cpp
void PythonIDE::InitRightPanels()
{
    // Configure Top Tabs
    right_top_tabs.TabsAtBottom();
    right_top_tabs.Add(var_explorer.SizePos(), "Variable Explorer");
    right_top_tabs.Add(help_panel.SizePos(), "Help");
    right_top_tabs.Add(plots_panel.SizePos(), "Plots");
    right_top_tabs.Add(files_panel.SizePos(), "Files");
    
    right_top.Add(right_top_tabs.SizePos());

    // Configure Bottom Tabs
    right_bottom_tabs.TabsAtBottom();
    // right_bottom_tabs.Add(python_console.SizePos(), "IPython Console");
    right_bottom_tabs.Add(new ParentCtrl, "History"); // Placeholder
    
    right_bottom.Add(right_bottom_tabs.SizePos());
}
```

## Files Modified
- `uppsrc/ScriptIDE/PythonIDE.h`
- `uppsrc/ScriptIDE/PythonIDE.cpp`

## Success Criteria
- Two tabbed panels visible on the right
- Tabs are at the bottom of each panel
- Can switch between tabs
- Variable Explorer is the first tab in the top panel
