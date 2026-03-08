#include <CodeEditor/CodeEditor.h>
#include "MaestroHub.h"

namespace Upp {

struct PumlSyntax : EditorSyntax {
	virtual void Highlight(const wchar *s, const wchar *end, HighlightOutput& hls, CodeEditor *editor, int line, int64 pos) override {
		const HlStyle& normal = hl_style[INK_NORMAL];
		const HlStyle& comment = hl_style[INK_COMMENT];
		const HlStyle& keyword = hl_style[INK_KEYWORD];
		const HlStyle& op = hl_style[INK_OPERATOR];
		const HlStyle& str = hl_style[INK_CONST_STRING];

		while(s < end) {
			if(*s == (wchar)39) { // Single quote for comment
				hls.Put((int)(end - s), comment);
				return;
			}
			if(*s == '"') {
				const wchar *q = s + 1;
				while(q < end && *q != '"') q++;
				if(q < end) q++;
				hls.Put((int)(q - s), str);
				s = q;
				continue;
			}
			if(IsAlpha(*s)) {
				const wchar *q = s;
				while(q < end && IsAlNum(*q)) q++;
				String id = WString(s, (int)(q - s)).ToString();
				if(id == "state" || id == "note" || id == "as" || id == "if" || id == "else" || id == "endif")
					hls.Put((int)(q - s), keyword);
				else
					hls.Put((int)(q - s), normal);
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

static void sCreatePumlSyntax(One<EditorSyntax>& e)
{
	e.Create<PumlSyntax>();
}

StateEditor::StateEditor() {
	CtrlLayout(*this);
	
	static bool reg = false;
	if(!reg) {
		EditorSyntax::Register("puml", sCreatePumlSyntax, "*.puml", "PlantUML");
		reg = true;
	}
	
split.Horz(puml_editor, graph_view);
	
toolbar.Set(THISBACK(OnToolbar));
	
puml_editor.WhenAction = [=] {
		KillTimeCallback(1);
		SetTimeCallback(500, THISBACK(UpdatePreview), 1);
	};
	
puml_editor.Highlight("puml");
}

void StateEditor::OnToolbar(Bar& bar) {
	bar.Add("Save", CtrlImg::save(), THISBACK(Save));
	bar.Separator();
	bar.Add("New State", CtrlImg::plus(), THISBACK(NewState));
	bar.Add("New Transition", CtrlImg::right_arrow(), THISBACK(NewTransition));
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
	GraphLib::Graph& g = graph_view.GetGraph();
	g.Clear();
	
	Index<String> state_ids;
	Vector<String> lines = Split(puml, '\n');
	
	RegExp re_state("state\\\\s+([a-zA-Z0-9_]+)");
	RegExp re_trans("([a-zA-Z0-9_\\\\*\\\\[\\\\]]+)\\\\s+-->\\\\s+([a-zA-Z0-9_\\\\*\\\\[\\\\]]+)");

	for(const String& line : lines) {
		String l = TrimBoth(line);
		if(re_state.Match(l)) {
			state_ids.FindAdd(re_state[0]);
		}
	}
	
	for(const String& line : lines) {
		if(re_trans.Match(line)) {
			String from = re_trans[0];
			String to = re_trans[1];
			if(from != "[*]") state_ids.FindAdd(from);
			if(to != "[*]") state_ids.FindAdd(to);
		}
	}
	
	int x = 50, y = 50;
	for(const String& id : state_ids) {
		GraphLib::Node& n = graph_view.AddNode(id, Point(x, y));
		n.SetLabel(id);
		n.AddPin("in", GraphLib::PinKind::Input);
		n.AddPin("out", GraphLib::PinKind::Output);
		x += 150;
		if(x > 600) { x = 50; y += 120; }
	}
	
	for(const String& line : lines) {
		if(re_trans.Match(line)) {
			String from = re_trans[0];
			String to = re_trans[1];
			
			if(from == "[*]") {
				int idx = g.FindNode(to);
				if(idx >= 0) g.GetNode(idx).SetFill(Color(220, 255, 220));
			}
			else if(to == "[*]") {
				int idx = g.FindNode(from);
				if(idx >= 0) g.GetNode(idx).SetFill(Color(255, 220, 220));
			}
			else {
				graph_view.AddEdge(from, "out", to, "in");
			}
		}
	}
	
graph_view.Refresh();
}

void StateEditor::NewState() {
	puml_editor.Insert(puml_editor.GetCursor(), "state NewState {\n}\n");
}

void StateEditor::NewTransition() {
	puml_editor.Insert(puml_editor.GetCursor(), "[*] --> InitialState\n");
}

void StateEditor::Save() {
	String dir = AppendFileName(root, "docs/workflows");
	RealizeDirectory(dir);
	String path = AppendFileName(dir, current_id + ".puml");
	if(SaveFile(path, puml_editor.GetData())) {
		PromptOK("Workflow saved.");
	}
}

} // namespace Upp