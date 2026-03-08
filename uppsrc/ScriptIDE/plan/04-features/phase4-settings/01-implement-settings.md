# Task: Implement Settings Dialog

## Goal
Implement a modal dialog to manage IDE settings (fonts, line numbers, etc.) and persist them.

## Implementation Details

### 1. Update SettingsLayout in ScriptIDE.lay
```
LAYOUT(SettingsLayout, 400, 200)
	ITEM(Label, dv___0, SetLabel(t_("Editor Font")).LeftPosZ(8, 72).TopPosZ(8, 19))
	ITEM(DropFont, editor_font, LeftPosZ(84, 140).TopPosZ(8, 19))
	ITEM(EditInt, editor_font_size, LeftPosZ(228, 40).TopPosZ(8, 19))
	ITEM(Label, dv___3, SetLabel(t_("Console Font")).LeftPosZ(8, 72).TopPosZ(32, 19))
	ITEM(DropFont, console_font, LeftPosZ(84, 140).TopPosZ(32, 19))
	ITEM(EditInt, console_font_size, LeftPosZ(228, 40).TopPosZ(32, 19))
	ITEM(Option, show_line_numbers, SetLabel(t_("Show Line Numbers")).LeftPosZ(8, 140).TopPosZ(60, 16))
	ITEM(Option, show_whitespace, SetLabel(t_("Show Whitespace")).LeftPosZ(8, 140).TopPosZ(80, 16))
	ITEM(Button, ok, SetLabel(t_("OK")).RightPosZ(84, 72).BottomPosZ(8, 24))
	ITEM(Button, cancel, SetLabel(t_("Cancel")).RightPosZ(8, 72).BottomPosZ(8, 24))
END_LAYOUT
```

### 2. Create SettingsDlg class
In `Settings.h`:
```cpp
class SettingsDlg : public WithSettingsLayout<TopWindow> {
public:
    typedef SettingsDlg CLASSNAME;
    SettingsDlg();
    
    void Set(const PythonIDESettings& s);
    void Get(PythonIDESettings& s);
};
```

In `Settings.cpp`:
```cpp
SettingsDlg::SettingsDlg()
{
    CtrlLayoutOKCancel(*this, "Settings");
}

void SettingsDlg::Set(const PythonIDESettings& s)
{
    editor_font = s.editor_font;
    editor_font_size = s.editor_font.GetHeight();
    console_font = s.console_font;
    console_font_size = s.console_font.GetHeight();
    show_line_numbers = s.show_line_numbers;
    show_whitespace = s.show_whitespace;
}

void SettingsDlg::Get(PythonIDESettings& s)
{
    s.editor_font = ~editor_font;
    s.editor_font.Height(~editor_font_size);
    s.console_font = ~console_font;
    s.console_font.Height(~console_font_size);
    s.show_line_numbers = ~show_line_numbers;
    s.show_whitespace = ~show_whitespace;
}
```

### 3. Integration in PythonIDE
Add `settings` member and `OnSettings` method.
Load/Save settings in `PythonIDE` constructor and `Close`.

## Files Modified
- `uppsrc/ScriptIDE/ScriptIDE.lay`
- `uppsrc/ScriptIDE/Settings.h`
- `uppsrc/ScriptIDE/Settings.cpp`
- `uppsrc/ScriptIDE/PythonIDE.h`
- `uppsrc/ScriptIDE/PythonIDE.cpp`

## Success Criteria
- Can open Settings dialog from Tools menu
- Can change fonts and options
- Clicking OK applies settings to editor/console
- Settings are saved to disk and restored on restart
