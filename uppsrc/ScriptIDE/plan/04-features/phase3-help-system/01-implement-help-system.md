# Task: Implement Help System

## Goal
Implement a simple help viewer using `RichTextCtrl` to display Python documentation.

## Implementation Details

We'll use `RichTextCtrl` in the `help_panel` of the right top tabs.

### Changes in PythonIDE.h

```cpp
class PythonIDE : public DockWindow {
    // ...
    RichTextCtrl help_viewer;
    
    void ShowHelp(const String& topic);
};
```

### Changes in PythonIDE.cpp

```cpp
void PythonIDE::InitRightPanels()
{
    // ...
    right_top_tabs.Add(help_viewer.SizePos(), "Help");
    // ...
}

void PythonIDE::ShowHelp(const String& topic)
{
    // For now, just a placeholder. 
    // In a real IDE, this would fetch from ByteVM or a documentation database.
    String qtf;
    qtf << "[_^https://docs.python.org/3/search.html?q=" << topic << "^ Search Python Docs for: " << topic << "]";
    help_viewer.SetQTF(qtf);
    right_top_tabs.Set(1); // Switch to Help tab
}
```

## Success Criteria
- Help tab shows a RichText control
- Can display basic QTF content
- Can be triggered (even if just by a stub for now)
