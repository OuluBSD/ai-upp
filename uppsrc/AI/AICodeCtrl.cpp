#include "AI.h"
#include <ide/ide.h>

NAMESPACE_UPP

#if 0

AICodeCtrl::AICodeCtrl()
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
	cursorinfo.ColumnWidths("2 4 4 1 1 2");
	
	depthfirst.AddColumn("File");
	depthfirst.AddColumn("Pos");
	depthfirst.AddColumn("Kind");
	depthfirst.AddColumn("Id");
	depthfirst.AddColumn("Type");
	depthfirst.AddColumn("Ref-pos");
	depthfirst.AddColumn("Error");
	depthfirst.ColumnWidths("1 1 2 4 4 1 4");
	
}

ArrayMap<String, MetaSrcPkg>& AICodeCtrl::MetaSrcPkgs()
{
	static ArrayMap<String, MetaSrcPkg> map;
	return map;
}

void AICodeCtrl::SetFont(Font fnt) { editor.SetFont(fnt); }

void AICodeCtrl::Load(const String& includes, String filename, Stream& str, byte charset)
{
	Ide* ide = TheIde();
	this->filepath = NormalizePath(filename);
	this->includes = includes;
	String dir = GetFileDirectory(filename);
	String pkg_name = GetFileTitle(dir.Left(dir.GetCount() - 1));
	this->aion_path = AppendFileName(dir, "AI.json");

	MetaSrcPkg& aion = MetaSrcPkgs().GetAdd(aion_path);
	aion.SetPath(aion_path);
	aion.Load();

	this->content = str.Get((int)str.GetSize());
	this->charset = charset;

	UpdateEditor();
}

void AICodeCtrl::UpdateEditor()
{
	Vector<String> lines = Split(this->content, "\n", false);
	MetaSrcFile& f = MetaEnv().ResolveFileInfo(this->includes, this->filepath);

	editor_to_line.SetCount(0);
	line_to_editor.SetCount(0);
	comment_to_line.SetCount(0);
	editor_to_line.SetCount(lines.GetCount());
	comment_to_line.SetCount(lines.GetCount(), -1);
	for(int i = 0; i < editor_to_line.GetCount(); i++)
		editor_to_line[i] = i;

	try {
		struct Item : Moveable<Item> {
			String txt;
			int line;
			bool operator()(const Item& a, const Item& b) const
			{
				return a.line > b.line;
			} // in reverse because of simple insertion method
		};
		Vector<Item> items;
		for(AiAnnotationItem& item : f.ai_items) {
			AiAnnotationItem::SourceRange* df = item.FindAnySourceRange();
			if (!df)
				continue;
			for(const AiAnnotationItem::SourceRange::Item& c : df->items) {
				if (c.data_i < 0 || c.data_i >= item.GetDataCount())
					throw Exc("error: invalid data_i in AiAnnotationItem::SourceRange::Item");
				String data = item.GetDataString(c.data_i);
				Item& it = items.Add();
				it.line = df->begin.y + c.rel_line;
				it.txt = data;
			}
		}

		Sort(items, Item());

		for(const Item& it : items) {
			if(it.line < 0 || it.line >= lines.GetCount())
				continue;

			String spaces;
			const String& line = lines[it.line];
			for(int i = 0; i < line.GetCount(); i++) {
				int chr = line[i];
				if(!IsSpace(chr))
					break;
				spaces.Cat(chr);
			}
			if(TrimLeft(line).Left(1) == "}")
				spaces.Cat('\t');
			String insert_line = spaces + "/// " + it.txt;
			lines.Insert(it.line, insert_line);
			editor_to_line.Insert(it.line, -1);
			comment_to_line.Insert(it.line, it.line);
		}
	}
	catch (Exc e) {
		editor.Clear();
		PromptOK(e);
		return;
	}
	line_to_editor.SetCount(lines.GetCount(), -1);
	for(int i = 0; i < editor_to_line.GetCount(); i++) {
		int l = editor_to_line[i];
		if(l >= 0)
			line_to_editor[l] = i;
	}

	Point scroll_pos = editor.GetScrollPos();
	int cursor = editor.GetCursor();
	String s = Join(lines, "\n");
	editor.Set(s, charset);
	editor.SetScrollPos(scroll_pos);
	editor.SetCursor(cursor);
}

