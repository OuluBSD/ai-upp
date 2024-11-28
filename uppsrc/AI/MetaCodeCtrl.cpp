#include "AI.h"
#include <ide/ide.h>

NAMESPACE_UPP

MetaCodeCtrl::MetaCodeCtrl()
{
	Add(hsplit.SizePos());
	hsplit.Horz() << editor << tabs;
	
	tabs.Add(rsplit.SizePos(), "Code");
	tabs.Add(process.SizePos(), "Process");
	tabs.WhenSet = THISBACK(OnTab);
	
	editor.Highlight("cpp");
	editor.LineNumbers(true);
	editor.SetReadOnly();
	editor.WhenBar << THISBACK(ContextMenu);
	editor.WhenSel << THISBACK(CheckEditorCursor);

	rsplit.Vert() << cursorinfo << depthfirst;
	
	cursorinfo.AddColumn("Kind");
	cursorinfo.AddColumn("Id");
	cursorinfo.AddColumn("Type");
	cursorinfo.AddColumn("Pos");
	cursorinfo.AddColumn("Ref-pos");
	cursorinfo.ColumnWidths("2 4 4 1 1");
	
	depthfirst.AddColumn("File");
	depthfirst.AddColumn("Pos");
	depthfirst.AddColumn("Kind");
	depthfirst.AddColumn("Id");
	depthfirst.AddColumn("Type");
	depthfirst.AddColumn("Ref-pos");
	depthfirst.ColumnWidths("1 1 2 4 4 1");
	
}

void MetaCodeCtrl::SetFont(Font fnt) { editor.SetFont(fnt); }

void MetaCodeCtrl::Load(const String& includes, String filename, Stream& str, byte charset)
{
	Ide* ide = TheIde();
	this->filepath = NormalizePath(filename);
	this->includes = includes;
	
	MetaEnvironment& env = MetaEnv();
	env.Load(filepath, includes);

	this->content = str.Get((int)str.GetSize());
	this->charset = charset;

	UpdateEditor();
}

void MetaCodeCtrl::UpdateEditor()
{
	
	auto& env = MetaEnv();
	MetaSrcPkg& pkg = env.ResolveFile(this->includes, this->filepath);
	String rel_path = pkg.GetRelativePath(this->filepath);
	int pkg_i = pkg.id;
	int file_i = pkg.filenames.Find(rel_path);
	ASSERT(pkg_i >= 0 && file_i >= 0);
	MetaNodeSubset sub;
	env.SplitNode(env.root, sub, pkg_i, file_i);
	
	gen.Process(sub);
	gen_file = gen.GetResultFile(pkg_i, file_i);
	String code;
	if (gen_file) {
		code = gen_file ? gen_file->code : String();
		
		editor_to_line <<= gen_file->editor_to_line;
		comment_to_node <<= gen_file->comment_to_node;
	}

	Point scroll_pos = editor.GetScrollPos();
	int cursor = editor.GetCursor();
	
	editor.Set(code, charset);
	editor.SetScrollPos(scroll_pos);
	editor.SetCursor(cursor);
}

void MetaCodeCtrl::Save(Stream& str, byte charset)
{
	str.Put(this->content);
	// MetaSrcPkg& aion = MetaSrcPkgs().GetAdd(aion_path);
	// aion.Save();
}

void MetaCodeCtrl::SetEditPos(LineEdit::EditPos pos) {}

void MetaCodeCtrl::SetPickUndoData(LineEdit::UndoData pos) {}

LineEdit::UndoData MetaCodeCtrl::PickUndoData() { return LineEdit::UndoData(); }

LineEdit::EditPos MetaCodeCtrl::GetEditPos() { return LineEdit::EditPos(); }

void MetaCodeCtrl::ContextMenu(Bar& bar)
{
	bar.Separator();
	bar.Add("Add comment", THISBACK(AddComment));
	bar.Add("Remove comment", THISBACK(RemoveComment));
	bar.Separator();
	//bar.Sub("AI", [&](Bar& bar) {
	bar.Add("Create AI comments for this scope", THISBACK(MakeAiComments));
	bar.Add("Run base analysis for this scope", THISBACK1(RunTask, MetaProcess::FN_BASE_ANALYSIS));
	//});
}

