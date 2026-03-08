# Task: Implement File Operations (New, Open, Save, Save As)

## Goal
Implement basic file management actions in the IDE.

## Implementation Details

We need to implement the stubs in `PythonIDE.cpp`.

### 1. File Handling State
In `PythonIDE.h`:
```cpp
class PythonIDE : public DockWindow {
    // ...
    struct FileInfo {
        String path;
        bool dirty = false;
    };
    
    // For now, since we have only one editor active, we'll track its file
    FileInfo current_file;
    
    void LoadFile(const String& path);
    void SaveFile(const String& path);
    bool ConfirmSave();
};
```

### 2. Implementation in PythonIDE.cpp

```cpp
void PythonIDE::OnNewFile()
{
	if(!ConfirmSave()) return;
	
	code_editor.Clear();
	current_file.path = "";
	current_file.dirty = false;
	editor_tabs.Clear();
	editor_tabs.AddFile("<untitled>", CtrlImg::File());
}

void PythonIDE::OnOpenFile()
{
	if(!ConfirmSave()) return;
	
	FileSel fs;
	fs.Type("Python files", "*.py");
	if(fs.ExecuteOpen("Open Python File")) {
		LoadFile(fs.Get());
	}
}

void PythonIDE::LoadFile(const String& path)
{
	String content = Upp::LoadFile(path);
	code_editor.Set(content);
	current_file.path = path;
	current_file.dirty = false;
	
	editor_tabs.Clear();
	editor_tabs.AddFile(GetFileName(path), CtrlImg::File());
}

void PythonIDE::OnSaveFile()
{
	if(current_file.path.IsEmpty()) {
		OnSaveFileAs();
	}
	else {
		SaveFile(current_file.path);
	}
}

void PythonIDE::OnSaveFileAs()
{
	FileSel fs;
	fs.Type("Python files", "*.py");
	if(fs.ExecuteSaveAs("Save Python File As")) {
		SaveFile(fs.Get());
	}
}

void PythonIDE::SaveFile(const String& path)
{
	if(Upp::SaveFile(path, code_editor.Get())) {
		current_file.path = path;
		current_file.dirty = false;
		// Update tab title
		editor_tabs.Set(0, GetFileName(path));
	}
	else {
		Exclamation("Failed to save file: " + path);
	}
}

bool PythonIDE::ConfirmSave()
{
	if(!current_file.dirty) return true;
	
	int res = Prompt(CtrlImg::Question(), "Save changes to " + (current_file.path.IsEmpty() ? "untitled" : current_file.path) + "?",
	                 "Save", "Don't Save", "Cancel");
	
	if(res == 1) {
		OnSaveFile();
		return !current_file.dirty;
	}
	if(res == 0) return true;
	return false;
}
```

## Changes in PythonIDE Constructor
Connect `code_editor.WhenAction` to track dirty state.
```cpp
code_editor.WhenAction = [=] { current_file.dirty = true; };
```

## Files Modified
- `uppsrc/ScriptIDE/PythonIDE.h`
- `uppsrc/ScriptIDE/PythonIDE.cpp`

## Success Criteria
- Can create a new empty file
- Can open an existing .py file from disk
- Can save changes to disk
- Can use "Save As" to create a new file
- Prompts to save when dirty before opening/new/closing
