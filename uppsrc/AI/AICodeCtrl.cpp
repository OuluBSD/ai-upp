#include "AI.h"
#include <ide/ide.h>


NAMESPACE_UPP


AICodeCtrl::AICodeCtrl() {
	AddFrame(vscroll);
	vscroll.Vert();
	vscroll << [this]{Refresh();};
	fnt = Arial(lineh);
	clr_sel = Color(182, 197, 255);
	clr_ann = Color(255, 197, 182);
}

ArrayMap<String,AionFile>& AICodeCtrl::AionFiles() {
	static ArrayMap<String,AionFile> map;
	return map;
}

void AICodeCtrl::SetFont(Font fnt) {
	this->fnt = fnt;
	lineh = fnt.GetHeight() * 5 / 4;
	vscroll.SetLine(lineh);
}

void AICodeCtrl::Load(String filename, Stream& str, byte charset) {
	Ide* ide = TheIde();
	this->filepath = NormalizePath(filename);
	String dir = GetFileDirectory(filename);
	String pkg_name = GetFileTitle(dir.Left(dir.GetCount()-1));
	this->aion_path = AppendFileName(dir, "AI.json");
	
	AionFile& aion = AionFiles().GetAdd(aion_path);
	aion.Load(aion_path);
	
	//AionSource src(this->filepath, aion);
	//src.Update();
	
	String tab_str;
	tab_str.Cat(' ', ide->editortabsize);
	
	String file_content = LoadFile(filename);
	file_content.Replace("\r\n","\n");
	file_content.Replace("\t",tab_str);
	srclines = Split(file_content, "\n", false);
	vscroll.SetTotal(srclines.GetCount() * lineh);
	
	this->content = str.Get(str.GetSize());
	
	ArrayMap<String, FileAnnotation>& x = CodeIndex();
	int i = x.Find(this->filepath);
	if (i >= 0) {
		FileAnnotation& f = x[i];
		
		//String txt = LoadFile(this->filepath);
		//Vector<String> lines = Split(txt, "\n", false);
		
		for(int i = 0; i < f.ai_items.GetCount(); i++) {
			const auto& item = f.ai_items[i];
			LOG(i << ": " << item.pos << ": " << item.id << ", type:" << item.type << ", nest: " << item.nest);
			
			/*String item_txt;
			for (int l = item.begin.y; l <= item.end.y; l++) {
				if (l < 0 || l >= lines.GetCount())
					continue;
				const String& line = lines[l];
				int begin = 0, end = line.GetCount();
				if (l == item.begin.y)
					begin = item.begin.x;
				else if (l == item.end.y)
					end = min(end, item.end.x);
				
				if (!item_txt.IsEmpty())
					item_txt.Cat('\n');
				item_txt << line.Mid(begin, end-begin);
			}
			LOG(item_txt);*/
		}
	}
	
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
			else if (sel_ann && i >= sel_ann->begin.y && i <= sel_ann->end.y)
				draw.DrawRect(Rect(0,y,sz.cx,y1), clr_ann);
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
	SetSelectedAnnotationFromLine();
	Refresh();
}

void AICodeCtrl::SetSelectedAnnotationFromLine() {
	ArrayMap<String, FileAnnotation>& x = CodeIndex();
	int i = x.Find(this->filepath);
	sel_ann = 0;
	if (i >= 0) {
		FileAnnotation& f = x[i];
		for(int i = 0; i < f.ai_items.GetCount(); i++) {
			auto& item = f.ai_items[i];
			if (sel_line >= item.begin.y && sel_line <= item.end.y) {
				sel_ann = &item;
				break;
			}
		}
	}
}


END_UPP_NAMESPACE

