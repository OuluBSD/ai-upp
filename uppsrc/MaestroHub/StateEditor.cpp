#include "StateEditor.h"

NAMESPACE_UPP

namespace {

struct PumlTransition : Moveable<PumlTransition> {
	String from;
	String to;
	String label;
	int    line = 0;
};

struct PumlSyntax : EditorSyntax {
	Vector<String> keywords;

	PumlSyntax() {
		keywords << "state" << "note" << "title" << "skinparam" << "as"
		         << "left" << "right" << "top" << "bottom"
		         << "@startuml" << "@enduml";
	}

	bool IsKeyword(const String& word) const {
		for(const auto& k : keywords)
			if(word == k)
				return true;
		return false;
	}

	void Highlight(const wchar *s, const wchar *end, HighlightOutput& hls, CodeEditor *, int, int64) override {
		const HlStyle& normal = hl_style[INK_NORMAL];
		const HlStyle& comment = hl_style[INK_COMMENT];
		const HlStyle& keyword = hl_style[INK_KEYWORD];
		const HlStyle& op = hl_style[INK_OPERATOR];
		const HlStyle& str = hl_style[INK_CONST_STRING];
		
		const wchar *p = s;
		while(p < end) {
			if(*p == '\'' || (*p == '/' && (p + 1) < end && p[1] == '/')) {
				hls.Put((int)(end - p), comment);
				break;
			}
			if(*p == '\"') {
				const wchar *q = p + 1;
				while(q < end && *q != '\"')
					q++;
				int len = (q < end) ? (int)(q - p + 1) : (int)(end - p);
				hls.Put(len, str);
				p += len;
				continue;
			}
			if(*p == '-' && (p + 2) < end && p[1] == '-' && p[2] == '>') {
				hls.Put(3, op);
				p += 3;
				continue;
			}
			if(*p == '[' && (p + 2) < end && p[1] == '*' && p[2] == ']') {
				hls.Put(3, keyword);
				p += 3;
				continue;
			}
			if(IsAlpha(*p) || *p == '@') {
				const wchar *q = p;
				String word;
				while(q < end && (IsAlNum(*q) || *q == '@' || *q == '_')) {
					word.Cat((char)ToLower((int)*q));
					q++;
				}
				if(IsKeyword(word))
					hls.Put((int)(q - p), keyword);
				else
					hls.Put((int)(q - p), normal);
				p = q;
				continue;
			}
			hls.Put(1, normal);
			p++;
		}
	}
};

void CreatePumlSyntax(One<EditorSyntax>& e)
{
	e.Create<PumlSyntax>();
}

void EnsurePumlSyntax()
{
	static bool registered = false;
	if(registered)
		return;
	EditorSyntax::Register("puml", callback(CreatePumlSyntax), "*.puml", "PlantUML");
	registered = true;
}

String TrimQuotes(String s)
{
	s = TrimBoth(s);
	if(s.GetCount() >= 2 && ((s[0] == '\"' && s[s.GetCount() - 1] == '\"') ||
	                         (s[0] == '\'' && s[s.GetCount() - 1] == '\'')))
		return s.Mid(1, s.GetCount() - 2);
	return s;
}

String NormalizeNodeId(String s)
{
	s = TrimBoth(s);
	if(s == "[*]" || s == "*")
		return "Start";
	s = TrimQuotes(s);
	s = TrimBoth(s);
	if(s.EndsWith("{"))
		s = TrimBoth(s.Left(s.GetCount() - 1));
	return s;
}

void ParsePuml(const String& puml, Index<String>& nodes, VectorMap<String, String>& labels,
               Vector<PumlTransition>& transitions, Vector<String>& errors, Vector<Point>& error_marks)
{
	Vector<String> lines = Split(puml, '\n', false);
	for(int i = 0; i < lines.GetCount(); i++) {
		String raw = TrimBoth(lines[i]);
		if(raw.IsEmpty())
			continue;
		String trimmed = TrimLeft(raw);
		if(trimmed.StartsWith("'") || trimmed.StartsWith("//"))
			continue;
		if(trimmed.StartsWith("@"))
			continue;
		
		if(trimmed.StartsWith("state ")) {
			String rest = TrimBoth(trimmed.Mid(6));
			if(rest.IsEmpty()) {
				errors.Add(Format("Line %d: missing state name.", i + 1));
				error_marks.Add(Point(0, i));
				continue;
			}
			String label;
			String id = rest;
			int as_pos = ToLower(rest).Find(" as ");
			if(as_pos >= 0) {
				label = TrimQuotes(rest.Left(as_pos));
				id = TrimBoth(rest.Mid(as_pos + 4));
			}
			id = NormalizeNodeId(id);
			if(id.IsEmpty()) {
				errors.Add(Format("Line %d: invalid state name.", i + 1));
				error_marks.Add(Point(0, i));
				continue;
			}
			nodes.Add(id);
			if(!label.IsEmpty())
				labels.GetAdd(id) = label;
			continue;
		}
		
		int arrow = trimmed.Find("-->");
		int arrow_len = 3;
		if(arrow < 0) {
			arrow = trimmed.Find("->");
			arrow_len = 2;
		}
		if(arrow >= 0) {
			String left = TrimBoth(trimmed.Left(arrow));
			String right = TrimBoth(trimmed.Mid(arrow + arrow_len));
			String label;
			int colon = right.Find(":");
			if(colon >= 0) {
				label = TrimBoth(right.Mid(colon + 1));
				right = TrimBoth(right.Left(colon));
			}
			left = NormalizeNodeId(left);
			right = NormalizeNodeId(right);
			if(left.IsEmpty() || right.IsEmpty()) {
				errors.Add(Format("Line %d: invalid transition syntax.", i + 1));
				error_marks.Add(Point(0, i));
				continue;
			}
			nodes.Add(left);
			nodes.Add(right);
			PumlTransition& t = transitions.Add();
			t.from = left;
			t.to = right;
			t.label = label;
			t.line = i;
			continue;
		}
		
		if(trimmed.GetCount()) {
			errors.Add(Format("Line %d: unsupported syntax.", i + 1));
			error_marks.Add(Point(0, i));
		}
	}
}

}

