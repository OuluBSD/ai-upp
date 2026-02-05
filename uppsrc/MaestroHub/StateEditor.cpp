#include <CodeEditor/CodeEditor.h>
#include "StateEditor.h"

namespace Upp {

struct PumlSyntax : EditorSyntax {
	virtual void Highlight(const wchar *s, const wchar *end, HighlightOutput& hls, CodeEditor *, int, int64) override {
		const HlStyle& normal = hl_style[INK_NORMAL];
		const HlStyle& comment = hl_style[INK_COMMENT];
		const HlStyle& keyword = hl_style[INK_KEYWORD];
		const HlStyle& op = hl_style[INK_OPERATOR];
		const HlStyle& str = hl_style[INK_CONST_STRING];

		while(s < end) {
			if(*s == '\'') {
				hls.Put(end - s, comment);
				return;
			}
			if(*s == '"') {
				const wchar *q = s + 1;
				while(q < end && *q != '"') q++;
				if(q < end) q++;
				hls.Put(int(q - s), str);
				s = q;
				continue;
			}
			if(IsAlpha(*s)) {
				const wchar *q = s;
				while(q < end && IsAlNum(*q)) q++;
				String id = WString(s, int(q - s)).ToString();
				if(id == "state" || id == "note" || id == "as" || id == "if" || id == "else" || id == "endif")
					hls.Put(int(q - s), keyword);
				else
					hls.Put(int(q - s), normal);
				s = q;
				continue;
			}
			if(*s == '-' || *s == '>' || *s == '*' || *s == '[' || *s == ']') {
				hls.Put(1, op);
				s++;
				continue;
			}
			hls.Put(1, normal);
			s++;
		}
	}
};

void CreatePumlSyntax(One<EditorSyntax>& e)
{
	e.Create<PumlSyntax>();
}

StateEditor::StateEditor() {
	CtrlLayout(*this);
	
split.Horz(puml_editor, graph_view);
	
toolbar.Set(THISBACK(OnToolbar));
	
puml_editor.WhenAction = [=] {
		SetTimeCallback(500, THISBACK(UpdatePreview), 1);
	};
}

void StateEditor::OnToolbar(Bar& bar) {
	bar.Add("Save", THISBACK(Save));
	bar.Separator();
	bar.Add("New State", THISBACK(NewState));
	bar.Add("New Transition", THISBACK(NewTransition));
}

void StateEditor::Load(const String& maestro_root, const String& id) {
	root = maestro_root;
	current_id = id;
	wfm.Create(root);
	
	String path = AppendFileName(AppendFileName(root, "docs/workflows"), id + ".puml");
	puml_editor.SetData(LoadFile(path));
	UpdatePreview();
}

void StateEditor::UpdatePreview() {
	String puml = puml_editor.GetData();
	Cout() << "Updating PUML Preview...\n";
	// Future: use WorkflowManager to parse and update graph_view
}

void StateEditor::NewState() {
	puml_editor.Insert(puml_editor.GetCursor(), "state NewState {\n}\n");
}

void StateEditor::NewTransition() {
	puml_editor.Insert(puml_editor.GetCursor(), "[*] --> InitialState\n");
}

void StateEditor::Save() {
	String path = AppendFileName(AppendFileName(root, "docs/workflows"), current_id + ".puml");
	if(SaveFile(path, puml_editor.GetData())) {
		PromptOK("Workflow saved.");
	}
}

} // namespace Upp
