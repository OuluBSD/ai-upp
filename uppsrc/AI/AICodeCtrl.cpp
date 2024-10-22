#include "AI.h"
#include <ide/ide.h>

NAMESPACE_UPP

AICodeCtrl::AICodeCtrl()
{
	Add(editor.SizePos());
	AddFrame(navigator.Left(split, 150));
	split.Vert() << itemlist << commentlist << datalist;

	editor.Highlight("cpp");
	editor.SetReadOnly();

	editor.WhenBar << THISBACK(ContextMenu);
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
				it.line = item.pos.y + c.rel_line;
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

void AICodeCtrl::MakeAiComments() {}

void AICodeCtrl::StoreAion()
{
	AionFile& af = AiIndex().ResolveFile(this->filepath);
	af.PostSave();
}

void AICodeCtrl::SetSelectedLineFromEditor() { sel_line = editor.GetCursorLine(); }

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

END_UPP_NAMESPACE

