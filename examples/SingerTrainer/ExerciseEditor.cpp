#include "ExerciseEditor.h"

namespace Upp {

ExerciseEditor::ExerciseEditor() {
	Add(editor.SizePos());
	
	Add(filename_field.BottomPos(0, 30).LeftPos(0, 200));
	filename_field.SetText("exercises/custom_exercise.py");
	
	Add(save_btn.BottomPos(0, 30).LeftPos(205, 80));
	save_btn.SetLabel("Save");
	save_btn << [=] { Save(); };
	
	Add(run_btn.BottomPos(0, 30).LeftPos(290, 80));
	run_btn.SetLabel("Run");
	run_btn << [=] { RunScript(); };
	
	editor.SetFont(Monospace(12));
	
	// Adjust editor position to not overlap with buttons
	editor.SetRect(0, 0, 100, 100); // placeholder
	editor.HSizePos().VSizePos(0, 35);
}

void ExerciseEditor::Load(const String& path) {
	editor.Set(Upp::LoadFile(path));
	filename_field.SetText(path);
}

void ExerciseEditor::Save() {
	String path = filename_field.GetText();
	Upp::SaveFile(path, editor.Get());
}

void ExerciseEditor::RunScript() {
	if (scripting) {
		scripting->ExecuteScript(editor.Get());
	}
}

}
