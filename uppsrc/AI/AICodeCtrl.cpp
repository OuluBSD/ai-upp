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

void AICodeCtrl::SetFont(Font fnt) {
	this->fnt = fnt;
	lineh = fnt.GetHeight() * 5 / 4;
	vscroll.SetLine(lineh);
}

void AICodeCtrl::Load(String filename, Stream& str, byte charset) {
	Ide* ide = TheIde();
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
	
	ReadNavigator();
}

void AICodeCtrl::Save(Stream& str, byte charset) {
	str.Put(this->content);
	
	AionFile& aion = AionFiles().GetAdd(aion_path);
	aion.Save();
}

void AICodeCtrl::ReadNavigator() {
	Ide *ide = TheIde();
	if(!ide)
		return;
	
	ide->editor.Navigator(true);
	
	Array<Navigator::NavItem> nitem;
	Index<String> nests;
	auto Nest = [&](const AnnotationItem& m, const String& path) {
		if(m.nspace == m.nest)
			return m.nest + "\xff" + path;
		return m.nest;
	};
	
	for(const AnnotationItem& m : ide->editor.annotations) {
		Navigator::NavItem& n = nitem.Add();
		(AnnotationItem&)n = m;
		n.path = ide->editfile;
		nests.FindAdd(n.nest = Nest(m, ide->editfile));
	}
	SortIndex(nests);
	DUMPC(nests);
	
	/*
struct AnnotationItem : Moveable<AnnotationItem> {
	String id; // Upp::Class::Method(Upp::Point p)
	String name; // Method
	String type; // for String x, Upp::String, surely valid for variables only
	String pretty; // void Class::Method(Point p)
	String nspace; // Upp
	String uname; // METHOD
	String nest; // Upp::Class
	String unest; // UPP::CLASS
	String bases; // base classes of struct/class
	Point  pos = Null;
	int    kind = Null;
	bool   definition = false;
	bool   isvirtual = false;
	bool   isstatic = false;
	
	void Serialize(Stream& s);
};
*/
	for(int i = 0; i < nitem.GetCount(); i++) {
		Navigator::NavItem& item = nitem[i];
		LOG(i << ": " << item.id << ", type:" << item.type << ", nest: " << item.nest);
	}
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

