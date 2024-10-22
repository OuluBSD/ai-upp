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
	editor.WhenAction << THISBACK(CheckEditorCursor);

	rsplit.Vert() << cursorinfo << rangevars << rangecalls << timeline;
}

ArrayMap<String, AionFile>& AICodeCtrl::AionFiles()
{
	static ArrayMap<String, AionFile> map;
	return map;
}

void AICodeCtrl::SetFont(Font fnt) { editor.SetFont(fnt); }

void AICodeCtrl::Load(String filename, Stream& str, byte charset)
{
	Ide* ide = TheIde();
	this->filepath = NormalizePath(filename);
	String dir = GetFileDirectory(filename);
	String pkg_name = GetFileTitle(dir.Left(dir.GetCount() - 1));
	this->aion_path = AppendFileName(dir, "AI.json");

	AionFile& aion = AionFiles().GetAdd(aion_path);
	aion.SetPath(aion_path);
	aion.Load();

	this->content = str.Get(str.GetSize());
	this->charset = charset;

	UpdateEditor();
}

void AICodeCtrl::UpdateEditor()
{
	Vector<String> lines = Split(this->content, "\n", false);
	AiFileInfo& f = AiIndex().ResolveFileInfo(this->filepath);

	editor_to_line.SetCount(0);
	line_to_editor.SetCount(0);
	comment_to_line.SetCount(0);
	editor_to_line.SetCount(lines.GetCount());
	comment_to_line.SetCount(lines.GetCount(), -1);
	for(int i = 0; i < editor_to_line.GetCount(); i++)
		editor_to_line[i] = i;

	{
		struct Item : Moveable<Item> {
			String txt;
			int line;
			bool operator()(const Item& a, const Item& b) const
			{
				return a.line > b.line;
			} // in reverse because of simple insertion method
		};
		Vector<Item> items;
		for(const AiAnnotationItem& item : f.ai_items) {
			for(const AiAnnotationItem::Comment& c : item.comments) {
				Item& it = items.Add();
				it.line = item.begin.y + c.rel_line;
				it.txt = c.txt;
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
	if(!sel_ann)
		return;
	if(sel_line < 0 || sel_line >= editor_to_line.GetCount())
		return;
	int origl = editor_to_line[sel_line];
	int l = origl - sel_ann->begin.y;
	String txt;
	if(!EditText(txt, "Add comment", ""))
		return;
	AiAnnotationItem::Comment& c = sel_ann->comments.Add();
	c.line_hash = 0;
	c.rel_line = l;
	c.txt = txt;

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
	int l = origl - sel_ann->begin.y;
	sel_ann->RemoveCommentLine(l);

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
	if(sel_ann->begin.y > 0)
		args.code.Remove(0, sel_ann->begin.y);
	if(sel_ann->begin.x > 0)
		args.code[0] = args.code[0].Mid(sel_ann->begin.x);
	int len = sel_ann->end.y - sel_ann->begin.y + 1;
	args.code.SetCount(len);
	if(sel_ann->end.x > 0)
		args.code.Top() = args.code.Top().Left(sel_ann->end.x);
	// DUMPC(args.code);
	auto* cur_sel_ann = sel_ann;

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
		cur_sel_ann->comments.Clear();
		for(auto c : ~comments) {
			auto& comment = cur_sel_ann->comments.Add();
			comment.rel_line = c.key;
			comment.txt = c.value;
			comment.line_hash =
				c.key < args.code.GetCount() ? args.code[c.key].GetHashValue() : 0;
		}

		PostCallback([this] {
			StoreAion();
			UpdateEditor();
		});
	});
}

void AICodeCtrl::StoreAion()
{
	AionFile& af = AiIndex().ResolveFile(this->filepath);
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
	AiFileInfo& f = AiIndex().ResolveFileInfo(this->filepath);
	sel_ann = 0;
	sel_f = 0;

	for(int i = 0; i < f.ai_items.GetCount(); i++) {
		auto& item = f.ai_items[i];
		if(sel_line >= item.begin.y && sel_line <= item.end.y) {
			sel_f = &f;
			sel_ann = &item;
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
	if (!sel_ann) {
		cursorinfo.Clear();
		rangevars.Clear();
		rangecalls.Clear();
		timeline.Clear();
		return;
	}
	AiAnnotationItem& ann = *sel_ann;
	
}

END_UPP_NAMESPACE