void AICodeCtrl::Save(Stream& str, byte charset)
{
	str.Put(this->content);
	// MetaSrcPkg& aion = MetaSrcPkgs().GetAdd(aion_path);
	// aion.Save();
}

void AICodeCtrl::SetEditPos(LineEdit::EditPos pos) {}

void AICodeCtrl::SetPickUndoData(LineEdit::UndoData pos) {}

LineEdit::UndoData AICodeCtrl::PickUndoData() { return LineEdit::UndoData(); }

LineEdit::EditPos AICodeCtrl::GetEditPos() { return LineEdit::EditPos(); }

void AICodeCtrl::ContextMenu(Bar& bar)
{
	bar.Separator();
	bar.Add("Add comment", THISBACK(AddComment));
	bar.Add("Remove comment", THISBACK(RemoveComment));
	bar.Separator();
	//bar.Sub("AI", [&](Bar& bar) {
	bar.Add("Create AI comments for this scope", THISBACK(MakeAiComments));
	bar.Add("Run base analysis for this scope", THISBACK1(RunTask, AIProcess::FN_BASE_ANALYSIS));
	//});
}

void AICodeCtrl::AddComment()
{
	SetSelectedLineFromEditor();
	if(sel_line < 0)
		return;
	SetSelectedAnnotationFromLine();
	if(!sel_ann || !sel_ann_f)
		return;
	if(sel_line < 0 || sel_line >= editor_to_line.GetCount())
		return;
	int origl = editor_to_line[sel_line];
	int l = origl - sel_ann_f->begin.y;
	String txt;
	if(!EditText(txt, "Add comment", ""))
		return;
	int data_i = sel_ann->FindAddData(txt);
	SourceRange::Item& c = sel_ann_f->items.Add();
	c.kind = SourceRange::Item::COMMENT;
	c.rel_line = l;
	c.data_i = data_i;
	sel_ann->Sort();

	StoreAion();
	UpdateEditor();
}

