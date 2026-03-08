# Task: Implement File Tree Sidebar

## Goal
Implement the file navigation tree in the left dockable panel.

## Implementation in FileTree.h

```cpp
#ifndef _ScriptIDE_FileTree_h_
#define _ScriptIDE_FileTree_h_

namespace Upp {

class FileTree : public ParentCtrl {
public:
    typedef FileTree CLASSNAME;
    FileTree();

    void SetRoot(const String& path);
    void Refresh();

private:
    TreeCtrl tree;
    String root_path;

    void OnOpen(int id);
};

}

#endif
```

## Implementation in FileTree.cpp

```cpp
#include "ScriptIDE.h"

namespace Upp {

FileTree::FileTree()
{
    Add(tree.SizePos());
    tree.WhenOpen = [=](int id) { OnOpen(id); };
}

void FileTree::SetRoot(const String& path)
{
    root_path = path;
    Refresh();
}

void FileTree::Refresh()
{
    tree.Clear();
    if(root_path.IsEmpty()) return;
    
    // TODO: Recursive file population
    tree.SetRoot(CtrlImg::Dir(), root_path);
}

void FileTree::OnOpen(int id)
{
    // TODO: Open file in editor
}

}
```

## Implementation in PythonIDE

Update `PythonIDE::DockInit` to populate the tree.

## Files Modified
- `uppsrc/ScriptIDE/FileTree.h`
- `uppsrc/ScriptIDE/FileTree.cpp`
- `uppsrc/ScriptIDE/PythonIDE.cpp`

## Success Criteria
- File tree shows current project root
- Can expand folders (even if stubs for now)
- Clicking file triggers open event (even if stubbed)