void MetaCodeCtrl::AddComment()
{
	SetSelectedLineFromEditor();
	if(!sel_node)
		return;
	int sel_line = editor.GetCursorLine();
	int origl = editor_to_line[sel_line];
	int l = origl - sel_node->begin.y;
	String txt;
	if(!EditText(txt, "Add comment", ""))
		return;
	MetaNode& cn = sel_node->Add();
	cn.kind = METAKIND_COMMENT;
	cn.end = Point(0,origl);
	cn.begin = Point(0,origl);
	cn.id = txt;
	cn.file = sel_node->file;
	cn.pkg = sel_node->pkg;
	//sel_ann->Sort();
	StoreAion();
	UpdateEditor();
}

void MetaCodeCtrl::RemoveComment()
{
	SetSelectedLineFromEditor();
	int sel_line = editor.GetCursorLine();
	MetaNode* c = sel_line < comment_to_node.GetCount() ? comment_to_node[sel_line] : 0;
	if (c) {
		c->Destroy();
		StoreAion();
		UpdateEditor();
	}
}

Vector<String> GetStringArea(const String& content, Point begin, Point end) {
	Vector<String> code;
	String s = content;
	s.Replace("\r", "");
	code = Split(s, "\n", false);

	// Trim annotation area of the code
	if(begin.y > 0)
		code.Remove(0, begin.y);
	if(begin.x > 0)
		code[0] = code[0].Mid(begin.x);
	int len = end.y - begin.y + 1;
	code.SetCount(len);
	if(end.x > 0)
		code.Top() = code.Top().Left(end.x);
	// DUMPC(code);
	return code;
}

Vector<String> MetaCodeCtrl::GetAnnotationAreaCode() {
	return GetStringArea(this->content, sel_node->begin, sel_node->end);
}

void MetaCodeCtrl::MakeAiComments()
{
	SetSelectedLineFromEditor();
	if(!sel_node) {
		PromptOK(DeQtf("No line selected"));
		return;
	}
	
	TaskMgr& m = AiTaskManager();
	args.Clear();
	args.fn = CodeArgs::SCOPE_COMMENTS;
	args.lang = "C++";
	args.code = GetAnnotationAreaCode();

	auto cur_sel_node = sel_node;

	m.GetCode(args, [&, cur_sel_node](String result) {
		Vector<String> lines = Split(result, "\n");
		VectorMap<int, String> comments;
		for(String& l : lines) {
			int a = l.Find("line #");
			if(a < 0)
				continue;
			l = l.Mid(a + 6);
			int line = ScanInt(l); // Get the number of the line
			a = l.Find(":");
			if(a < 0)
				continue;
			l = TrimBoth(l.Mid(a + 1)); // Get the answer only
			if(l.IsEmpty())
				continue;
			if(l[0] == '\"') // Remove possible quotes
				l = l.Mid(1, l.GetCount() - 2);
			a = l.Find("//");
			if(a >= 0)
				l = TrimLeft(l.Mid(a + 2));
			comments.GetAdd(line) = l;
		}
		// DUMPM(comments);
		cur_sel_node->RemoveAllDeep(METAKIND_COMMENT);
		for(auto c : ~comments) {
			Point pt = cur_sel_node->begin;
			pt.y += c.key;
			//MetaNode* closest = cur_sel_node->FindClosest(pt);
			MetaNode& cn = cur_sel_node->Add();
			cn.kind = METAKIND_COMMENT;
			cn.id = c.value;
			cn.begin = cn.end = pt;
			cn.file = cur_sel_node->file;
			cn.pkg = cur_sel_node->pkg;
		}
		//cur_sel_node->Sort();
		
		Ptr<Ctrl> p = this; //static_cast<Pte<MetaCodeCtrl>*>(this);
		PostCallback([p] {
			if (p) {
				MetaCodeCtrl* c = dynamic_cast<MetaCodeCtrl*>(&*p);
				if (c) {
					c->StoreAion();
					c->UpdateEditor();
				}
			}
		});
	});
	
}

