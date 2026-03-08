#include "ScriptIDE.h"

namespace Upp {

void PythonIDESettings::Serialize(Stream& s)
{
    s % editor_font
      % console_font
      % show_line_numbers
      % show_spaces
      % work_directory;
}

SettingsDlg::SettingsDlg()
{
	CtrlLayoutOKCancel(*this, "Settings");
	
	for(int i = 0; i < Font::GetFaceCount(); i++) {
		editor_font.Add(i, Font::GetFaceName(i));
		console_font.Add(i, Font::GetFaceName(i));
	}
}

void SettingsDlg::Set(const PythonIDESettings& s)
{
	editor_font <<= s.editor_font.GetFace();
	editor_font_size <<= s.editor_font.GetHeight();
	console_font <<= s.console_font.GetFace();
	console_font_size <<= s.console_font.GetHeight();
	show_line_numbers <<= s.show_line_numbers;
	show_spaces <<= s.show_spaces;
}

void SettingsDlg::Get(PythonIDESettings& s)
{
	s.editor_font = Font(~editor_font, ~editor_font_size);
	s.console_font = Font(~console_font, ~console_font_size);
	s.show_line_numbers = ~show_line_numbers;
	s.show_spaces = ~show_spaces;
}

}