StateEditor::StateEditor() {
	CtrlLayout(*this, "Workflow State Machine Editor");
	
	EnsurePumlSyntax();
	puml_editor.Highlight("puml");
	puml_editor.LineNumbers(true);
	
	error_label.SetInk(LtRed());
	error_label.SetText("");
	
	puml_panel.Add(puml_editor.HSizePos().VSizePos(0, Zy(20)));
	puml_panel.Add(error_label.HSizePos().BottomPos(0, Zy(20)));
	
	split.Horz(puml_panel, graph_view);
	
	toolbar.Set(THISBACK(OnToolbar));
	
	puml_editor.WhenAction = [=] {
		SetTimeCallback(250, THISBACK(UpdatePreview), 1);
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
	Index<String> nodes;
	VectorMap<String, String> labels;
	Vector<PumlTransition> transitions;
	Vector<String> errors;
	Vector<Point> error_marks;
	
	ParsePuml(puml, nodes, labels, transitions, errors, error_marks);
	
	GraphLib::Graph& graph = graph_view.GetGraph();
	graph.Clear();
	
	int col = 0;
	int row = 0;
	const int col_width = 200;
	const int row_height = 140;
	for(int i = 0; i < nodes.GetCount(); i++) {
		Point pos(40 + col * col_width, 40 + row * row_height);
		GraphLib::Node& node = graph_view.AddNode(nodes[i], pos);
		String label = labels.Get(nodes[i], nodes[i]);
		node.SetLabel(label);
		if(nodes[i] == "Start") {
			node.shape = 1;
			node.sz = Size(50, 50);
		}
		col++;
		if(col >= 3) {
			col = 0;
			row++;
		}
	}
	
	for(const auto& t : transitions) {
		GraphLib::Edge& e = graph.AddEdge(t.from, t.to);
		e.SetDirected(true);
		if(!t.label.IsEmpty())
			e.SetLabel(t.label);
	}
	
	graph_view.Refresh();
	
	if(errors.IsEmpty()) {
		error_label.SetText("");
	} else {
		error_label.SetText(errors[0]);
	}
	puml_editor.Errors(pick(error_marks));
}

void StateEditor::NewState() {
	puml_editor.Insert(puml_editor.GetCursor(), "state NewState\n");
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

END_UPP_NAMESPACE