void MetaCodeCtrl::RunTask(MetaProcess::FnType t) {
	SetSelectedLineFromEditor();
	if(!sel_node) {
		PromptOK(DeQtf("No node selected"));
		return;
	}
	auto code = GetAnnotationAreaCode();
	auto& codeidx = CodeIndex();
	int i = codeidx.Find(filepath);
	if (!sel_node || i < 0) {
		PromptOK(DeQtf("Error: no pointers found"));
		return;
	}
	FileAnnotation& fa = codeidx[i];
	
	process.RunTask(this->filepath, *sel_node, pick(code), MetaProcess::FN_BASE_ANALYSIS);
	
	tabs.Set(1);
}

void MetaCodeCtrl::OnTab() {
	int tab = tabs.Get();
	
	if (tab == 1) {
		tc.Set(-500, [this]{
			process.Data();
		});
	}
	else {
		tc.Kill();
	}
}

void MetaCodeCtrl::StoreAion()
{
	auto& env = MetaEnv();
	MetaSrcPkg& af = env.ResolveFile(this->includes, this->filepath);
	env.Store(af, true);
}

void MetaCodeCtrl::SetSelectedLineFromEditor()
{
	int64 pos = editor.GetCursor64();
	Point pt;
	int sel_line = editor.GetCursorLine();
	int origl = editor_to_line[sel_line];
	pt.y = origl;
	pt.x = pos - editor.GetPos(sel_line);
	for (auto n : ~gen_file->code_nodes) {
		if (n.key.Contains(pt)) {
			MetaNode& sel = *n.value;
			ASSERT(!sel.only_temporary);
			this->sel_node = &sel;
			return;
		}
	}
	this->sel_node = 0;
}

void MetaCodeCtrl::CheckEditorCursor() {
	int cur = editor.GetCursor();
	if (cur != prev_editor_cursor) {
		prev_editor_cursor = cur;
		OnEditorCursor();
	}
}

void MetaCodeCtrl::OnEditorCursor() {
	if (!gen_file)
		return;
	SetSelectedLineFromEditor();
	AnnotationData();
}

void MetaCodeCtrl::VisitCursorInfo(MetaNode& n, int& row) {
	cursorinfo.Set(row, 0, n.kind >= 0 ? GetCursorKindName((CXCursorKind)n.kind) : String());
	cursorinfo.Set(row, 1, n.id);
	cursorinfo.Set(row, 2, n.type);
	cursorinfo.Set(row, 3, n.begin);
	MetaNode* decl = n.is_ref ? MetaEnv().FindDeclaration(n) : 0;
	if (decl)
		cursorinfo.Set(row, 4, decl->begin);
	else if (n.is_ref)
		cursorinfo.Set(row, 4, "<decl not found>");
	else
		cursorinfo.Set(row, 4, Value());
	row++;
	
	for (auto& s : n.sub)
		VisitCursorInfo(s, row);
}

void MetaCodeCtrl::AnnotationData() {
	auto& codeidx = CodeIndex();
	int i = codeidx.Find(filepath);
	
	if (!sel_node || i < 0) {
		cursorinfo.Clear();
		return;
	}
	
	int row = 0;
	VisitCursorInfo(*sel_node, row);
	cursorinfo.SetCount(row);
	
	CodeVisitor vis;
	vis.SetLimit(1000);
	vis.Begin();
	vis.Visit(filepath, *sel_node);
	
	row = 0;
	for(const auto& it : vis.export_items) {
		depthfirst.Set(row, 0, it.file);
		depthfirst.Set(row, 1, it.pos);
		if (it.node) {
			MetaNode& n = *it.node;
			depthfirst.Set(row, 2, n.kind >= 0 ? GetCursorKindName((CXCursorKind)n.kind) : String());
			depthfirst.Set(row, 3, n.id);
			depthfirst.Set(row, 4, n.type);
			if (it.link_node)
				depthfirst.Set(row, 5, it.link_node->begin);
			else
				depthfirst.Set(row, 5, Value());
			depthfirst.Set(row, 6, it.error);
			row++;
		}
		else {
			depthfirst.Set(row, 2, Value());
			depthfirst.Set(row, 3, Value());
			depthfirst.Set(row, 4, Value());
			depthfirst.Set(row, 5, Value());
			depthfirst.Set(row, 6, Value());
			depthfirst.Set(row, 7, it.error);
			row++;
		}
	}
	depthfirst.SetCount(row);
}

END_UPP_NAMESPACE

