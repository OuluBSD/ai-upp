# Task: Refine File Tree Display

## Goal
Improve the `FileTree` component to show user-friendly paths.
- The root node should show the folder name instead of the full absolute path.
- Ensure only relevant filenames are shown in the tree.

## Strategy

1.  **Modify `FileTree::Refresh`**:
    Instead of passing `root_path` as the text to `tree.SetRoot`, extract the last component of the path (the folder name).

2.  **Handle Root Value**:
    Ensure the `value` of the root node remains the full path so that `Populate` works correctly.

## Implementation Details

### FileTree.cpp
```cpp
void FileTree::Refresh()
{
	tree.Clear();
	if(root_path.IsEmpty()) return;

	String root_label = GetFileName(root_path);
	if(root_label.IsEmpty()) root_label = root_path; // Fallback for root directories

	tree.SetRoot(TreeCtrl::Node(CtrlImg::Dir(), root_label).Set(root_path).CanOpen());
	Populate(0);
}
```

## Files Modified
- `uppsrc/ScriptIDE/FileTree.cpp`

## Success Criteria
- File tree root shows "ScriptIDE" (or whatever the folder name is) instead of "/home/user/Dev/ScriptIDE".
- Sub-items still show correct filenames.
- Double-clicking still opens the correct full path.
