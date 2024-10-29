#include "AI.h"
#include <ide/ide.h>

NAMESPACE_UPP

AICodeCtrl::AICodeCtrl()
{
	Add(hsplit.SizePos());
	hsplit.Horz() << editor << rsplit;

	editor.Highlight("cpp");
	editor.SetReadOnly();
	editor.WhenBar << THISBACK(ContextMenu);
	editor.WhenSel << THISBACK(CheckEditorCursor);

	rsplit.Vert() << cursorinfo << depthfirst;
	
	cursorinfo.AddColumn("Id");
	cursorinfo.AddColumn("Type");
	cursorinfo.AddColumn("Ref-pos");
	
	depthfirst.AddColumn("File");
	depthfirst.AddColumn("Pos");
	depthfirst.AddColumn("Id");
	depthfirst.AddColumn("Type");
	depthfirst.AddColumn("Ref-pos");
	depthfirst.ColumnWidths("4 1 4 4 1");
	
}

ArrayMap<String, AionFile>& AICodeCtrl::AionFiles()
{
	static ArrayMap<String, AionFile> map;
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

	AionFile& aion = AionFiles().GetAdd(aion_path);
	aion.SetPath(aion_path);
	aion.Load();

	this->content = str.Get(str.GetSize());
	this->charset = charset;
	this->hash_sha1 = SHA1String(this->content);

	UpdateEditor();
}

void AICodeCtrl::UpdateEditor()
{
	Vector<String> lines = Split(this->content, "\n", false);
	AiFileInfo& f = AiIndex().ResolveFileInfo(this->includes, this->filepath);

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
			AiAnnotationItem::SourceFile& df = item.RealizeFileByHashSha1(hash_sha1);
			for(const AiAnnotationItem::SourceFile::Item& c : df.items) {
				if (c.data_i < 0 || c.data_i >= item.GetDataCount())
					throw Exc("error: invalid data_i in AiAnnotationItem::SourceFile::Item");
				String data = item.GetDataString(c.data_i);
				Item& it = items.Add();
				it.line = df.begin.y + c.rel_line;
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
	// AionFile& aion = AionFiles().GetAdd(aion_path);
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
	bar.Sub("AI", [&](Bar& b) {
		b.Add("Create AI comments for the scope", THISBACK(MakeAiComments));
	});
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
	SourceFile::Item& c = sel_ann_f->items.Add();
	c.kind = SourceFile::Item::COMMENT;
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
	String s = this->content;
	s.Replace("\r", "");
	args.code = Split(s, "\n", false);

	// Trim annotation area of the code
	if(sel_ann_f->begin.y > 0)
		args.code.Remove(0, sel_ann_f->begin.y);
	if(sel_ann_f->begin.x > 0)
		args.code[0] = args.code[0].Mid(sel_ann_f->begin.x);
	int len = sel_ann_f->end.y - sel_ann_f->begin.y + 1;
	args.code.SetCount(len);
	if(sel_ann_f->end.x > 0)
		args.code.Top() = args.code.Top().Left(sel_ann_f->end.x);
	// DUMPC(args.code);
	auto* cur_sel_ann = sel_ann;
	auto* cur_sel_ann_f = sel_ann_f;

	m.GetCode(args, [&, cur_sel_ann](String result) {
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
		cur_sel_ann_f->RemoveAll(SourceFile::Item::COMMENT);
		for(auto c : ~comments) {
			int data_i = cur_sel_ann->FindAddData(c.value);
			auto& item = cur_sel_ann_f->items.Add();
			item.kind = SourceFile::Item::COMMENT;
			item.rel_line = c.key;
			item.data_i = data_i;
			//item.line_hash =
			//	c.key < args.code.GetCount() ? args.code[c.key].GetHashValue() : 0;
		}
		cur_sel_ann->Sort();

		PostCallback([this] {
			StoreAion();
			UpdateEditor();
		});
	});
}

void AICodeCtrl::StoreAion()
{
	AionFile& af = AiIndex().ResolveFile(this->includes, this->filepath);
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
	ASSERT(!this->filepath.IsEmpty() && !this->hash_sha1.IsEmpty());
	AiFileInfo& f = AiIndex().ResolveFileInfo(this->includes, this->filepath);
	sel_ann_f = 0;
	sel_ann = 0;
	sel_f = 0;

	for(int i = 0; i < f.ai_items.GetCount(); i++) {
		AiAnnotationItem& item = f.ai_items[i];
		SourceFile& sf = item.RealizeFileByHashSha1(this->hash_sha1);
		if(sel_line >= sf.begin.y && sel_line <= sf.end.y) {
			sel_f = &f;
			sel_ann = &item;
			sel_ann_f = &sf;
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
	AiFileInfo& info = *sel_f;
	AiAnnotationItem& ann = *sel_ann;
	SourceFile& ann_f = *sel_ann_f;
	FileAnnotation& fa = codeidx[i];
	
	int row = 0;
	for(int i = 0; i < fa.items.GetCount(); i++) {
		const AnnotationItem& ai = fa.items[i];
		if (ann_f.begin.y <= ai.pos.y && ai.pos.y <= ann_f.end.y) {
			cursorinfo.Set(row, 0, ai.id);
			cursorinfo.Set(row, 1, ai.type);
			cursorinfo.Set(row, 2, Value());
			row++;
		}
	}
	for(int i = 0; i < fa.locals.GetCount(); i++) {
		const AnnotationItem& ai = fa.locals[i];
		if (ann_f.begin.y <= ai.pos.y && ai.pos.y <= ann_f.end.y) {
			cursorinfo.Set(row, 0, ai.id);
			cursorinfo.Set(row, 1, ai.type);
			cursorinfo.Set(row, 2, Value());
			row++;
		}
	}
	for(int i = 0; i < fa.refs.GetCount(); i++) {
		const ReferenceItem& ref = fa.refs[i];
		if (ann_f.begin.y <= ref.pos.y && ref.pos.y <= ann_f.end.y) {
			cursorinfo.Set(row, 0, ref.id);
			cursorinfo.Set(row, 1, ref.pos.y);
			cursorinfo.Set(row, 2, ref.ref_pos.y);
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
		if (it.ann) {
			const auto& ai = *it.ann;
			depthfirst.Set(row, 2, ai.id);
			depthfirst.Set(row, 3, ai.type);
			depthfirst.Set(row, 4, Value());
			row++;
		}
		if (it.ref) {
			const auto& ref = *it.ref;
			depthfirst.Set(row, 2, ref.id);
			depthfirst.Set(row, 3, ref.pos.y);
			depthfirst.Set(row, 4, ref.ref_pos.y);
			row++;
		}
	}
	depthfirst.SetCount(row);
}

END_UPP_NAMESPACE