void AICodeCtrl::RemoveComment()
{
	SetSelectedLineFromEditor();
	if(sel_line < 0)
		return;
	SetSelectedAnnotationFromLine();
	if(!sel_ann)
		return;
	if(sel_line < 0 || sel_line >= comment_to_line.GetCount())
		return;
	int origl = comment_to_line[sel_line];
	int l = origl - sel_ann_f->begin.y;
	sel_ann_f->RemoveLineItem(l);

	StoreAion();
	UpdateEditor();
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

Vector<String> AICodeCtrl::GetAnnotationAreaCode() {
	return GetStringArea(this->content, sel_ann_f->begin, sel_ann_f->end);
}

void AICodeCtrl::MakeAiComments()
{
	SetSelectedLineFromEditor();
	if(sel_line < 0) {
		PromptOK(DeQtf("No line selected"));
		return;
	}
	SetSelectedAnnotationFromLine();
	if(!sel_ann) {
		PromptOK(DeQtf("No annotation selected"));
		return;
	}
	TaskMgr& m = AiTaskManager();
	args.Clear();
	args.fn = CodeArgs::SCOPE_COMMENTS;
	args.lang = "C++";
	args.code = GetAnnotationAreaCode();

	auto* cur_sel_ann = sel_ann;
	auto* cur_sel_ann_f = sel_ann_f;

	m.GetCode(args, [&, cur_sel_ann, cur_sel_ann_f](String result) {
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
		cur_sel_ann_f->RemoveAll(SourceRange::Item::COMMENT);
		for(auto c : ~comments) {
			int data_i = cur_sel_ann->FindAddData(c.value);
			auto& item = cur_sel_ann_f->items.Add();
			item.kind = SourceRange::Item::COMMENT;
			item.rel_line = c.key;
			item.data_i = data_i;
			//item.line_hash =
			//	c.key < args.code.GetCount() ? args.code[c.key].GetHashValue() : 0;
		}
		cur_sel_ann->Sort();
		
		Ptr<Ctrl> p = this; //static_cast<Pte<AICodeCtrl>*>(this);
		PostCallback([p] {
			if (p) {
				AICodeCtrl* c = dynamic_cast<AICodeCtrl*>(&*p);
				if (c) {
					c->StoreAion();
					c->UpdateEditor();
				}
			}
		});
	});
}

void AICodeCtrl::RunTask(AIProcess::FnType t) {
	SetSelectedLineFromEditor();
	if(sel_line < 0) {
		PromptOK(DeQtf("No line selected"));
		return;
	}
	SetSelectedAnnotationFromLine();
	if(!sel_ann) {
		PromptOK(DeQtf("No annotation selected"));
		return;
	}
	auto code = GetAnnotationAreaCode();
	auto& codeidx = CodeIndex();
	int i = codeidx.Find(filepath);
	if (!sel_f || !sel_ann || !sel_ann_f || i < 0) {
		PromptOK(DeQtf("Error: no pointers found"));
		return;
	}
	FileAnnotation& fa = codeidx[i];
	
	process.RunTask(this->filepath, fa, *sel_ann_f, pick(code), AIProcess::FN_BASE_ANALYSIS);
	
	tabs.Set(1);
}

void AICodeCtrl::OnTab() {
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

void AICodeCtrl::StoreAion()
{
	MetaSrcPkg& af = MetaEnv().ResolveFile(this->includes, this->filepath);
	af.PostSave();
}

void AICodeCtrl::SetSelectedLineFromEditor()
{
	int l = editor.GetCursorLine();
	sel_line = l >= 0 && l < editor_to_line.GetCount() ? editor_to_line[l] : -1;
	if(sel_line < 0)
		sel_line = l >= 0 && l < comment_to_line.GetCount() ? comment_to_line[l] : -1;
}

void AICodeCtrl::SetSelectedAnnotationFromLine()
{
	ASSERT(!this->filepath.IsEmpty());
	MetaSrcFile& f = MetaEnv().ResolveFileInfo(this->includes, this->filepath);
	sel_ann_f = 0;
	sel_ann = 0;
	sel_f = 0;
	
	UpdateMetaSrcFile(f, this->filepath);
	
	for(int i = 0; i < f.ai_items.GetCount(); i++) {
		AiAnnotationItem& item = f.ai_items[i];
		auto* sf = item.FindAnySourceRange();
		if(sf && sel_line >= sf->begin.y && sel_line <= sf->end.y) {
			sel_f = &f;
			sel_ann = &item;
			sel_ann_f = sf;
			break;
		}
	}
}

void AICodeCtrl::CheckEditorCursor() {
	int cur = editor.GetCursor();
	if (cur != prev_editor_cursor) {
		prev_editor_cursor = cur;
		OnEditorCursor();
	}
}

void AICodeCtrl::OnEditorCursor() {
	SetSelectedLineFromEditor();
	SetSelectedAnnotationFromLine();
	AnnotationData();
}

void AICodeCtrl::AnnotationData() {
	auto& codeidx = CodeIndex();
	int i = codeidx.Find(filepath);
	if (!sel_f || !sel_ann || i < 0) {
		cursorinfo.Clear();
		return;
	}
	MetaSrcFile& info = *sel_f;
	AiAnnotationItem& ann = *sel_ann;
	SourceRange& ann_f = *sel_ann_f;
	FileAnnotation& fa = codeidx[i];
	
	int row = 0;
	for(int i = 0; i < fa.items.GetCount(); i++) {
		const AnnotationItem& ai = fa.items[i];
		if (ann_f.begin.y <= ai.pos.y && ai.pos.y <= ann_f.end.y) {
			cursorinfo.Set(row, 0, ai.kind >= 0 ? GetCursorKindName((CXCursorKind)ai.kind) : String());
			cursorinfo.Set(row, 1, ai.id);
			cursorinfo.Set(row, 2, ai.type);
			cursorinfo.Set(row, 3, ai.pos);
			cursorinfo.Set(row, 4, Value());
			row++;
		}
	}
	for(int i = 0; i < fa.locals.GetCount(); i++) {
		const AnnotationItem& ai = fa.locals[i];
		if (ann_f.begin.y <= ai.pos.y && ai.pos.y <= ann_f.end.y) {
			cursorinfo.Set(row, 0, ai.kind >= 0 ? GetCursorKindName((CXCursorKind)ai.kind) : String());
			cursorinfo.Set(row, 1, ai.id);
			cursorinfo.Set(row, 2, ai.type);
			cursorinfo.Set(row, 3, ai.pos);
			cursorinfo.Set(row, 4, Value());
			row++;
		}
	}
	for(int i = 0; i < fa.refs.GetCount(); i++) {
		const ReferenceItem& ref = fa.refs[i];
		if (ann_f.begin.y <= ref.pos.y && ref.pos.y <= ann_f.end.y) {
			cursorinfo.Set(row, 0, Value());
			cursorinfo.Set(row, 1, ref.id);
			cursorinfo.Set(row, 2, Value());
			cursorinfo.Set(row, 3, ref.pos);
			cursorinfo.Set(row, 4, ref.ref_pos);
			row++;
		}
	}
	for(int i = 0; i < fa.stmts.GetCount(); i++) {
		const StatementItem& stmt = fa.stmts[i];
		if (ann_f.begin.y <= stmt.begin.y && stmt.end.y <= ann_f.end.y) {
			String code = Join(GetStringArea(this->content, stmt.begin, stmt.end), "\\n");
			cursorinfo.Set(row, 0, stmt.kind >= 0 ? GetCursorKindName((CXCursorKind)stmt.kind) : String());
			cursorinfo.Set(row, 1, code);
			cursorinfo.Set(row, 2, Value());
			cursorinfo.Set(row, 3, stmt.begin);
			cursorinfo.Set(row, 4, Value());
			row++;
		}
	}
	cursorinfo.SetCount(row);
	
	CodeVisitor vis;
	vis.SetLimit(1000);
	vis.Begin();
	vis.Visit(filepath, fa, ann_f.begin, ann_f.end);
	
	row = 0;
	for(const auto& it : vis.export_items) {
		depthfirst.Set(row, 0, it.file);
		depthfirst.Set(row, 1, it.pos);
		if (it.have_ann || it.have_ref || it.have_link) {
			if (it.have_ann) {
				const auto& ai = it.ann;
				depthfirst.Set(row, 2, ai.kind >= 0 ? GetCursorKindName((CXCursorKind)ai.kind) : String());
				depthfirst.Set(row, 3, ai.id);
				depthfirst.Set(row, 4, ai.type);
				depthfirst.Set(row, 5, Value());
				depthfirst.Set(row, 6, it.error);
				row++;
			}
			if (it.have_ref) {
				const auto& ref = it.ref;
				depthfirst.Set(row, 2, Value());
				depthfirst.Set(row, 3, ref.id);
				if (it.have_link) {
					depthfirst.Set(row, 4, it.link.type);
				}
				else {
					depthfirst.Set(row, 4, Value());
				}
				depthfirst.Set(row, 5, ref.ref_pos);
				depthfirst.Set(row, 6, it.error);
				row++;
			}
			if (it.have_link) {
				const auto& ai = it.link;
				depthfirst.Set(row, 2, ai.kind >= 0 ? GetCursorKindName((CXCursorKind)ai.kind) : String());
				depthfirst.Set(row, 3, ai.id);
				depthfirst.Set(row, 4, ai.type);
				depthfirst.Set(row, 5, Value());
				depthfirst.Set(row, 6, it.error);
				row++;
			}
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

#endif

END_UPP_NAMESPACE

