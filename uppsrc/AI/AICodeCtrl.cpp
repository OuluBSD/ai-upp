#include "AI.h"
#include <ide/ide.h>

NAMESPACE_UPP


AICodeCtrl::AICodeCtrl() {
	AddFrame(vscroll);
	vscroll.Vert();
	vscroll << [this]{Refresh();};
	fnt = Arial(lineh);
	clr_sel = Color(182, 197, 255);
}

ArrayMap<String,AionFile>& AICodeCtrl::AionFiles() {
	static ArrayMap<String,AionFile> map;
	return map;
}

void AICodeCtrl::SetIde(Ide* ide) {
	this->ide = ide;
}

void AICodeCtrl::SetFont(Font fnt) {
	this->fnt = fnt;
	lineh = fnt.GetHeight() * 5 / 4;
	vscroll.SetLine(lineh);
}

void AICodeCtrl::Load(String filename, Stream& str, byte charset) {
	this->filepath = filename;
	String dir = GetFileDirectory(filename);
	String pkg_name = GetFileTitle(dir.Left(dir.GetCount()-1));
	this->aion_path = AppendFileName(dir, pkg_name + ".aion");
	
	AionFile& aion = AionFiles().GetAdd(aion_path);
	aion.Load(aion_path);
	
	String tab_str;
	tab_str.Cat(' ', ide->editortabsize);
	
	String file_content = LoadFile(filename);
	file_content.Replace("\r\n","\n");
	file_content.Replace("\t",tab_str);
	srclines = Split(file_content, "\n", false);
	vscroll.SetTotal(srclines.GetCount() * lineh);
	
	this->content = str.Get(str.GetSize());
}

void AICodeCtrl::Save(Stream& str, byte charset) {
	str.Put(this->content);
	
	AionFile& aion = AionFiles().GetAdd(aion_path);
	aion.Save();
}

void AICodeCtrl::SetEditPos(LineEdit::EditPos pos) {
	
}

void AICodeCtrl::SetPickUndoData(LineEdit::UndoData pos) {
	
}

LineEdit::UndoData AICodeCtrl::PickUndoData() {
	return LineEdit::UndoData();
}

LineEdit::EditPos AICodeCtrl::GetEditPos() {
	return LineEdit::EditPos();
}

void AICodeCtrl::Paint(Draw& draw) {
	Size sz = GetSize();
	draw.DrawRect(sz, White());
	int y = -vscroll;
	for(int i = 0; i < srclines.GetCount(); i++) {
		int y1 = y + lineh;
		if (y1 >= 0 && y < sz.cy) {
			if (i == sel_line)
				draw.DrawRect(Rect(0,y,sz.cx,y1), clr_sel);
			const String& line = srclines[i];
			draw.DrawText(0, y, line, fnt, Black());
		}
		y = y1;
	}
}

void AICodeCtrl::Layout() {
	vscroll.SetPage(GetSize().cy);
}

void AICodeCtrl::MouseWheel(Point p, int zdelta, dword keyflags) {
	vscroll.Wheel(zdelta);
}

void AICodeCtrl::LeftDown(Point p, dword flags) {
	int y = -vscroll;
	int clicky = p.y - y;
	int line = clicky / lineh;
	if (line >= 0 && line < srclines.GetCount())
		sel_line = line;
	else
		sel_line = -1;
	Refresh();
}


END_UPP_NAMESPACE

